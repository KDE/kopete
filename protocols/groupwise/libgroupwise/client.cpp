/*
    Kopete Groupwise Protocol
    client.cpp - The main interface for the Groupwise protocol

    Copyright (c) 2004      SUSE Linux AG	 	 http://www.suse.com
              (c) 2008      Novell, Inc.

    Based on Iris, Copyright (C) 2003  Justin Karneges

    Kopete (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>
 
    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include <qapplication.h>
#include <qtimer.h>

#include "chatroommanager.h"
#include "gwclientstream.h"
#include "privacymanager.h"
#include "requestfactory.h"
#include "task.h"
#include "tasks/conferencetask.h"
#include "tasks/connectiontask.h"
#include "tasks/createconferencetask.h"
#include "tasks/getdetailstask.h"
#include "tasks/getstatustask.h"
#include "tasks/joinconferencetask.h"
#include "tasks/keepalivetask.h"
#include "tasks/leaveconferencetask.h"
#include "tasks/logintask.h"
#include "tasks/rejectinvitetask.h"
#include "tasks/sendinvitetask.h"
#include "tasks/sendmessagetask.h"
#include "tasks/setstatustask.h"
#include "tasks/statustask.h"
#include "tasks/typingtask.h"
#include "userdetailsmanager.h"
#include "client.h"

class Client::ClientPrivate
{
public:
	ClientPrivate() {}

	ClientStream *stream;
	int id_seed;
	Task *root;
	QString host, user, userDN, pass;
	QString osname, tzname, clientName, clientVersion;
	uint port;
/*	int tzoffset;*/
	bool active;
	RequestFactory * requestFactory;
	ChatroomManager * chatroomMgr;
	UserDetailsManager * userDetailsMgr;
	PrivacyManager * privacyMgr;
	uint protocolVersion;
	QValueList<GroupWise::CustomStatus> customStatuses;
	QTimer * keepAliveTimer;
};

Client::Client(QObject *par, uint protocolVersion )
:QObject(par, "groupwiseclient")
{
	d = new ClientPrivate;
/*	d->tzoffset = 0;*/
	d->active = false;
	d->osname = "N/A";
	d->clientName = "N/A";
	d->clientVersion = "0.0";
	d->id_seed = 0xaaaa;
	d->root = new Task(this, true);
	d->chatroomMgr = 0;
	d->requestFactory = new RequestFactory;
	d->userDetailsMgr = new UserDetailsManager( this, "userdetailsmgr" );
	d->privacyMgr = new PrivacyManager( this, "privacymgr" );
	d->stream = 0;
	d->protocolVersion = protocolVersion;
	// Sends regular keepalives so the server knows we are still running
	d->keepAliveTimer = new QTimer( this );
	connect( d->keepAliveTimer, SIGNAL( timeout() ), SLOT( sendKeepAlive() ) );
}

Client::~Client()
{
	delete d->root;
	delete d->requestFactory;
	delete d->userDetailsMgr;
	delete d;
}

void Client::connectToServer( ClientStream *s, const NovellDN &server, bool auth )
{
	d->stream = s;
	//connect(d->stream, SIGNAL(connected()), SLOT(streamConnected()));
	//connect(d->stream, SIGNAL(handshaken()), SLOT(streamHandshaken()));
	connect(d->stream, SIGNAL(error(int)), SLOT(streamError(int)));
	//connect(d->stream, SIGNAL(sslCertificateReady(const QSSLCert &)), SLOT(streamSSLCertificateReady(const QSSLCert &)));
	connect(d->stream, SIGNAL(readyRead()), SLOT(streamReadyRead()));
	//connect(d->stream, SIGNAL(closeFinished()), SLOT(streamCloseFinished()));

	d->stream->connectToServer(server, auth);
}

void Client::setOSName(const QString &name)
{
	d->osname = name;
}

void Client::setClientName(const QString &s)
{
	d->clientName = s;
}

void Client::setClientVersion(const QString &s)
{
	d->clientVersion = s;
}

