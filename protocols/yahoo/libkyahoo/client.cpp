/*
    Kopete Yahoo Protocol
    
    Copyright (c) 2005-2006 André Duffeck <andre.duffeck@kdemail.net>
    Copyright (c) 2004 Duncan Mac-Vicar P. <duncan@kde.org>
    Copyright (c) 2004 Matt Rogers <matt.rogers@kdemail.net>
    Copyright (c) 2004 SuSE Linux AG <http://www.suse.com>
    Copyright (C) 2003  Justin Karneges
    
    Kopete (c) 2002-2006 by the Kopete developers <kopete-devel@kde.org>
 
    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include <qtimer.h>

#include <kdebug.h>
#include <kurl.h>
#include <ksocketbase.h>

#include "yahooclientstream.h"
#include "yahooconnector.h"
#include "task.h"
#include "logintask.h"
#include "listtask.h"
#include "statusnotifiertask.h"
#include "mailnotifiertask.h"
#include "messagereceivertask.h"
#include "sendnotifytask.h"
#include "sendmessagetask.h"
#include "logofftask.h"
#include "changestatustask.h"
#include "modifybuddytask.h"
#include "picturenotifiertask.h"
#include "requestpicturetask.h"
#include "stealthtask.h"
#include "sendpicturetask.h"
#include "webcamtask.h"
#include "conferencetask.h"
#include "sendauthresptask.h"
#include "pingtask.h"
#include "yabtask.h"
#include "modifyyabtask.h"
#include "chatsessiontask.h"
#include "sendfiletask.h"
#include "filetransfernotifiertask.h"
#include "receivefiletask.h"
#include "client.h"
#include "yahootypes.h"
#include "yahoobuddyiconloader.h"

using namespace KNetwork;

class Client::ClientPrivate
{
public:
	ClientPrivate() {}

	ClientStream *stream;
	int id_seed;
	Task *root;
	QString host, user, pass;
	uint port;
	bool active;
	YahooBuddyIconLoader *iconLoader;
	int error;
	QString errorString;
	QString errorInformation;
	
	// tasks
	bool tasksInitialized;
	LoginTask * loginTask;
	ListTask *listTask;
	StatusNotifierTask *statusTask;
	MailNotifierTask *mailTask;
	MessageReceiverTask *messageReceiverTask;
	PictureNotifierTask *pictureNotifierTask;
	WebcamTask *webcamTask;
	ConferenceTask *conferenceTask;
	YABTask *yabTask;
	FileTransferNotifierTask *fileTransferTask;

	// Connection data
	uint sessionID;
	QString yCookie;
	QString tCookie;
	QString cCookie;
	Yahoo::Status status;
	Yahoo::Status statusOnConnect;
	QString statusMessageOnConnect;
	int pictureFlag;
};

Client::Client(QObject *par) :QObject(par, "yahooclient" )
{
	d = new ClientPrivate;
/*	d->tzoffset = 0;*/
	d->active = false;

	d->root = new Task(this, true);
	d->statusOnConnect = Yahoo::StatusAvailable;
	setStatus( Yahoo::StatusDisconnected );
	d->tasksInitialized = false;
	d->stream = 0L;
	d->iconLoader = 0L;
	d->loginTask = new LoginTask( d->root );
	d->listTask = new ListTask( d->root );
	d->pictureFlag = 0;
	m_connector = 0L;

	m_pingTimer = new QTimer( this );
	QObject::connect( m_pingTimer, SIGNAL( timeout() ), this, SLOT( sendPing() ) );

	QObject::connect( d->loginTask, SIGNAL( haveSessionID( uint ) ), SLOT( lt_gotSessionID( uint ) ) );
	QObject::connect( d->loginTask, SIGNAL( loginResponse( int, const QString& ) ), 
				SLOT( slotLoginResponse( int, const QString& ) ) );
	QObject::connect( d->loginTask, SIGNAL( haveCookies() ), SLOT( slotGotCookies() ) );
	QObject::connect( d->listTask, SIGNAL( gotBuddy(const QString &, const QString &, const QString &) ), 
					SIGNAL( gotBuddy(const QString &, const QString &, const QString &) ) );
	QObject::connect( d->listTask, SIGNAL( stealthStatusChanged( const QString&, Yahoo::StealthStatus ) ), 
					SIGNAL( stealthStatusChanged( const QString&, Yahoo::StealthStatus ) ) );
}

