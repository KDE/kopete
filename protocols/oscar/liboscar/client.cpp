/*
	client.cpp - Kopete Oscar Protocol

	Copyright (c) 2004-2005 Matt Rogers <mattr@kde.org>

	Based on code Copyright (c) 2004 SuSE Linux AG <http://www.suse.com>
	Based on Iris, Copyright (C) 2003  Justin Karneges

	Kopete (c) 2002-2005 by the Kopete developers <kopete-devel@kde.org>

	*************************************************************************
	*                                                                       *
	* This library is free software; you can redistribute it and/or         *
	* modify it under the terms of the GNU Lesser General Public            *
	* License as published by the Free Software Foundation; either          *
	* version 2 of the License, or (at your option) any later version.      *
	*                                                                       *
	*************************************************************************
*/

#include "client.h"

#include <qtimer.h>

#include <kdebug.h> //for kdDebug()
#include <klocale.h>

#include "buddyicontask.h"
#include "clientreadytask.h"
#include "connectionhandler.h"
#include "changevisibilitytask.h"
#include "errortask.h"
#include "icquserinfo.h"
#include "icquserinfotask.h"
#include "logintask.h"
#include "connection.h"
#include "messagereceivertask.h"
#include "onlinenotifiertask.h"
#include "oscarclientstream.h"
#include "oscarconnector.h"
#include "oscarsettings.h"
#include "ownuserinfotask.h"
#include "profiletask.h"
#include "senddcinfotask.h"
#include "sendmessagetask.h"
#include "serverredirecttask.h"
#include "servicesetuptask.h"
#include "ssimanager.h"
#include "ssimodifytask.h"
#include "ssiauthtask.h"
#include "offlinemessagestask.h"
#include "task.h"
#include "typingnotifytask.h"
#include "userinfotask.h"
#include "usersearchtask.h"
#include "warningtask.h"


class Client::ClientPrivate
{
public:
	ClientPrivate() {}

	QString host, user, pass;
	uint port;
	int tzoffset;
	bool active;

	enum { StageOne, StageTwo };
	int stage;

	//Protocol specific data
	bool isIcq;
	bool redirectRequested;
	QByteArray cookie;
	DWORD connectAsStatus; // icq only
	QString connectWithMessage; // icq only
	Oscar::Settings* settings;

	//Tasks
	ErrorTask* errorTask;
	OnlineNotifierTask* onlineNotifier;
	OwnUserInfoTask* ownStatusTask;
	MessageReceiverTask* messageReceiverTask;
	SSIAuthTask* ssiAuthTask;
	ICQUserInfoRequestTask* icqInfoTask;
	UserInfoTask* userInfoTask;
	CloseConnectionTask* closeConnectionTask;
	TypingNotifyTask * typingNotifyTask;
	//Managers
	SSIManager* ssiManager;
	ConnectionHandler connections;

	//Our Userinfo
	UserDetails ourDetails;

};

Client::Client( QObject* parent )
:QObject( parent, "oscarclient" )
{
	m_loginTask = 0L;
	m_loginTaskTwo = 0L;

	d = new ClientPrivate;
	d->tzoffset = 0;
	d->active = false;
	d->isIcq = false; //default to AIM
	d->redirectRequested = false;
	d->connectAsStatus = 0x0; // default to online
	d->ssiManager = new SSIManager( this );
	d->settings = new Oscar::Settings();
	d->errorTask = 0L;
	d->onlineNotifier = 0L;
	d->ownStatusTask = 0L;
	d->messageReceiverTask = 0L;
	d->ssiAuthTask = 0L;
	d->icqInfoTask = 0L;
	d->userInfoTask = 0L;
	d->closeConnectionTask = 0L;
	d->stage = ClientPrivate::StageOne;
	d->typingNotifyTask = 0L;
}

Client::~Client()
{

	//delete the connections differently than in deleteConnections()
	//deleteLater() seems to cause destruction order issues
	deleteStaticTasks();
	delete d->ssiManager;
	delete d;
}