void Client::start( const QString &host, const uint port, const QString &userId, const QString &pass )
{
	d->host = host;
	d->port = port;
	d->user = userId;
	d->pass = pass;

	initialiseEventTasks();
	
	LoginTask * login = new LoginTask( d->root );
	
	connect( login, SIGNAL( gotMyself( const GroupWise::ContactDetails &  ) ), 
			this, SIGNAL( accountDetailsReceived( const GroupWise::ContactDetails & ) ) );
			
	connect( login, SIGNAL( gotFolder( const FolderItem & ) ), 
			this, SIGNAL( folderReceived( const FolderItem & ) ) );
			
	connect( login, SIGNAL( gotContact( const ContactItem &  ) ), 
			this, SIGNAL( contactReceived( const ContactItem &  ) ) );
			
	connect( login, SIGNAL( gotContactUserDetails( const GroupWise::ContactDetails & ) ), 
			this, SIGNAL( contactUserDetailsReceived( const GroupWise::ContactDetails & ) ) ) ;

	connect( login, SIGNAL( gotPrivacySettings( bool, bool, const QStringList &, const QStringList & ) ),
			privacyManager(), SLOT( slotGotPrivacySettings( bool, bool, const QStringList &, const QStringList & ) ) );

	connect( login, SIGNAL( gotCustomStatus( const GroupWise::CustomStatus & ) ), 
			SLOT( lt_gotCustomStatus( const GroupWise::CustomStatus & ) ) );

	connect( login, SIGNAL( gotKeepalivePeriod( int ) ), SLOT( lt_gotKeepalivePeriod( int ) ) );

	connect( login, SIGNAL( finished() ), this, SLOT( lt_loginFinished() ) );
	
	login->initialise();
	login->go( true );
	
	d->active = true;
}

void Client::close()
{
	debug( "Client::close()" );
	d->keepAliveTimer->stop();
	if(d->stream) {
		d->stream->disconnect(this);
		d->stream->close();
		d->stream = 0;
	}
}

QString Client::host()
{
	return d->host;
}

int Client::port()
{
	return d->port;
}

QValueList<GroupWise::CustomStatus> Client::customStatuses()
{
	return d->customStatuses;
}

void Client::initialiseEventTasks()
{
	// The StatusTask handles incoming status changes
	StatusTask * st = new StatusTask( d->root ); // FIXME - add an additional EventRoot?
	connect( st, SIGNAL( gotStatus( const QString &, Q_UINT16, const QString & ) ), SIGNAL( statusReceived( const QString &, Q_UINT16, const QString & ) ) );
	// The ConferenceTask handles incoming conference events, messages, joins, leaves, etc
	ConferenceTask * ct = new ConferenceTask( d->root ); 
	connect( ct, SIGNAL( message( const ConferenceEvent & ) ), SLOT( ct_messageReceived( const ConferenceEvent & ) ) );
	connect( ct, SIGNAL( typing( const ConferenceEvent & ) ), SIGNAL( contactTyping( const ConferenceEvent & ) ) );
	connect( ct, SIGNAL( notTyping( const ConferenceEvent & ) ), SIGNAL( contactNotTyping( const ConferenceEvent & ) ) );
	connect( ct, SIGNAL( joined( const ConferenceEvent & ) ), SIGNAL( conferenceJoinNotifyReceived( const ConferenceEvent & ) ) );
	connect( ct, SIGNAL( left( const ConferenceEvent & ) ), SIGNAL( conferenceLeft( const ConferenceEvent & ) ) );
	connect( ct, SIGNAL( invited( const ConferenceEvent & ) ), SIGNAL( invitationReceived( const ConferenceEvent & ) ) );
	connect( ct, SIGNAL( otherInvited( const ConferenceEvent & ) ), SIGNAL( inviteNotifyReceived( const ConferenceEvent & ) ) );
	connect( ct, SIGNAL( invitationDeclined( const ConferenceEvent & ) ), SIGNAL( invitationDeclined( const ConferenceEvent & ) ) );
	connect( ct, SIGNAL( closed( const ConferenceEvent & ) ), SIGNAL( conferenceClosed( const ConferenceEvent & ) ) );
	connect( ct, SIGNAL( autoReply( const ConferenceEvent & ) ), SIGNAL( autoReplyReceived( const ConferenceEvent & ) ) );
	connect( ct, SIGNAL( broadcast( const ConferenceEvent & ) ), SIGNAL( broadcastReceived( const ConferenceEvent & ) ) );
	connect( ct, SIGNAL( systemBroadcast( const ConferenceEvent & ) ), SIGNAL( systemBroadcastReceived( const ConferenceEvent & ) ) );
	

	// The ConnectionTask handles incoming connection events
	ConnectionTask* cont = new ConnectionTask( d->root );
	connect( cont, SIGNAL( connectedElsewhere() ), SIGNAL( connectedElsewhere() ) );
}

void Client::setStatus( GroupWise::Status status, const QString & reason, const QString & autoReply )
{
	debug( QString("Setting status to %1").arg( status ) );;
	SetStatusTask * sst = new SetStatusTask( d->root );
	sst->status( status, reason, autoReply );
	connect( sst, SIGNAL( finished() ), this, SLOT( sst_statusChanged() ) );
	sst->go( true );
	// TODO: set status change in progress flag
}

