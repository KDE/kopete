/*
    Kopete Yahoo Protocol
    
    Copyright (c) 2005-2006 André Duffeck <duffeck@kde.org>
    Copyright (c) 2004 Duncan Mac-Vicar P. <duncan@kde.org>
    Copyright (c) 2004 Matt Rogers <matt.rogers@kdemail.net>
    Copyright (c) 2004 SuSE Linux AG <http://www.suse.com>
    Copyright (C) 2003  Justin Karneges <justin@affinix.com>
    
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

#include "client.h"

#include <QTimer>
#include <QPixmap>

#include <kdebug.h>
#include <k3socketbase.h>

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
#include "alivetask.h"
#include "yabtask.h"
#include "modifyyabtask.h"
#include "chatsessiontask.h"
#include "sendfiletask.h"
#include "filetransfernotifiertask.h"
#include "receivefiletask.h"
#include "yahoochattask.h"
#include "yahootypes.h"
#include "yahoobuddyiconloader.h"

using namespace KNetwork;

namespace KYahoo {

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
	QSet<QString> stealthedBuddies;
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
	YahooChatTask *yahooChatTask;
	ReceiveFileTask *receiveFileTask;

	// Connection data
	uint sessionID;
	QString yCookie;
	QString tCookie;
	QString cCookie;
	Yahoo::Status status;
	Yahoo::Status statusOnConnect;
	QString statusMessageOnConnect;
	Yahoo::PictureStatus pictureFlag;
	int pictureChecksum;
	bool buddyListReady;
	QStringList pictureRequestQueue;
};

Client::Client(QObject *par) :QObject(par)
{
	setObjectName( QLatin1String("yahooclient") );
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
	d->pictureFlag = Yahoo::NoPicture;
	d->buddyListReady = false;
	m_connector = 0L;

	m_pingTimer = new QTimer( this );
	QObject::connect( m_pingTimer, SIGNAL(timeout()), this, SLOT(sendPing()) );
	m_aliveTimer = new QTimer( this );
	QObject::connect( m_aliveTimer, SIGNAL(timeout()), this, SLOT(sendAlive()) );
	
	QObject::connect( d->loginTask, SIGNAL(haveSessionID(uint)), SLOT(lt_gotSessionID(uint)) );
	QObject::connect( d->loginTask, SIGNAL(buddyListReady()), SLOT(processPictureQueue()) );
	QObject::connect( d->loginTask, SIGNAL(loginResponse(int,QString)), 
				SLOT(slotLoginResponse(int,QString)) );
	QObject::connect( d->loginTask, SIGNAL(haveCookies()), SLOT(slotGotCookies()) );
	QObject::connect( d->listTask, SIGNAL(gotBuddy(QString,QString,QString)), 
					SIGNAL(gotBuddy(QString,QString,QString)) );
	QObject::connect( d->listTask, SIGNAL(stealthStatusChanged(QString,Yahoo::StealthStatus)), 
					SLOT(notifyStealthStatusChanged(QString,Yahoo::StealthStatus)) );
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
	kDebug(YAHOO_RAW_DEBUG) ;
	d->host = host;
	d->port = port;
	d->user = userId;
	d->pass = pass;
	setStatus( Yahoo::StatusConnecting );

	m_connector = new KNetworkConnector;
	m_connector->setOptHostPort( host, port );
	d->stream = new ClientStream( m_connector, this );
	QObject::connect( d->stream, SIGNAL(connected()), this, SLOT(cs_connected()) );
	QObject::connect( d->stream, SIGNAL(error(int)), this, SLOT(streamError(int)) );
	QObject::connect( d->stream, SIGNAL(readyRead()), this, SLOT(streamReadyRead()) );
	QObject::connect( d->stream, SIGNAL(connectionClosed()), this, SLOT(streamDisconnected()) );
	
	d->stream->connectToServer( host, false );
}

void Client::cancelConnect()
{
	d->loginTask->reset();
}

void Client::cs_connected()
{
	kDebug(YAHOO_RAW_DEBUG) ;
	emit connected();
	kDebug(YAHOO_RAW_DEBUG) << " starting login task ... ";

	// Clear stealth settings
	d->stealthedBuddies.clear();

	d->loginTask->setStateOnConnect( (d->statusOnConnect == Yahoo::StatusInvisible) ? Yahoo::StatusInvisible : Yahoo::StatusAvailable );
	d->loginTask->go();
	d->active = true;
}

void Client::close()
{
	kDebug(YAHOO_RAW_DEBUG) ;
	m_pingTimer->stop();
	m_aliveTimer->stop();
	if( d->active )
	{
		LogoffTask *lt = new LogoffTask( d->root );
		lt->go( true );
	}
	if( d->tasksInitialized)
		deleteTasks();	
	d->loginTask->reset();
	if( d->stream ) {
		QObject::disconnect( d->stream, SIGNAL(readyRead()), this, SLOT(streamReadyRead()) );
		d->stream->deleteLater();
	}
	d->stream = 0L;
	if( m_connector )
		m_connector->deleteLater();
	m_connector = 0L;
	d->active = false;
	d->buddyListReady = false;
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
	kDebug(YAHOO_RAW_DEBUG) << "CLIENT ERROR (Error " <<  error << ")";
	QString msg;

	d->active = false;

	// Examine error
	if( error == ClientStream::ErrConnection && m_connector )			// Ask Connector in this case
	{
		d->error = m_connector->errorCode();
		d->errorString = KSocketBase::errorString( (KSocketBase::SocketError)d->error );
	}
	else if( d->stream )
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

void Client::streamDisconnected()
{
	d->active = false;
	emit disconnected();
}

void Client::lt_loginFinished()
{
	kDebug(YAHOO_RAW_DEBUG) ;

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
		d->statusMessageOnConnect.clear();
		setStatus( d->statusOnConnect );
		/* YM Client sends a alert every minute
		* If we time out and try to log out and log in
		* we can lose messages therefore we should set this
		* to the same as the Yahoo Messenger client
		*. so as we do not get disconnected
		* Also it sends a PING at every hour.
		*/
		m_aliveTimer->start( 1 * 60 * 1000 );
		m_pingTimer->start( 59 * 60 * 1000 );
		initTasks();
	} else {
		d->active = false;
		close();
	}

	kDebug(YAHOO_RAW_DEBUG) << "Emitting loggedIn";
	emit loggedIn( response, msg );
}