Oscar::Settings* Client::clientSettings() const
{
	return d->settings;
}

void Client::connectToServer( Connection *c, const QString& server, bool auth )
{
	d->connections.append( c );
	if ( auth == true )
	{
		m_loginTask = new StageOneLoginTask( c->rootTask() );
		connect( m_loginTask, SIGNAL( finished() ), this, SLOT( lt_loginFinished() ) );
	}

	connect( c, SIGNAL( socketError( int, const QString& ) ), SIGNAL( socketError( int, const QString& ) ) );
	c->connectToServer(server, auth);
}

void Client::start( const QString &host, const uint port, const QString &userId, const QString &pass )
{
	Q_UNUSED( host );
	Q_UNUSED( port );
	d->user = userId;
	d->pass = pass;
	d->stage = ClientPrivate::StageOne;
	d->active = false;
}

void Client::close()
{
	d->active = false;
//	kdDebug( OSCAR_RAW_DEBUG ) << k_funcinfo << "Closing " << d->connections.count() << " connections" << endl;
	//these are based on a connection. delete them.
	d->connections.clear();
	deleteStaticTasks();

	//don't clear the stored status between stage one and two
	if ( d->stage == ClientPrivate::StageTwo )
	{
		d->connectAsStatus = 0x0;
		d->connectWithMessage = QString::null;
	}
//	kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Clearing our internal SSI list" << endl;
	d->ssiManager->clear();

}

void Client::setStatus( AIMStatus status, const QString &_message )
{
	// AIM: you're away exactly when your away message isn't empty.
	// can't use QString::null as a message either; ProfileTask
	// interprets null as "don't change".
	QString message;
	if ( status == Online )
		message = QString::fromAscii("");
	else
	{
		if ( _message.isEmpty() )
			message = QString::fromAscii(" ");
		else
			message = _message;
	}

	Connection* c = d->connections.connectionForFamily( 0x0002 );
	if ( !c )
		return;
	ProfileTask* pt = new ProfileTask( c->rootTask() );
	pt->setAwayMessage( message );
	pt->go( true );
}

void Client::setStatus( DWORD status, const QString &message )
{
	// ICQ: if we're active, set status. otherwise, just store the status for later.
	if ( d->active )
	{
		//the first connection is always the BOS connection
		Connection* c = d->connections.connectionForFamily( 0x0013 );
		if ( !c )
			return; //TODO trigger an error of some sort?
		
		ChangeVisibilityTask* cvt = new ChangeVisibilityTask( c->rootTask() );
		if ( ( status & 0x0100 ) == 0x0100 )
		{
			kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Setting invisible" << endl;
			cvt->setVisible( false );
		}
		else
		{
			kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Setting visible" << endl;
			cvt->setVisible( true );
		}
		cvt->go( true );
		c = d->connections.connectionForFamily( 0x0002 );
		if ( !c )
			return;
		
		SendDCInfoTask* sdcit = new SendDCInfoTask( c->rootTask(), status );
		sdcit->go( true ); //autodelete
		// TODO: send away message
	}
	else
	{
		d->connectAsStatus = status;
		d->connectWithMessage = message;
	}
}

UserDetails Client::ourInfo() const
{
	return d->ourDetails;
}

QString Client::host()
{
	return d->host;
}

int Client::port()
{
	return d->port;
}

SSIManager* Client::ssiManager() const
{
	return d->ssiManager;
}

// SLOTS //

void Client::streamConnected()
{
	kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << endl;
	d->stage = ClientPrivate::StageTwo;
	if ( m_loginTaskTwo )
		m_loginTaskTwo->go();
}