void Client::requestStatus( const QString & userDN )
{
	GetStatusTask * gst = new GetStatusTask( d->root );
	gst->userDN( userDN );
	connect( gst, SIGNAL( gotStatus( const QString &, Q_UINT16, const QString & ) ), SIGNAL( statusReceived( const QString &, Q_UINT16, const QString & ) ) );
	gst->go( true );
}

void Client::sendMessage( const QStringList & addresseeDNs, const OutgoingMessage & message )
{
	SendMessageTask * smt = new SendMessageTask( d->root );
	smt->message( addresseeDNs, message );
	connect( smt, SIGNAL( finished() ), SLOT( smt_messageSent() ) );
	smt->go( true );
}

void Client::sendTyping( const GroupWise::ConferenceGuid & conferenceGuid, bool typing )
{
	TypingTask * tt = new TypingTask( d->root );
	tt->typing( conferenceGuid, typing );
	tt->go( true );
}

void Client::createConference( const int clientId )
{
	QStringList dummy;
	createConference( clientId, dummy );
}

void Client::createConference( const int clientId, const QStringList & participants )
{
	CreateConferenceTask * cct = new CreateConferenceTask( d->root );
	cct->conference( clientId, participants );
	connect( cct, SIGNAL( finished() ), SLOT( cct_conferenceCreated() ) );
	cct->go( true );
}
void Client::requestDetails( const QStringList & userDNs )
{
	GetDetailsTask * gdt = new GetDetailsTask( d->root );
	gdt->userDNs( userDNs );
	connect( gdt, SIGNAL( gotContactUserDetails( const GroupWise::ContactDetails & ) ),
			this, SIGNAL( contactUserDetailsReceived( const GroupWise::ContactDetails & ) ) );
	gdt->go( true );
}

void Client::joinConference( const GroupWise::ConferenceGuid & guid )
{
	JoinConferenceTask * jct = new JoinConferenceTask( d->root );
	jct->join( guid );
	connect( jct, SIGNAL( finished() ), SLOT( jct_joinConfCompleted() ) );
	jct->go( true );
}

void Client::rejectInvitation( const GroupWise::ConferenceGuid & guid )
{
	RejectInviteTask * rit = new RejectInviteTask ( d->root );
	rit->reject( guid );
	// we don't do anything with the results of this task
	rit->go( true );
}

void Client::leaveConference( const GroupWise::ConferenceGuid & guid )
{
	LeaveConferenceTask * lct = new LeaveConferenceTask( d->root );
	lct->leave( guid );
	//connect( lct, SIGNAL( finished() ), SLOT( lct_leftConference() ) );
	lct->go( true );
}

void Client::sendInvitation( const GroupWise::ConferenceGuid & guid, const QString & dn, const GroupWise::OutgoingMessage & message )
{
	SendInviteTask * sit = new SendInviteTask( d->root );
	QStringList invitees( dn );
	sit->invite( guid, dn, message );
	sit->go( true );
}

// SLOTS //
void Client::streamError( int error )
{
	debug( QString( "CLIENT ERROR (Error %1)" ).arg( error ) );
}

void Client::streamReadyRead()
{
	debug( "CLIENT STREAM READY READ" );
	// take the incoming transfer and distribute it to the task tree
	Transfer * transfer = d->stream->read();
	distribute( transfer );
}

void Client::lt_loginFinished()
{
	debug( "Client::lt_loginFinished()" );
	const LoginTask * lt = (LoginTask *)sender();
	if ( lt->success() )
	{
		debug( "Client::lt_loginFinished() LOGIN SUCCEEDED" );
		// set our initial status
		SetStatusTask * sst = new SetStatusTask( d->root );
		sst->status( GroupWise::Available, QString::null, QString::null );
		sst->go( true );
		emit loggedIn();
		// fetch details for any privacy list items that aren't in our contact list.
		// There is a chicken-and-egg case regarding this: We need the privacy before reading the contact list so
		// blocked contacts are shown as blocked.  But we need not fetch user details for the privacy lists
		// before reading the contact list, as many privacy items' details are already in the contact list
  privacyManager()->getDetailsForPrivacyLists();
	}
	else
	{
		debug( "Client::lt_loginFinished() LOGIN FAILED" );
		emit loginFailed();
	}
	// otherwise client should disconnect and signal failure that way??
}

void Client::sst_statusChanged()
{
	const SetStatusTask * sst = (SetStatusTask *)sender();
	if ( sst->success() )
	{
		emit ourStatusChanged( sst->requestedStatus(), sst->awayMessage(), sst->autoReply() );
	}
}