void Client::lt_gotSessionID( uint id )
{
	kDebug(YAHOO_RAW_DEBUG) << "Got SessionID: " << id;	
	d->sessionID = id;
}

void Client::slotGotCookies()
{
	kDebug(YAHOO_RAW_DEBUG) << "Y: " << d->loginTask->yCookie()
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
	smt->setText( QLatin1String( "<ding>" ) );
	smt->setPicureFlag( pictureFlag() );
	smt->go( true );
}

void Client::sendFile( unsigned int transferId, const QString &to, const QString &msg, KUrl url )
{
	SendFileTask *sft = new SendFileTask( d->root );

	QObject::connect( sft, SIGNAL(complete(uint)), SIGNAL(fileTransferComplete(uint)) );
	QObject::connect( sft, SIGNAL(bytesProcessed(uint,uint)), SIGNAL(fileTransferBytesProcessed(uint,uint)) );
	QObject::connect( sft, SIGNAL(error(uint,int,QString)), SIGNAL(fileTransferError(uint,int,QString)) );

	QObject::connect( this, SIGNAL(fileTransferCanceled(uint)), sft, SLOT(canceled(uint)) );

	sft->setTarget( to );
	sft->setMessage( msg );
	sft->setFileUrl( url );
	sft->setTransferId( transferId );
	sft->go( true );
}