void Client::lt_loginFinished()
{
	/* Check for stage two login first, since we create the stage two
	 * task when we finish stage one
	 */
	if ( d->stage == ClientPrivate::StageTwo )
	{
		//we've finished logging in. start the services setup
		kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "stage two done. setting up services" << endl;
		initializeStaticTasks();
		ServiceSetupTask* ssTask = new ServiceSetupTask( d->connections.defaultConnection()->rootTask() );
		connect( ssTask, SIGNAL( finished() ), this, SLOT( serviceSetupFinished() ) );
		ssTask->go( true ); //fire and forget
		m_loginTaskTwo->deleteLater();
		m_loginTaskTwo = 0;
	}
	else if ( d->stage == ClientPrivate::StageOne )
	{
		kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "stage one login done" << endl;
		disconnect( m_loginTask, SIGNAL( finished() ), this, SLOT( lt_loginFinished() ) );

		if ( m_loginTask->statusCode() == 0 ) //we can start stage two
		{
			kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "no errors from stage one. moving to stage two" << endl;

			//cache these values since they'll be deleted when we close the connections (which deletes the tasks)
			d->host = m_loginTask->bosServer();
			d->port = m_loginTask->bosPort().toUInt();
			d->cookie = m_loginTask->loginCookie();
			close();
			QTimer::singleShot( 100, this, SLOT(startStageTwo() ) );
		}
		else
		{
			kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "errors reported. not moving to stage two" << endl;
			close(); //deletes the connections for us
		}

		m_loginTask->deleteLater();
		m_loginTask = 0;
	}

}

void Client::startStageTwo()
{
	//create a new connection and set it up
	Connection* c = createConnection( d->host, QString::number( d->port ) );
	d->closeConnectionTask = new CloseConnectionTask( c->rootTask() );
	QObject::connect( d->closeConnectionTask, SIGNAL( disconnected( int, const QString& ) ),
	                  this, SLOT( disconnectionError( int, const QString& ) ) );

	//create the new login task
	m_loginTaskTwo = new StageTwoLoginTask( c->rootTask() );
	m_loginTaskTwo->setCookie( d->cookie );
	QObject::connect( m_loginTaskTwo, SIGNAL( finished() ), this, SLOT( lt_loginFinished() ) );


	//connect
	QObject::connect( c, SIGNAL( connected() ), this, SLOT( streamConnected() ) );
	connectToServer( c, d->host, false ) ;
	
}

void Client::serviceSetupFinished()
{
	d->active = true;

	if ( isIcq() )
	{
		setStatus( d->connectAsStatus, d->connectWithMessage );

		//retrieve offline messages
		Connection* c = d->connections.connectionForFamily( 0x0015 );
		if ( !c )
			return;
		
		OfflineMessagesTask *offlineMsgTask = new OfflineMessagesTask( c->rootTask() );
		connect( offlineMsgTask, SIGNAL( receivedOfflineMessage(const Oscar::Message& ) ),
				this, SIGNAL( messageReceived(const Oscar::Message& ) ) );
		offlineMsgTask->go( true );
	}

	emit haveSSIList();
	emit loggedIn();
}

void Client::receivedIcqInfo( const QString& contact, unsigned int type )
{
	kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "received icq info for " << contact
		<< " of type " << type << endl;

	if ( type == ICQUserInfoRequestTask::Short )
		emit receivedIcqShortInfo( contact );
	else
		emit receivedIcqLongInfo( contact );
}

void Client::receivedInfo( Q_UINT16 sequence )
{
	UserDetails details = d->userInfoTask->getInfoFor( sequence );
	emit receivedUserInfo( details.userId(), details );
}

void Client::offlineUser( const QString& user, const UserDetails& )
{
	emit userIsOffline( user );
}

void Client::haveOwnUserInfo()
{
	kdDebug( OSCAR_RAW_DEBUG ) << k_funcinfo << endl;
	UserDetails ud = d->ownStatusTask->getInfo();
	d->ourDetails = ud;
	emit haveOwnInfo();
}


// INTERNALS //

QString Client::userId() const
{
	return d->user;
}

QString Client::password() const
{
	return d->pass;
}

QCString Client::ipAddress() const
{
	//!TODO determine ip address
	return "127.0.0.1";
}