Client::~Client()
{
	close();
	delete d->iconLoader;
	delete d->root;
	delete d;
}

void Client::connect( const QString &host, const uint port, const QString &userId, const QString &pass )
{
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;
	d->host = host;
	d->port = port;
	d->user = userId;
	d->pass = pass;
	setStatus( Yahoo::StatusConnecting );

	m_connector = new KNetworkConnector;
	m_connector->setOptHostPort( host, port );
	d->stream = new ClientStream( m_connector, this );
	QObject::connect( d->stream, SIGNAL( connected() ), this, SLOT( cs_connected() ) );
	QObject::connect( d->stream, SIGNAL( error(int) ), this, SLOT( streamError(int) ) );
	QObject::connect( d->stream, SIGNAL( readyRead() ), this, SLOT( streamReadyRead() ) );
	
	d->stream->connectToServer( host, false );
}

void Client::cancelConnect()
{
	d->loginTask->reset();
}

void Client::cs_connected()
{
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;
	emit connected();
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << " starting login task ... "<<  endl;

	d->loginTask->setStateOnConnect( (d->statusOnConnect == Yahoo::StatusInvisible) ? Yahoo::StatusInvisible : Yahoo::StatusAvailable );
	d->loginTask->go();
	d->active = true;
}

void Client::close()
{
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;
	m_pingTimer->stop();
	if( d->active )
	{
		LogoffTask *lt = new LogoffTask( d->root );
		lt->go( true );
	}
	if( d->tasksInitialized)
		deleteTasks();	
	d->loginTask->reset();
	if( d->stream ) {
		QObject::disconnect( d->stream, SIGNAL( readyRead() ), this, SLOT( streamReadyRead() ) );
		d->stream->deleteLater();
	}
	d->stream = 0L;
	if( m_connector )
		m_connector->deleteLater();
	m_connector = 0L;
}

int Client::error()
{
	return d->error;
}

QString Client::errorString()
{
	return d->errorString;
}

QString Client::errorInformation()
{
	return d->errorInformation;
}

// SLOTS //
void Client::streamError( int error )
{
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << "CLIENT ERROR (Error " <<  error << ")" << endl;
	QString msg;

	d->active = false;

	// Examine error
	if( error == ClientStream::ErrConnection )			// Ask Connector in this case
	{
		d->error = m_connector->errorCode();
		d->errorString = KSocketBase::errorString( (KSocketBase::SocketError)d->error );
	}
	else
	{
		d->error = error;
		d->errorString = d->stream->errorText();
	}
	close();
	if( status() == Yahoo::StatusConnecting )
		emit loginFailed();
	else
		emit disconnected();
}

void Client::streamReadyRead()
{
	// take the incoming transfer and distribute it to the task tree
	Transfer * transfer = d->stream->read();
	distribute( transfer );
}

void Client::lt_loginFinished()
{
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;

	slotLoginResponse( d->loginTask->statusCode(), d->loginTask->statusString() );
}

void Client::slotLoginResponse( int response, const QString &msg )
{
	if( response == Yahoo::LoginOk )
	{
		if( !(d->statusOnConnect == Yahoo::StatusAvailable ||
				d->statusOnConnect == Yahoo::StatusInvisible) ||
				!d->statusMessageOnConnect.isEmpty() )
			changeStatus( d->statusOnConnect, d->statusMessageOnConnect, Yahoo::StatusTypeAway );
		d->statusMessageOnConnect = QString::null;
		setStatus( d->statusOnConnect );
		m_pingTimer->start( 60 * 1000 );
		initTasks();
	} else {
		d->active = false;
		close();
	}

	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << "Emitting loggedIn" << endl;
	emit loggedIn( response, msg );
}

void Client::lt_gotSessionID( uint id )
{
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << "Got SessionID: " << id << endl;	
	d->sessionID = id;
}

void Client::slotGotCookies()
{
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << "Y: " << d->loginTask->yCookie()
					<< " T: " << d->loginTask->tCookie()
					<< " C: " << d->loginTask->cCookie() << endl;
	d->yCookie = d->loginTask->yCookie();
	d->tCookie = d->loginTask->tCookie();
	d->cCookie = d->loginTask->cCookie();
}

// INTERNALS //

// ***** Messaging handling *****
void Client::sendTyping( const QString &who, bool typing )
{
	SendNotifyTask *snt = new SendNotifyTask( d->root );
	snt->setTarget( who );
	snt->setState( typing ? SendNotifyTask::Active : SendNotifyTask::NotActive );
	snt->setType( SendNotifyTask::NotifyTyping );
	snt->go( true );
}