void Client::receiveFile( unsigned int transferId, const QString &userId, KUrl remoteURL, KUrl localURL )
{
	ReceiveFileTask *rft = new ReceiveFileTask( d->root );

	QObject::connect( rft, SIGNAL(complete(uint)), SIGNAL(fileTransferComplete(uint)) );
	QObject::connect( rft, SIGNAL(bytesProcessed(uint,uint)), SIGNAL(fileTransferBytesProcessed(uint,uint)) );
	QObject::connect( rft, SIGNAL(error(uint,int,QString)), SIGNAL(fileTransferError(uint,int,QString)) );
	QObject::connect( this, SIGNAL(fileTransferCanceled(uint)), rft, SLOT(canceled(uint)) );

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

void Client::rejectFile( const QString &userId, KUrl remoteURL )
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
	kDebug(YAHOO_RAW_DEBUG) << "status: " << status
					<< " message: " << message
					<< " type: " << type << endl;	
	ChangeStatusTask *cst = new ChangeStatusTask( d->root );
	cst->setStatus( status );
	cst->setMessage( message );
	cst->setType( type );
	cst->go( true );
	
	if( status == Yahoo::StatusInvisible )
		stealthContact( QString(), Yahoo::StealthOnline, Yahoo::StealthClear );

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
		kDebug(YAHOO_RAW_DEBUG) << "Disconnected. NOT sending a PING.";
		return;
	}
	kDebug(YAHOO_RAW_DEBUG) << "Sending a PING.";
	PingTask *pt = new PingTask( d->root );
	pt->go( true );
}

void Client::sendAlive()
{
	if( !d->active )
	{
		kDebug(YAHOO_RAW_DEBUG) << "Disconnected. NOT sending a ALIVE.";
		return;
	}
	kDebug(YAHOO_RAW_DEBUG) << "Sending a ALIVE.";
	AliveTask *at = new AliveTask( d->root );
	at->go( true );
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
	
	QObject::connect(mbt, SIGNAL(buddyAddResult(QString,QString,bool)),
			 SIGNAL(buddyAddResult(QString,QString,bool)));

	mbt->setType( ModifyBuddyTask::AddBuddy );
	mbt->setTarget( userId );
	mbt->setGroup( group );
	mbt->setMessage( message );
	mbt->go( true );
}

void Client::removeBuddy( const QString &userId, const QString &group )
{
	ModifyBuddyTask *mbt = new ModifyBuddyTask( d->root );

	QObject::connect(mbt, SIGNAL(buddyRemoveResult(QString,QString,bool)),
			 SIGNAL(buddyRemoveResult(QString,QString,bool)));

	mbt->setType( ModifyBuddyTask::RemoveBuddy );
	mbt->setTarget( userId );
	mbt->setGroup( group );
	mbt->go( true );
}

void Client::moveBuddy( const QString &userId, const QString &oldGroup, const QString &newGroup )
{
	ModifyBuddyTask *mbt = new ModifyBuddyTask( d->root );

	QObject::connect(mbt, SIGNAL(buddyChangeGroupResult(QString,QString,bool)),
			 SIGNAL(buddyChangeGroupResult(QString,QString,bool)));

	mbt->setType( ModifyBuddyTask::MoveBuddy );
	mbt->setTarget( userId );
	mbt->setOldGroup( oldGroup );
	mbt->setGroup( newGroup );
	mbt->go( true );
}

// ***** Buddyicon handling *****

void Client::processPictureQueue()
{
	kDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;
	d->buddyListReady = true;
	if( d->pictureRequestQueue.isEmpty() )
	{
		return;
	}

	requestPicture( d->pictureRequestQueue.front() );
	d->pictureRequestQueue.pop_front();

	
	if( !d->pictureRequestQueue.isEmpty() )
	{
		QTimer::singleShot( 1000, this, SLOT(processPictureQueue()) );
	}
}

void Client::requestPicture( const QString &userId )
{
	if( !d->buddyListReady )
	{
		d->pictureRequestQueue << userId;
		return;
	}

	RequestPictureTask *rpt = new RequestPictureTask( d->root );
	rpt->setTarget( userId );
	rpt->go( true );
}

void Client::downloadPicture(  const QString &userId, KUrl url, int checksum )
{
	if( !d->iconLoader )
	{
		d->iconLoader = new YahooBuddyIconLoader( this );
		QObject::connect( d->iconLoader, SIGNAL(fetchedBuddyIcon(QString,QByteArray,int)),
				SIGNAL(pictureDownloaded(QString,QByteArray,int)) );
	}

	d->iconLoader->fetchBuddyIcon( QString(userId), KUrl(url), checksum );
}

void Client::uploadPicture( KUrl url )
{
	kDebug(YAHOO_RAW_DEBUG) << "URL: " << url.url();
	SendPictureTask *spt = new SendPictureTask( d->root );
	spt->setType( SendPictureTask::UploadPicture );
	spt->setFilename( url.fileName() );
	if ( url.isLocalFile() )
		spt->setPath( url.toLocalFile() );
	else
		spt->setPath( url.url() );
	spt->go( true );
}