void Client::notifyTaskError( const Oscar::SNAC& s, int errCode, bool fatal )
{
	emit taskError( s, errCode, fatal );
}

void Client::notifySocketError( int errCode, const QString& msg )
{
	emit socketError( errCode, msg );
}

void Client::sendMessage( const Oscar::Message& msg, bool isAuto)
{
	Connection* c = d->connections.connectionForFamily( 0x0004 );
	if ( !c )
		return;
	SendMessageTask *sendMsgTask = new SendMessageTask( c->rootTask() );
	// Set whether or not the message is an automated response
	sendMsgTask->setAutoResponse( isAuto );
	sendMsgTask->setMessage( msg );
	sendMsgTask->go( true );
}

void Client::requestAuth( const QString& contactid, const QString& reason )
{
	Connection* c = d->connections.connectionForFamily( 0x0013 );
	if ( !c )
		return;
	d->ssiAuthTask->sendAuthRequest( contactid, reason );
}

void Client::sendAuth( const QString& contactid, const QString& reason, bool auth )
{
	Connection* c = d->connections.connectionForFamily( 0x0013 );
	if ( !c )
		return;
	d->ssiAuthTask->sendAuthReply( contactid, reason, auth );
}

bool Client::isActive() const
{
	return d->active;
}

bool Client::isIcq() const
{
	return d->isIcq;
}

void Client::setIsIcq( bool isIcq )
{
	d->isIcq = isIcq;
}

void Client::debug( const QString& str )
{
	Q_UNUSED(str);
//	qDebug( "CLIENT: %s", str.ascii() );
}

void Client::initializeStaticTasks()
{
	//set up the extra tasks
	Connection* c = d->connections.defaultConnection();
	if ( !c )
		return;
	d->errorTask = new ErrorTask( c->rootTask() );
	d->onlineNotifier = new OnlineNotifierTask( c->rootTask() );
	d->ownStatusTask = new OwnUserInfoTask( c->rootTask() );
	d->messageReceiverTask = new MessageReceiverTask( c->rootTask() );
	d->ssiAuthTask = new SSIAuthTask( c->rootTask() );
	d->icqInfoTask = new ICQUserInfoRequestTask( c->rootTask() );
	d->userInfoTask = new UserInfoTask( c->rootTask() );
	d->typingNotifyTask = new TypingNotifyTask( c->rootTask() );

	connect( d->onlineNotifier, SIGNAL( userIsOnline( const QString&, const UserDetails& ) ),
	         this, SIGNAL( receivedUserInfo( const QString&, const UserDetails& ) ) );
	connect( d->onlineNotifier, SIGNAL( userIsOffline( const QString&, const UserDetails& ) ),
	         this, SLOT( offlineUser( const QString&, const UserDetails & ) ) );

	connect( d->ownStatusTask, SIGNAL( gotInfo() ), this, SLOT( haveOwnUserInfo() ) );

	connect( d->messageReceiverTask, SIGNAL( receivedMessage( const Oscar::Message& ) ),
	         this, SIGNAL( messageReceived( const Oscar::Message& ) ) );

	connect( d->ssiAuthTask, SIGNAL( authRequested( const QString&, const QString& ) ),
	         this, SIGNAL( authRequestReceived( const QString&, const QString& ) ) );
	connect( d->ssiAuthTask, SIGNAL( authReplied( const QString&, const QString&, bool ) ),
	         this, SIGNAL( authReplyReceived( const QString&, const QString&, bool ) ) );

	connect( d->icqInfoTask, SIGNAL( receivedInfoFor( const QString&, unsigned int ) ),
	         this, SLOT( receivedIcqInfo( const QString&, unsigned int ) ) );

	connect( d->userInfoTask, SIGNAL( receivedProfile( const QString&, const QString& ) ),
	         this, SIGNAL( receivedProfile( const QString&, const QString& ) ) );
	connect( d->userInfoTask, SIGNAL( receivedAwayMessage( const QString&, const QString& ) ),
	         this, SIGNAL( receivedAwayMessage( const QString&, const QString& ) ) );
	connect( d->typingNotifyTask, SIGNAL( typingStarted( const QString& ) ),
	         this, SIGNAL( userStartedTyping( const QString& ) ) );
	connect( d->typingNotifyTask, SIGNAL( typingFinished( const QString& ) ),
	         this, SIGNAL( userStoppedTyping( const QString& ) ) );
}