void Client::ct_messageReceived( const ConferenceEvent & messageEvent )
{
	debug( "parsing received message's RTF" );
	ConferenceEvent transformedEvent = messageEvent;
	RTF2HTML parser;
	QString rtf = messageEvent.message;
	if ( !rtf.isEmpty() )
		transformedEvent.message = parser.Parse( rtf.latin1(), "" );

	// fixes for RTF to HTML conversion problems
	// we can drop these once the server reenables the sending of unformatted text
	// redundant linebreak at the end of the message
	QRegExp rx(" </span> </span> </span><br>$");
	transformedEvent.message.replace( rx, "</span></span></span>" );
	// missing linebreak after first line of an encrypted message
	QRegExp ry("-----BEGIN PGP MESSAGE----- </span> </span> </span>");
	transformedEvent.message.replace( ry, "-----BEGIN PGP MESSAGE-----</span></span></span><br/>" );

	emit messageReceived( transformedEvent );
}

void Client::cct_conferenceCreated()
{
	const CreateConferenceTask * cct = ( CreateConferenceTask * )sender();
	if ( cct->success() )
	{
		emit conferenceCreated( cct->clientConfId(), cct->conferenceGUID() );
	}
	else
	{
		emit conferenceCreationFailed( cct->clientConfId(), cct->statusCode() );
	}
}

void Client::jct_joinConfCompleted()
{
	const JoinConferenceTask * jct = ( JoinConferenceTask * )sender();
#ifdef LIBGW_DEBUG
	debug( QString( "Joined conference %1, participants are: " ).arg( jct->guid() ) );
	QStringList parts = jct->participants();
	for ( QStringList::Iterator it = parts.begin(); it != parts.end(); ++it )
		debug( QString( " - %1" ).arg(*it) );
	debug( "invitees are: " );
	QStringList invitees = jct->invitees();
	for ( QStringList::Iterator it = invitees.begin(); it != invitees.end(); ++it )
		debug( QString( " - %1" ).arg(*it) );
#endif
	emit conferenceJoined( jct->guid(), jct->participants(), jct->invitees() );
}

void Client::lt_gotCustomStatus( const GroupWise::CustomStatus & custom )
{
	d->customStatuses.append( custom );
}

// INTERNALS //

QString Client::userId()
{
	return d->user;
}

void Client::setUserDN( const QString & userDN )
{
	d->userDN = userDN;
}

QString Client::userDN()
{
	return d->userDN;
}

QString Client::password()
{
	return d->pass;
}

QString Client::userAgent()
{
	return QString::fromLatin1( "%1/%2 (%3)" ).arg( d->clientName, d->clientVersion, d->osname );
}

QCString Client::ipAddress()
{
	// TODO: remove hardcoding
	return "10.10.11.103";
}

void Client::distribute( Transfer * transfer )
{
	if( !rootTask()->take( transfer ) )
		debug( "CLIENT: root task refused transfer" );
	// at this point the transfer is no longer needed
	delete transfer;
}

void Client::send( Request * request )
{
	debug( "CLIENT::send()" );
	if( !d->stream )
	{	
		debug( "CLIENT - NO STREAM TO SEND ON!");
		return;
	}
// 	QString out = request.toString();
// 	debug(QString("Client: outgoing: [\n%1]\n").arg(out));
// 	xmlOutgoing(out);

	d->stream->write( request );
}

void Client::debug( const QString &str )
{
#ifdef LIBGW_USE_KDEBUG
	kdDebug( GROUPWISE_DEBUG_LIBGW ) << "debug: " << str << endl;
#else
	qDebug( "CLIENT: %s\n", str.ascii() );
#endif
}

QString Client::genUniqueId()
{
	QString s;
	s.sprintf("a%x", d->id_seed);
	d->id_seed += 0x10;
	return s;
}

PrivacyManager * Client::privacyManager()
{
	return d->privacyMgr;
}

RequestFactory * Client::requestFactory()
{
	return d->requestFactory;
}

UserDetailsManager * Client::userDetailsManager()
{
	return d->userDetailsMgr;
}

Task * Client::rootTask()
{
	return d->root;
}

uint Client::protocolVersion() const
{
	return d->protocolVersion;
}

ChatroomManager * Client::chatroomManager()
{
	if ( !d->chatroomMgr )
		d->chatroomMgr = new ChatroomManager( this, "chatroommgr" );
	return d->chatroomMgr;
}

void Client::lt_gotKeepalivePeriod( int period )
{
	d->keepAliveTimer->start( period * 60 * 1000 );
}

void Client::sendKeepAlive()
{
	KeepAliveTask * kat = new KeepAliveTask( d->root );
	kat->setup();
	kat->go( true );
}

void Client::smt_messageSent()
{
	const SendMessageTask * smt = ( SendMessageTask * )sender();
	if ( smt->success() )
	{
		debug( "message sent OK" );
	}
	else
	{
		debug( "message sending failed!" );
		emit messageSendingFailed();
	}
}

#include "client.moc"