void Client::sendPictureChecksum( const QString &userId, int checksum )
{
	kDebug(YAHOO_RAW_DEBUG) << "checksum: " << checksum;
	SendPictureTask *spt = new SendPictureTask( d->root );
	spt->setType( SendPictureTask::SendChecksum );
	spt->setChecksum( checksum );
	if( !userId.isEmpty() )
		spt->setTarget( userId );
	spt->go( true );	
}

void Client::sendPictureInformation( const QString &userId, const QString &url, int checksum )
{
	kDebug(YAHOO_RAW_DEBUG) << "checksum: " << checksum;
	SendPictureTask *spt = new SendPictureTask( d->root );
	spt->setType( SendPictureTask::SendInformation );
	spt->setChecksum( checksum );
	spt->setUrl( url );
	spt->setTarget( userId );
	spt->go( true );
}

void Client::setPictureStatus( Yahoo::PictureStatus status )
{
	if( d->pictureFlag == status )
		return;

	kDebug(YAHOO_RAW_DEBUG) << "Setting PictureStatus to: " << status;
	d->pictureFlag = status;
	SendPictureTask *spt = new SendPictureTask( d->root );
	spt->setType( SendPictureTask::SendStatus );
	spt->setStatus( status );
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
	QObject::connect( myt, SIGNAL(gotEntry(YABEntry*)), this, SIGNAL(gotYABEntry(YABEntry*)) );
	QObject::connect( myt, SIGNAL(error(YABEntry*,QString)), this, SIGNAL(modifyYABEntryError(YABEntry*,QString)));
	myt->go(true);
}

void Client::addYABEntry(  YABEntry &entry )
{
	ModifyYABTask *myt = new ModifyYABTask( d->root );
	myt->setAction( ModifyYABTask::AddEntry );
	myt->setEntry( entry );
	QObject::connect( myt, SIGNAL(gotEntry(YABEntry*)), this, SIGNAL(gotYABEntry(YABEntry*)) );
	QObject::connect( myt, SIGNAL(error(YABEntry*,QString)), this, SIGNAL(modifyYABEntryError(YABEntry*,QString)));
	myt->go(true);
}

void Client::deleteYABEntry(  YABEntry &entry )
{
	ModifyYABTask *myt = new ModifyYABTask( d->root );
	myt->setAction( ModifyYABTask::DeleteEntry );
	myt->setEntry( entry );
	myt->go(true);
}

// ***** Yahoo Chat *****
void Client::getYahooChatCategories()
{
	d->yahooChatTask->getYahooChatCategories();
}

void Client::getYahooChatRooms( const Yahoo::ChatCategory &category )
{
	d->yahooChatTask->getYahooChatRooms( category );
}

void Client::joinYahooChatRoom( const Yahoo::ChatRoom &room )
{
	d->yahooChatTask->joinRoom( room );
}

void Client::sendYahooChatMessage( const QString &msg, const QString &handle )
{
	d->yahooChatTask->sendYahooChatMessage( msg, handle );
}

void Client::leaveChat()
{
	d->yahooChatTask->logout();
}

// ***** other *****
void Client::notifyError( const QString &info, const QString & errorString, LogLevel level )
{
	kDebug(YAHOO_RAW_DEBUG) << QString::fromLatin1("\nThe following error occurred: %1\n    Reason: %2\n    LogLevel: %3")
		.arg(info).arg(errorString).arg(level) << endl;
	d->errorString = errorString;
	d->errorInformation = info;
	emit error( level );
}

Yahoo::StealthStatus Client::stealthStatus( const QString &userId ) const
{
	if ( d->stealthedBuddies.contains( userId ) )
		return Yahoo::StealthActive;
	else
		return Yahoo::StealthNotActive;
}