void Client::removeGroup( const QString& groupName )
{
	Connection* c = d->connections.connectionForFamily( 0x0013 );
	if ( !c )
		return;

	kdDebug( OSCAR_RAW_DEBUG ) << k_funcinfo << "Removing group " << groupName << " from SSI" << endl;
	SSIModifyTask* ssimt = new SSIModifyTask( c->rootTask() );
	if ( ssimt->removeGroup( groupName ) )
		ssimt->go( true );
}

void Client::addGroup( const QString& groupName )
{
	Connection* c = d->connections.connectionForFamily( 0x0013 );
	if ( !c )
		return;

	kdDebug( OSCAR_RAW_DEBUG ) << k_funcinfo << "Adding group " << groupName << " to SSI" << endl;
	SSIModifyTask* ssimt = new SSIModifyTask( c->rootTask() );
	if ( ssimt->addGroup( groupName ) )
		ssimt->go( true );
}

void Client::addContact( const QString& contactName, const QString& groupName )
{
	Connection* c = d->connections.connectionForFamily( 0x0013 );
	if ( !c )
		return;

	kdDebug( OSCAR_RAW_DEBUG ) << k_funcinfo << "Adding contact " << contactName << " to SSI in group " << groupName << endl;
	SSIModifyTask* ssimt = new SSIModifyTask( c->rootTask() );
	if ( ssimt->addContact( contactName, groupName )  )
		ssimt->go( true );

}

void Client::removeContact( const QString& contactName )
{
	Connection* c = d->connections.connectionForFamily( 0x0013 );
	if ( !c )
		return;

	kdDebug( OSCAR_RAW_DEBUG ) << k_funcinfo << "Removing contact " << contactName << " from SSI" << endl;
	SSIModifyTask* ssimt = new SSIModifyTask( c->rootTask() );
	if ( ssimt->removeContact( contactName ) )
		ssimt->go( true );
}

void Client::renameGroup( const QString & oldGroupName, const QString & newGroupName )
{
	Connection* c = d->connections.connectionForFamily( 0x0013 );
	if ( !c )
		return;

	kdDebug( OSCAR_RAW_DEBUG ) << k_funcinfo << "Renaming group " << oldGroupName << " to " << newGroupName << endl;
	SSIModifyTask* ssimt = new SSIModifyTask( c->rootTask() );
	if ( ssimt->renameGroup( oldGroupName, newGroupName ) )
		ssimt->go( true );
}

void Client::changeContactGroup( const QString& contact, const QString& newGroupName )
{
	Connection* c = d->connections.connectionForFamily( 0x0013 );
	if ( !c )
		return;

	kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Changing " << contact << "'s group to "
		<< newGroupName << endl;
	SSIModifyTask* ssimt = new SSIModifyTask( c->rootTask() );
	if ( ssimt->changeGroup( contact, newGroupName ) )
		ssimt->go( true );
}

void Client::requestFullInfo( const QString& contactId )
{
	Connection* c = d->connections.connectionForFamily( 0x0015 );
	if ( !c )
		return;
	d->icqInfoTask->setUser( contactId );
	d->icqInfoTask->setType( ICQUserInfoRequestTask::Long );
	d->icqInfoTask->go();
}

void Client::requestShortInfo( const QString& contactId )
{
	Connection* c = d->connections.connectionForFamily( 0x0015 );
	if ( !c )
		return;
	d->icqInfoTask->setUser( contactId );
	d->icqInfoTask->setType( ICQUserInfoRequestTask::Short );
	d->icqInfoTask->go();
}