void Client::sendWebcamInvite( const QString &who )
{
	if( !d->webcamTask->transmitting() )
		d->webcamTask->registerWebcam();

	d->webcamTask->addPendingInvitation( who );
}

void Client::sendMessage( const QString &to, const QString &msg )
{
	SendMessageTask *smt = new SendMessageTask( d->root );
	smt->setTarget( to );
	smt->setText( msg );
	smt->setPicureFlag( pictureFlag() );
	smt->go( true );
}

void Client::setChatSessionState( const QString &to, bool close )
{
	ChatSessionTask *cst = new ChatSessionTask( d->root );
	cst->setTarget( to );
	cst->setType( close ? ChatSessionTask::UnregisterSession : ChatSessionTask::RegisterSession );
	cst->go( true );
}

void Client::sendBuzz( const QString &to )
{
	SendMessageTask *smt = new SendMessageTask( d->root );
	smt->setTarget( to );
	smt->setText( QString::fromLatin1( "<ding>" ) );
	smt->setPicureFlag( pictureFlag() );
	smt->go( true );
}

void Client::sendFile( unsigned int transferId, const QString &to, const QString &msg, KURL url )
{
	SendFileTask *sft = new SendFileTask( d->root );

	QObject::connect( sft, SIGNAL(complete(unsigned int)), SIGNAL(fileTransferComplete(unsigned int)) );
	QObject::connect( sft, SIGNAL(bytesProcessed(unsigned int, unsigned int)), SIGNAL(fileTransferBytesProcessed(unsigned int, unsigned int)) );
	QObject::connect( sft, SIGNAL(error(unsigned int, int, const QString &)), SIGNAL(fileTransferError(unsigned int, int, const QString &)) );

	QObject::connect( this, SIGNAL(fileTransferCanceled( unsigned int )), sft, SLOT(canceled( unsigned int )) );

	sft->setTarget( to );
	sft->setMessage( msg );
	sft->setFileUrl( url );
	sft->setTransferId( transferId );
	sft->go( true );
}

void Client::receiveFile( unsigned int transferId, const QString &userId, KURL remoteURL, KURL localURL )
{
	ReceiveFileTask *rft = new ReceiveFileTask( d->root );

	QObject::connect( rft, SIGNAL(complete(unsigned int)), SIGNAL(fileTransferComplete(unsigned int)) );
	QObject::connect( rft, SIGNAL(bytesProcessed(unsigned int, unsigned int)), SIGNAL(fileTransferBytesProcessed(unsigned int, unsigned int)) );
	QObject::connect( rft, SIGNAL(error(unsigned int, int, const QString &)), SIGNAL(fileTransferError(unsigned int, int, const QString &)) );
	QObject::connect( this, SIGNAL(fileTransferCanceled( unsigned int )), rft, SLOT(canceled( unsigned int )) );

	rft->setRemoteUrl( remoteURL );
	rft->setLocalUrl( localURL );
	rft->setTransferId( transferId );
	rft->setUserId( userId );
	if( remoteURL.url().startsWith( "http://" ) )
		rft->setType( ReceiveFileTask::FileTransferAccept );
	else
		rft->setType( ReceiveFileTask::FileTransfer7Accept );
	rft->go( true );
}

void Client::rejectFile( const QString &userId, KURL remoteURL )
{
	if( remoteURL.url().startsWith( "http://" ) )
		return;

	ReceiveFileTask *rft = new ReceiveFileTask( d->root );

	rft->setRemoteUrl( remoteURL );
	rft->setUserId( userId );
	rft->setType( ReceiveFileTask::FileTransfer7Reject );
	rft->go( true );
}

void Client::cancelFileTransfer( unsigned int transferId )
{
	emit fileTransferCanceled( transferId );
}

void Client::changeStatus( Yahoo::Status status, const QString &message, Yahoo::StatusType type )
{
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << "status: " << status
					<< " message: " << message
					<< " type: " << type << endl;	
	ChangeStatusTask *cst = new ChangeStatusTask( d->root );
	cst->setStatus( status );
	cst->setMessage( message );
	cst->setType( type );
	cst->go( true );
	
	if( status == Yahoo::StatusInvisible )
		stealthContact( QString::null, Yahoo::StealthOnline, Yahoo::StealthClear );

	setStatus( status );
}