void Client::notifyStealthStatusChanged( const QString &userId, Yahoo::StealthStatus state )
{
	if ( state == Yahoo::StealthActive )
		d->stealthedBuddies.insert( userId );
	else
		d->stealthedBuddies.remove( userId );
	
	emit stealthStatusChanged( userId, state );
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

int Client::pictureChecksum()
{
	return d->pictureChecksum;
}

void Client::setPictureChecksum( int cs )
{
	d->pictureChecksum = cs;
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
	kDebug(YAHOO_RAW_DEBUG) ;
	if( !rootTask()->take( transfer ) )
		kDebug(YAHOO_RAW_DEBUG) << "CLIENT: root task refused transfer";
	delete transfer;
}

void Client::send( Transfer* request )
{
	kDebug(YAHOO_RAW_DEBUG) << "CLIENT::send()";
	if( !d->stream )
	{	
		kDebug(YAHOO_RAW_DEBUG) << "CLIENT - NO STREAM TO SEND ON!";
		return;
	}

	d->stream->write( request );
}

void Client::debug(const QString &str)
{
       qDebug( "CLIENT: %s", qPrintable(str) );
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
	QObject::connect( d->statusTask, SIGNAL(statusChanged(QString,int,QString,int,int,int)), 
				SIGNAL(statusChanged(QString,int,QString,int,int,int)) );
	QObject::connect( d->statusTask, SIGNAL(stealthStatusChanged(QString,Yahoo::StealthStatus)), 
				SLOT(notifyStealthStatusChanged(QString,Yahoo::StealthStatus)) );
	QObject::connect( d->statusTask, SIGNAL(loginResponse(int,QString)), 
				SLOT(slotLoginResponse(int,QString)) );
	QObject::connect( d->statusTask, SIGNAL(authorizationRejected(QString,QString)), 
				SIGNAL(authorizationRejected(QString,QString)) );
	QObject::connect( d->statusTask, SIGNAL(authorizationAccepted(QString)), 
				SIGNAL(authorizationAccepted(QString)) );
	QObject::connect( d->statusTask, SIGNAL(gotAuthorizationRequest(QString,QString,QString)), 
				SIGNAL(gotAuthorizationRequest(QString,QString,QString)) );

	d->mailTask = new MailNotifierTask( d->root );
	QObject::connect( d->mailTask, SIGNAL(mailNotify(QString,QString,int)), 
				SIGNAL(mailNotify(QString,QString,int)) );

	d->messageReceiverTask = new MessageReceiverTask( d->root );
	QObject::connect( d->messageReceiverTask, SIGNAL(gotIm(QString,QString,long,int)),
				SIGNAL(gotIm(QString,QString,long,int)) );
	QObject::connect( d->messageReceiverTask, SIGNAL(systemMessage(QString)),
				SIGNAL(systemMessage(QString)) );
	QObject::connect( d->messageReceiverTask, SIGNAL(gotTypingNotify(QString,int)),
				SIGNAL(typingNotify(QString,int)) );
	QObject::connect( d->messageReceiverTask, SIGNAL(gotBuzz(QString,long)),
				SIGNAL(gotBuzz(QString,long)) );
	QObject::connect( d->messageReceiverTask, SIGNAL(gotWebcamInvite(QString)),
				SIGNAL(gotWebcamInvite(QString)) );

	d->pictureNotifierTask = new PictureNotifierTask( d->root );
	QObject::connect( d->pictureNotifierTask, SIGNAL(pictureStatusNotify(QString,int)),
				SIGNAL(pictureStatusNotify(QString,int)) );
	QObject::connect( d->pictureNotifierTask, SIGNAL(pictureChecksumNotify(QString,int)),
				SIGNAL(pictureChecksumNotify(QString,int)) );
	QObject::connect( d->pictureNotifierTask, SIGNAL(pictureInfoNotify(QString,KUrl,int)),
				SIGNAL(pictureInfoNotify(QString,KUrl,int)) );
	QObject::connect( d->pictureNotifierTask, SIGNAL(pictureRequest(QString)),
				SIGNAL(pictureRequest(QString)) );
	QObject::connect( d->pictureNotifierTask, SIGNAL(pictureUploaded(QString,int)),
				SIGNAL(pictureUploaded(QString,int)) );

	d->webcamTask = new WebcamTask( d->root );
	QObject::connect( d->webcamTask, SIGNAL(webcamImageReceived(QString,QPixmap)),
				SIGNAL(webcamImageReceived(QString,QPixmap)) );
	QObject::connect( d->webcamTask, SIGNAL(webcamNotAvailable(QString)),
				SIGNAL(webcamNotAvailable(QString)) );
	QObject::connect( d->webcamTask, SIGNAL(webcamClosed(QString,int)),
				SIGNAL(webcamClosed(QString,int)) );
	QObject::connect( d->webcamTask, SIGNAL(webcamPaused(QString)),
				SIGNAL(webcamPaused(QString)) );
	QObject::connect( d->webcamTask, SIGNAL(readyForTransmission()),
				SIGNAL(webcamReadyForTransmission()) );
	QObject::connect( d->webcamTask, SIGNAL(stopTransmission()),
				SIGNAL(webcamStopTransmission()) );
	QObject::connect( d->webcamTask, SIGNAL(viewerJoined(QString)),
				SIGNAL(webcamViewerJoined(QString)) );
	QObject::connect( d->webcamTask, SIGNAL(viewerLeft(QString)),
				SIGNAL(webcamViewerLeft(QString)) );
	QObject::connect( d->webcamTask, SIGNAL(viewerRequest(QString)),
				SIGNAL(webcamViewerRequest(QString)) );

	d->conferenceTask = new ConferenceTask( d->root );
	QObject::connect( d->conferenceTask, SIGNAL(gotInvite(QString,QString,QString,QStringList)),
				SIGNAL(gotConferenceInvite(QString,QString,QString,QStringList)) );
	QObject::connect( d->conferenceTask, SIGNAL(gotMessage(QString,QString,QString)),
				SIGNAL(gotConferenceMessage(QString,QString,QString)) );
	QObject::connect( d->conferenceTask, SIGNAL(userJoined(QString,QString)),
				SIGNAL(confUserJoined(QString,QString)) );
	QObject::connect( d->conferenceTask, SIGNAL(userLeft(QString,QString)),
				SIGNAL(confUserLeft(QString,QString)) );
	QObject::connect( d->conferenceTask, SIGNAL(userDeclined(QString,QString,QString)),
				SIGNAL(confUserDeclined(QString,QString,QString)) );

	d->yabTask = new YABTask( d->root );
	QObject::connect( d->yabTask, SIGNAL(gotEntry(YABEntry*)),
				SIGNAL(gotYABEntry(YABEntry*)) );
	QObject::connect( d->yabTask, SIGNAL(gotRevision(long,bool)),
				SIGNAL(gotYABRevision(long,bool)) );

	d->fileTransferTask = new FileTransferNotifierTask( d->root );
	QObject::connect( d->fileTransferTask, SIGNAL(incomingFileTransfer( const QString &, const QString &, 
					long, const QString &, const QString &, unsigned long, const QPixmap & )),
				SIGNAL(incomingFileTransfer( const QString &, const QString &, 
					long, const QString &, const QString &, unsigned long, const QPixmap & )) );

	d->yahooChatTask = new YahooChatTask( d->root );
	QObject::connect( d->yahooChatTask, SIGNAL(gotYahooChatCategories(QDomDocument)),
				SIGNAL(gotYahooChatCategories(QDomDocument)) );
	QObject::connect( d->yahooChatTask, SIGNAL(gotYahooChatRooms(Yahoo::ChatCategory,QDomDocument)),
				SIGNAL(gotYahooChatRooms(Yahoo::ChatCategory,QDomDocument)) );
	QObject::connect( d->yahooChatTask, SIGNAL(chatRoomJoined(int,int,QString,QString)),
				SIGNAL(chatRoomJoined(int,int,QString,QString)) );
	QObject::connect( d->yahooChatTask, SIGNAL(chatBuddyHasJoined(QString,QString,bool)),
				SIGNAL(chatBuddyHasJoined(QString,QString,bool)) );
	QObject::connect( d->yahooChatTask, SIGNAL(chatBuddyHasLeft(QString,QString)),
				SIGNAL(chatBuddyHasLeft(QString,QString)) );
	QObject::connect( d->yahooChatTask, SIGNAL(chatMessageReceived(QString,QString,QString)),
				SIGNAL(chatMessageReceived(QString,QString,QString)) );
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
	d->yahooChatTask->deleteLater();
	d->yahooChatTask = 0;
	d->receiveFileTask->deleteLater();
	d->receiveFileTask = 0;
}

}

#include "client.moc"