void Client::sendWarning( const QString& contact, bool anonymous )
{
	Connection* c = d->connections.connectionForFamily( 0x0004 );
	if ( !c )
		return;
	WarningTask* warnTask = new WarningTask( c->rootTask() );
	warnTask->setContact( contact );
	warnTask->setAnonymous( anonymous );
	QObject::connect( warnTask, SIGNAL( userWarned( const QString&, Q_UINT16, Q_UINT16 ) ),
	                  this, SIGNAL( userWarned( const QString&, Q_UINT16, Q_UINT16 ) ) );
	warnTask->go( true );
}

ICQGeneralUserInfo Client::getGeneralInfo( const QString& contact )
{
	return d->icqInfoTask->generalInfoFor( contact );
}

ICQWorkUserInfo Client::getWorkInfo( const QString& contact )
{
	return d->icqInfoTask->workInfoFor( contact );
}

ICQEmailInfo Client::getEmailInfo( const QString& contact )
{
	return d->icqInfoTask->emailInfoFor( contact );
}

ICQMoreUserInfo Client::getMoreInfo( const QString& contact )
{
	return d->icqInfoTask->moreInfoFor( contact );
}

ICQShortInfo Client::getShortInfo( const QString& contact )
{
	return d->icqInfoTask->shortInfoFor( contact );
}

void Client::requestAIMProfile( const QString& contact )
{
	d->userInfoTask->requestInfoFor( contact, UserInfoTask::Profile );
}

void Client::requestAIMAwayMessage( const QString& contact )
{
	d->userInfoTask->requestInfoFor( contact, UserInfoTask::AwayMessage );
}

void Client::requestStatusInfo( const QString& contact )
{
	d->userInfoTask->requestInfoFor( contact, UserInfoTask::General );
}

void Client::whitePagesSearch( const ICQWPSearchInfo& info )
{
	Connection* c = d->connections.connectionForFamily( 0x0015 );
	if ( !c )
		return;
	UserSearchTask* ust = new UserSearchTask( c->rootTask() );
	connect( ust, SIGNAL( foundUser( const ICQSearchResult& ) ),
	         this, SIGNAL( gotSearchResults( const ICQSearchResult& ) ) );
	connect( ust, SIGNAL( searchFinished( int ) ), this, SIGNAL( endOfSearch( int ) ) );
	ust->go( true ); //onGo does nothing in this task. This is just here so autodelete works
	ust->searchWhitePages( info );
}

void Client::uinSearch( const QString& uin )
{
	Connection* c = d->connections.connectionForFamily( 0x0015 );
	if ( !c )
		return;
	UserSearchTask* ust = new UserSearchTask( c->rootTask() );
	connect( ust, SIGNAL( foundUser( const ICQSearchResult& ) ),
	         this, SIGNAL( gotSearchResults( const ICQSearchResult& ) ) );
	connect( ust, SIGNAL( searchFinished( int ) ), this, SIGNAL( endOfSearch( int ) ) );
	ust->go( true ); //onGo does nothing in this task. This is just here so autodelete works
	ust->searchUserByUIN( uin );
}

void Client::updateProfile( const QString& profile )
{
	Connection* c = d->connections.connectionForFamily( 0x0002 );
	if ( !c )
		return;
	ProfileTask* pt = new ProfileTask( c->rootTask() );
	pt->setProfileText( profile );
	pt->go(true);
}

void Client::sendTyping( const QString & contact, bool typing )
{
	Connection* c = d->connections.connectionForFamily( 0x0004 );
	if ( !c )
		return;
	d->typingNotifyTask->setParams( contact, ( typing ? TypingNotifyTask::Begin : TypingNotifyTask::Finished ) );
	d->typingNotifyTask->go( false ); 	// don't delete the task after sending
}

void Client::connectToIconServer()
{
	Connection* c = d->connections.connectionForFamily( 0x0010 );
	if ( c )
		return;
	
	requestServerRedirect( 0x0010 );
	return;
}