void Client::sendAuthReply( const QString &userId, bool accept, const QString &msg )
{
	SendAuthRespTask *sarp = new SendAuthRespTask( d->root );
	sarp->setGranted( accept );
	sarp->setTarget( userId );
	sarp->setMessage( msg );
	sarp->go( true );
}

void Client::sendPing()
{
	if( !d->active )
	{
		kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << "Disconnected. NOT sending a PING." << endl;
		return;
	}
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << "Sending a PING." << endl;
	PingTask *pt = new PingTask( d->root );
	pt->go( true );
}

// ***** Contactlist handling *****

void Client::stealthContact(QString const &userId, Yahoo::StealthMode mode, Yahoo::StealthStatus state)
{
	StealthTask *st = new StealthTask( d->root );
	st->setTarget( userId );
	st->setState( state );
	st->setMode( mode );
	st->go( true );
}

void Client::addBuddy( const QString &userId, const QString &group, const QString &message )
{
	ModifyBuddyTask *mbt = new ModifyBuddyTask( d->root );
	mbt->setType( ModifyBuddyTask::AddBuddy );
	mbt->setTarget( userId );
	mbt->setGroup( group );
	mbt->setMessage( message );
	mbt->go( true );
}

void Client::removeBuddy( const QString &userId, const QString &group )
{
	ModifyBuddyTask *mbt = new ModifyBuddyTask( d->root );
	mbt->setType( ModifyBuddyTask::RemoveBuddy );
	mbt->setTarget( userId );
	mbt->setGroup( group );
	mbt->go( true );
}

void Client::moveBuddy( const QString &userId, const QString &oldGroup, const QString &newGroup )
{
	ModifyBuddyTask *mbt = new ModifyBuddyTask( d->root );
	mbt->setType( ModifyBuddyTask::MoveBuddy );
	mbt->setTarget( userId );
	mbt->setOldGroup( oldGroup );
	mbt->setGroup( newGroup );
	mbt->go( true );
}

// ***** Buddyicon handling *****

void Client::requestPicture( const QString &userId )
{
	RequestPictureTask *rpt = new RequestPictureTask( d->root );
	rpt->setTarget( userId );
	rpt->go( true );
}

void Client::downloadPicture(  const QString &userId, KURL url, int checksum )
{
	if( !d->iconLoader )
	{
		d->iconLoader = new YahooBuddyIconLoader( this );
		QObject::connect( d->iconLoader, SIGNAL(fetchedBuddyIcon(const QString&, KTempFile*, int )),
				SIGNAL(pictureDownloaded(const QString&, KTempFile*,  int ) ) );
	}

	d->iconLoader->fetchBuddyIcon( QString(userId), KURL(url), checksum );
}

void Client::uploadPicture( KURL url )
{
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << "URL: " << url.url() << endl;
	SendPictureTask *spt = new SendPictureTask( d->root );
	spt->setType( SendPictureTask::UploadPicture );
	spt->setFilename( url.fileName() );
	if ( url.isLocalFile() )
		spt->setPath( url.path() );
	else
		spt->setPath( url.url() );
	d->pictureFlag = 2;
	spt->go( true );
}

void Client::sendPictureChecksum( int checksum, const QString &who )
{
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << "checksum: " << checksum << endl;
	SendPictureTask *spt = new SendPictureTask( d->root );
	spt->setType( SendPictureTask::SendChecksum );
	spt->setChecksum( checksum );
	if( !who.isEmpty() )
		spt->setTarget( who );
	spt->go( true );	
}

void Client::sendPictureInformation( const QString &userId, const QString &url, int checksum )
{
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << "checksum: " << checksum << endl;
	SendPictureTask *spt = new SendPictureTask( d->root );
	spt->setType( SendPictureTask::SendInformation );
	spt->setChecksum( checksum );
	spt->setUrl( url );
	spt->setTarget( userId );
	spt->go( true );
}

void Client::sendPictureStatusUpdate( const QString &userId, int type )
{
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << "Setting PictureStatus to: " << type << endl;
	SendPictureTask *spt = new SendPictureTask( d->root );
	spt->setType( SendPictureTask::SendStatus );
	spt->setStatus( type );
	spt->setTarget( userId );
	spt->go( true );
}

// ***** Webcam handling *****

void Client::requestWebcam( const QString &userId )
{
	d->webcamTask->requestWebcam( userId );
}