void Client::requestBuddyIcon( const QString& user, const QByteArray& hash, BYTE hashType )
{
	Connection* c = d->connections.connectionForFamily( 0x0010 );
	if ( !c )
		return;

	BuddyIconTask* bit = new BuddyIconTask( c->rootTask() );
	connect( bit, SIGNAL( haveIcon( const QString&, QByteArray ) ),
	         this, SIGNAL( haveIconForContact( const QString&, QByteArray ) ) );
	bit->requestIconFor( user );
	bit->setHashType( hashType );
	bit->setHash( hash );
	bit->go( true );
}

void Client::requestServerRedirect( WORD family )
{
	//making the assumption that family 2 will always be the BOS connection
	//use it instead since we can't query for family 1
	Connection* c = d->connections.connectionForFamily( 0x0002 );
	if ( !c )
		return;
	if ( d->redirectRequested == false  )
	{
		d->redirectRequested = true;
		ServerRedirectTask* srt = new ServerRedirectTask( c->rootTask() );
		connect( srt, SIGNAL( haveServer( const QString&, const QByteArray&, WORD ) ),
		         this, SLOT( haveServerForRedirect( const QString&, const QByteArray&, WORD ) ) );
		srt->setService( family );
		srt->go( true );
	}
}

void Client::haveServerForRedirect( const QString& host, const QByteArray& cookie, WORD )
{
	//create a new connection and set it up
	int colonPos = host.find(':');
	QString realHost, realPort;
	if ( colonPos != -1 )
	{
		realHost = host.left( colonPos );
		realPort = host.right(4); //we only need 4 bytes
	}
	else
	{
		realHost = host;
		realPort = QString::fromLatin1("5190");
	}

	Connection* c = createConnection( realHost, realPort );
	//create the new login task
	m_loginTaskTwo = new StageTwoLoginTask( c->rootTask() );
	m_loginTaskTwo->setCookie( cookie );
	QObject::connect( m_loginTaskTwo, SIGNAL( finished() ), this, SLOT( serverRedirectFinished() ) );

	//connect
	connectToServer( c, d->host, false );
	QObject::connect( c, SIGNAL( connected() ), this, SLOT( streamConnected() ) );
}

void Client::serverRedirectFinished()
{
	//FIXME un-hardcode icon stuff
	if ( m_loginTaskTwo->statusCode() == 0 )
	{ //stage two was successful
		Connection* c = d->connections.connectionForFamily( 0x0010 );
		if ( !c )
			return;
		ClientReadyTask* crt = new ClientReadyTask( c->rootTask() );
		crt->setFamilies( c->supportedFamilies() );
		crt->go( true );
	}

	emit iconServerConnected();
}

Connection* Client::createConnection( const QString& host, const QString& port )
{
	KNetworkConnector* knc = new KNetworkConnector( this );
	knc->setOptHostPort( host, port.toUInt() );
	ClientStream* cs = new ClientStream( knc, knc );
	Connection* c = new Connection( knc, cs, "BOS" );
	cs->setConnection( c );
	c->setClient( this );
	return c;
}

void Client::deleteStaticTasks()
{
	delete d->errorTask;
	delete d->onlineNotifier;
	delete d->ownStatusTask;
	delete d->messageReceiverTask;
	delete d->ssiAuthTask;
	delete d->icqInfoTask;
	delete d->userInfoTask;
	delete d->closeConnectionTask;
	delete d->typingNotifyTask;

	d->errorTask = 0;
	d->onlineNotifier = 0;
	d->ownStatusTask = 0;
	d->messageReceiverTask = 0;
	d->ssiAuthTask = 0;
	d->icqInfoTask = 0;
	d->userInfoTask = 0;
	d->closeConnectionTask = 0;
	d->typingNotifyTask = 0;
}

#include "client.moc"
//kate: tab-width 4; indent-mode csands; space-indent off; replace-tabs off;