void Client::closeWebcam( const QString &userId )
{
	d->webcamTask->closeWebcam( userId );
}

void Client::sendWebcamImage( const QByteArray &ar )
{
	d->webcamTask->sendWebcamImage( ar );
}

void Client::closeOutgoingWebcam()
{
	d->webcamTask->closeOutgoingWebcam();
}


void Client::grantWebcamAccess( const QString &userId )
{
	d->webcamTask->grantAccess( userId );
}

// ***** Conferences *****
void Client::inviteConference( const QString &room, const QStringList &members, const QString &msg )
{
	d->conferenceTask->inviteConference( room, members, msg );
}

void Client::addInviteConference( const QString &room, const QStringList &who, const QStringList &members, const QString &msg )
{
	d->conferenceTask->addInvite( room, who, members, msg );
}

void Client::joinConference( const QString &room, const QStringList &members )
{
	d->conferenceTask->joinConference( room, members );
}

void Client::declineConference( const QString &room, const QStringList &members, const QString &msg )
{
	d->conferenceTask->declineConference( room, members, msg );
}

void Client::leaveConference( const QString &room, const QStringList &members )
{
	d->conferenceTask->leaveConference( room, members );
}

void Client::sendConferenceMessage( const QString &room, const QStringList &members, const QString &msg )
{
	d->conferenceTask->sendMessage( room, members, msg );
}

// ***** YAB *****
void Client::getYABEntries( long lastMerge, long lastRemoteRevision )
{
	d->yabTask->getAllEntries( lastMerge, lastRemoteRevision);
}

void Client::saveYABEntry( YABEntry &entry )
{
	ModifyYABTask *myt = new ModifyYABTask( d->root );
	myt->setAction( ModifyYABTask::EditEntry );
	myt->setEntry( entry );
	QObject::connect( myt, SIGNAL(gotEntry( YABEntry * )), this, SIGNAL( gotYABEntry( YABEntry * ) ) );
	QObject::connect( myt, SIGNAL(error( YABEntry *, const QString &)), this, SIGNAL(modifyYABEntryError( YABEntry *, const QString & )));
	myt->go(true);
}

void Client::addYABEntry(  YABEntry &entry )
{
	ModifyYABTask *myt = new ModifyYABTask( d->root );
	myt->setAction( ModifyYABTask::AddEntry );
	myt->setEntry( entry );
	QObject::connect( myt, SIGNAL(gotEntry( YABEntry * )), this, SIGNAL( gotYABEntry( YABEntry * ) ) );
	QObject::connect( myt, SIGNAL(error( YABEntry *, const QString &)), this, SIGNAL(modifyYABEntryError( YABEntry *, const QString & )));
	myt->go(true);
}

void Client::deleteYABEntry(  YABEntry &entry )
{
	ModifyYABTask *myt = new ModifyYABTask( d->root );
	myt->setAction( ModifyYABTask::DeleteEntry );
	myt->setEntry( entry );
	myt->go(true);
}

// ***** other *****
void Client::notifyError( const QString &info, const QString & errorString, LogLevel level )
{
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << QString::fromLatin1("\nThe following error occured: %1\n    Reason: %2\n    LogLevel: %3")
		.arg(info).arg(errorString).arg(level) << endl;
	d->errorString = errorString;
	d->errorInformation = info;
	emit error( level );
}

QString Client::userId()
{
	return d->user;
}

void Client::setUserId( const QString & userId )
{
	d->user = userId;
}

Yahoo::Status Client::status()
{
	return d->status;
}

void Client::setStatus( Yahoo::Status status )
{
	d->status = status;
}


void Client::setStatusOnConnect( Yahoo::Status status )
{
	d->statusOnConnect = status;
}

void Client::setStatusMessageOnConnect( const QString &msg )
{
	d->statusMessageOnConnect = msg;
}

void Client::setVerificationWord( const QString &word )
{
	d->loginTask->setVerificationWord( word );
}

QString Client::password()
{
	return d->pass;
}

QCString Client::ipAddress()
{
	//TODO determine ip address
	return "127.0.0.1";
}

QString Client::host()
{
	return d->host;
}

int Client::port()
{
	return d->port;
}

uint Client::sessionID()
{
	return d->sessionID;
}

int Client::pictureFlag()
{
	return d->pictureFlag;
}

void Client::setPictureFlag( int flag )
{
	d->pictureFlag = flag;
}

QString Client::yCookie()
{
	return d->yCookie;
}

QString Client::tCookie()
{
	return d->tCookie;
}

QString Client::cCookie()
{
	return d->cCookie;
}

void Client::distribute( Transfer * transfer )
{
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;
	if( !rootTask()->take( transfer ) )
		kdDebug(YAHOO_RAW_DEBUG) << "CLIENT: root task refused transfer" << endl;
	delete transfer;
}

void Client::send( Transfer* request )
{
	kdDebug(YAHOO_RAW_DEBUG) << "CLIENT::send()"<< endl;
	if( !d->stream )
	{	
		kdDebug(YAHOO_RAW_DEBUG) << "CLIENT - NO STREAM TO SEND ON!" << endl;
		return;
	}

	d->stream->write( request );
}

void Client::debug(const QString &str)
{
	qDebug( "CLIENT: %s", str.ascii() );
}

Task * Client::rootTask()
{
	return d->root;
}

void Client::initTasks()
{
	if( d->tasksInitialized )
		return;

	d->statusTask = new StatusNotifierTask( d->root );
	QObject::connect( d->statusTask, SIGNAL( statusChanged( const QString&, int, const QString&, int, int ) ), 
				SIGNAL( statusChanged( const QString&, int, const QString&, int, int ) ) );
	QObject::connect( d->statusTask, SIGNAL( stealthStatusChanged( const QString&, Yahoo::StealthStatus ) ), 
				SIGNAL( stealthStatusChanged( const QString&, Yahoo::StealthStatus ) ) );
	QObject::connect( d->statusTask, SIGNAL( loginResponse( int, const QString& ) ), 
				SLOT( slotLoginResponse( int, const QString& ) ) );
	QObject::connect( d->statusTask, SIGNAL( authorizationRejected( const QString&, const QString& ) ), 
				SIGNAL( authorizationRejected( const QString&, const QString& ) ) );
	QObject::connect( d->statusTask, SIGNAL( authorizationAccepted( const QString& ) ), 
				SIGNAL( authorizationAccepted( const QString& ) ) );
	QObject::connect( d->statusTask, SIGNAL( gotAuthorizationRequest( const QString &, const QString &, const QString & ) ), 
				SIGNAL( gotAuthorizationRequest( const QString &, const QString &, const QString & ) ) );
	QObject::connect( d->statusTask, SIGNAL( gotPictureChecksum( const QString &, int ) ),
				SIGNAL( pictureChecksumNotify( const QString &, int ) ) );

	d->mailTask = new MailNotifierTask( d->root );
	QObject::connect( d->mailTask, SIGNAL( mailNotify(const QString&, const QString&, int) ), 
				SIGNAL( mailNotify(const QString&, const QString&, int) ) );

	d->messageReceiverTask = new MessageReceiverTask( d->root );
	QObject::connect( d->messageReceiverTask, SIGNAL( gotIm(const QString&, const QString&, long, int) ),
				SIGNAL( gotIm(const QString&, const QString&, long, int) ) );
	QObject::connect( d->messageReceiverTask, SIGNAL( systemMessage(const QString&) ),
				SIGNAL( systemMessage(const QString&) ) );
	QObject::connect( d->messageReceiverTask, SIGNAL( gotTypingNotify(const QString &, int) ),
				SIGNAL( typingNotify(const QString &, int) ) );
	QObject::connect( d->messageReceiverTask, SIGNAL( gotBuzz( const QString &, long ) ),
				SIGNAL( gotBuzz( const QString &, long ) ) );
	QObject::connect( d->messageReceiverTask, SIGNAL( gotWebcamInvite(const QString &) ),
				SIGNAL( gotWebcamInvite(const QString &) ) );

	d->pictureNotifierTask = new PictureNotifierTask( d->root );
	QObject::connect( d->pictureNotifierTask, SIGNAL( pictureStatusNotify( const QString &, int ) ),
				SIGNAL( pictureStatusNotify( const QString &, int ) ) );
	QObject::connect( d->pictureNotifierTask, SIGNAL( pictureChecksumNotify( const QString &, int ) ),
				SIGNAL( pictureChecksumNotify( const QString &, int ) ) );
	QObject::connect( d->pictureNotifierTask, SIGNAL( pictureInfoNotify( const QString &, KURL, int ) ),
				SIGNAL( pictureInfoNotify( const QString &, KURL, int ) ) );
	QObject::connect( d->pictureNotifierTask, SIGNAL( pictureRequest( const QString & ) ),
				SIGNAL( pictureRequest( const QString & ) ) );
	QObject::connect( d->pictureNotifierTask, SIGNAL( pictureUploaded( const QString & ) ),
				SIGNAL( pictureUploaded( const QString & ) ) );

	d->webcamTask = new WebcamTask( d->root );
	QObject::connect( d->webcamTask, SIGNAL( webcamImageReceived( const QString &, const QPixmap &) ),
				SIGNAL( webcamImageReceived( const QString &, const QPixmap &) ) );
	QObject::connect( d->webcamTask, SIGNAL( webcamNotAvailable( const QString & ) ),
				SIGNAL( webcamNotAvailable( const QString & ) ) );
	QObject::connect( d->webcamTask, SIGNAL( webcamClosed( const QString &, int ) ),
				SIGNAL( webcamClosed( const QString &, int ) ) );
	QObject::connect( d->webcamTask, SIGNAL( webcamPaused(const QString&) ),
				SIGNAL( webcamPaused(const QString&) ) );
	QObject::connect( d->webcamTask, SIGNAL( readyForTransmission() ),
				SIGNAL( webcamReadyForTransmission() ) );
	QObject::connect( d->webcamTask, SIGNAL( stopTransmission() ),
				SIGNAL( webcamStopTransmission() ) );
	QObject::connect( d->webcamTask, SIGNAL( viewerJoined( const QString &) ),
				SIGNAL( webcamViewerJoined( const QString &) ) );
	QObject::connect( d->webcamTask, SIGNAL( viewerLeft( const QString &) ),
				SIGNAL( webcamViewerLeft( const QString &) ) );
	QObject::connect( d->webcamTask, SIGNAL( viewerRequest( const QString &) ),
				SIGNAL( webcamViewerRequest( const QString &) ) );

	d->conferenceTask = new ConferenceTask( d->root );
	QObject::connect( d->conferenceTask, SIGNAL( gotInvite( const QString &, const QString &, const QString &, const QStringList & ) ),
				SIGNAL( gotConferenceInvite( const QString &, const QString &, const QString &, const QStringList & ) ) );
	QObject::connect( d->conferenceTask, SIGNAL( gotMessage( const QString &, const QString &, const QString & ) ),
				SIGNAL( gotConferenceMessage( const QString &, const QString &, const QString & ) ) );
	QObject::connect( d->conferenceTask, SIGNAL( userJoined( const QString &, const QString & ) ),
				SIGNAL( confUserJoined( const QString &, const QString & ) ) );
	QObject::connect( d->conferenceTask, SIGNAL( userLeft( const QString &, const QString & ) ),
				SIGNAL( confUserLeft( const QString &, const QString & ) ) );
	QObject::connect( d->conferenceTask, SIGNAL( userDeclined( const QString &, const QString &, const QString & ) ),
				SIGNAL( confUserDeclined( const QString &, const QString &, const QString & ) ) );

	d->yabTask = new YABTask( d->root );
	QObject::connect( d->yabTask, SIGNAL( gotEntry( YABEntry * ) ),
				SIGNAL( gotYABEntry( YABEntry * ) ) );
	QObject::connect( d->yabTask, SIGNAL( gotRevision( long, bool ) ),
				SIGNAL( gotYABRevision( long, bool ) ) );

	d->fileTransferTask = new FileTransferNotifierTask( d->root );
	QObject::connect( d->fileTransferTask, SIGNAL(incomingFileTransfer( const QString &, const QString &, 
					long, const QString &, const QString &, unsigned long )),
				SIGNAL(incomingFileTransfer( const QString &, const QString &, 
					long, const QString &, const QString &, unsigned long )) );
}

void Client::deleteTasks()
{
	d->tasksInitialized = false;
	d->statusTask->deleteLater();
	d->statusTask = 0L;
	d->mailTask->deleteLater();
	d->mailTask = 0L;
	d->messageReceiverTask->deleteLater();
	d->messageReceiverTask = 0L;
	d->pictureNotifierTask->deleteLater();
	d->pictureNotifierTask = 0L;
	d->webcamTask->deleteLater();
	d->webcamTask = 0L;
	d->conferenceTask->deleteLater();
	d->conferenceTask = 0L;
	d->yabTask->deleteLater();
	d->yabTask = 0L;
	d->fileTransferTask->deleteLater();
	d->fileTransferTask = 0;
}

#include "client.moc"
