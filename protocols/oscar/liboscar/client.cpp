/*
	client.cpp - Kopete Oscar Protocol

	Copyright (c) 2004-2005 Matt Rogers <mattr@kde.org>
    Copyright (c) 2008 Roman Jarosz <kedgedev@centrum.cz>

	Based on code Copyright (c) 2004 SuSE Linux AG <http://www.suse.com>
	Based on Iris, Copyright (C) 2003  Justin Karneges <justin@affinix.com>

	Kopete (c) 2002-2008 by the Kopete developers <kopete-devel@kde.org>

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
#include <QList>
#include <QByteArray>
#include <QPointer>
#include <QTextCodec>
#include <QtNetwork/QSslSocket>

#include <kdebug.h> //for kDebug()
#include <klocale.h>

#include "filetransfertask.h"
#include "buddyicontask.h"
#include "clientreadytask.h"
#include "connectionhandler.h"
#include "chatnavservicetask.h"
#include "errortask.h"
#include "icquserinfo.h"
#include "icquserinfotask.h"
#include "logintask.h"
#include "connection.h"
#include "messagereceivertask.h"
#include "messageacktask.h"
#include "onlinenotifiertask.h"
#include "oscarclientstream.h"
#include "oscarsettings.h"
#include "oscarutils.h"
#include "ownuserinfotask.h"
#include "profiletask.h"
#include "senddcinfotask.h"
#include "sendmessagetask.h"
#include "serverredirecttask.h"
#include "servicesetuptask.h"
#include "contactmanager.h"
#include "ssimodifytask.h"
#include "ssiauthtask.h"
#include "offlinemessagestask.h"
#include "task.h"
#include "typingnotifytask.h"
#include "userinfotask.h"
#include "usersearchtask.h"
#include "warningtask.h"
#include "chatservicetask.h"
#include "rateclassmanager.h"
#include "icquserinfoupdatetask.h"
#include "icqchangepasswordtask.h"
#include "oscarmessageplugin.h"
#include "xtrazxtraznotify.h"
#include "xtrazxawayservice.h"
#include "closeconnectiontask.h"
#include "icqtlvinforequesttask.h"
#include "icqtlvinfoupdatetask.h"
#include "filetransferhandler.h"
#include "chatroomtask.h"
#include "chatroomhandler.h"


namespace
{
	class DefaultCodecProvider : public Client::CodecProvider
	{
	public:
		virtual QTextCodec* codecForContact( const QString& ) const
		{
			return QTextCodec::codecForMib( 4 );
		}
		virtual QTextCodec* codecForAccount() const
		{
			return QTextCodec::codecForMib( 4 );
		}
	};

	DefaultCodecProvider defaultCodecProvider;
}

namespace Oscar {

class Client::ClientPrivate
{
public:
	ClientPrivate() {}

	QString host, user, pass;
	uint port;
	bool encrypted;
	bool encrypted2;
	QString SSLName;
	int tzoffset;
	bool active;

	enum { StageOne, StageTwo };
	int stage;

	StageOneLoginTask* loginTask;
	QPointer<StageTwoLoginTask> loginTaskTwo;

	//Protocol specific data
	bool isIcq;
	bool redirectRequested;
	QList<Oscar::WORD> redirectionServices;
	Oscar::WORD currentRedirect;
	bool offlineMessagesRequested;
	QByteArray cookie;
	Oscar::Settings* settings;

	//Tasks
	ErrorTask* errorTask;
	OnlineNotifierTask* onlineNotifier;
	OwnUserInfoTask* ownStatusTask;
	MessageReceiverTask* messageReceiverTask;
	MessageAckTask* messageAckTask;
	SSIAuthTask* ssiAuthTask;
	ICQUserInfoRequestTask* icqInfoTask;
	ICQTlvInfoRequestTask* icqTlvInfoTask;
	UserInfoTask* userInfoTask;
	TypingNotifyTask * typingNotifyTask;
	SSIModifyTask* ssiModifyTask;
	//Managers
	ContactManager* ssiManager;
	ConnectionHandler connections;

	//Our Userinfo
	UserDetails ourDetails;

    //Infos
    QList<int> exchanges;

	struct Status
	{
		Oscar::DWORD status;
		QString message;     // for away-,DND-message etc., and for Xtraz status
		int xtraz;           // Xtraz status
		int mood;            // Mood
		QString title;       // Xtraz/Mood title
		bool sent;
	} status;

	//away messages
	struct AwayMsgRequest
	{
		QString contact;
		ICQStatus contactStatus;
	};
	QList<AwayMsgRequest> awayMsgRequestQueue;
	QTimer* awayMsgRequestTimer;
	CodecProvider* codecProvider;
	
	const Oscar::ClientVersion* version;
	Guid versionCap;
};

Client::Client( QObject* parent )
:QObject( parent )
{
	setObjectName( "oscarclient" );

	d = new ClientPrivate;
	d->tzoffset = 0;
	d->active = false;
	d->isIcq = false; //default to AIM
	d->redirectRequested = false;
	d->currentRedirect = 0;
	d->offlineMessagesRequested = false;
	d->status.status = 0x0; // default to online
	d->status.xtraz = -1; // default to no Xtraz
	d->status.mood = -1;
	d->status.sent = false;
	d->ssiManager = new ContactManager( this );
	d->settings = new Oscar::Settings();
	d->errorTask = 0L;
	d->onlineNotifier = 0L;
	d->ownStatusTask = 0L;
	d->messageReceiverTask = 0L;
	d->messageAckTask = 0L;
	d->ssiAuthTask = 0L;
	d->icqInfoTask = 0L;
	d->icqTlvInfoTask = 0L;
	d->userInfoTask = 0L;
	d->stage = ClientPrivate::StageOne;
	d->loginTask = 0L;
	d->loginTaskTwo = 0L;
	d->typingNotifyTask = 0L;
	d->ssiModifyTask = 0L;
	d->awayMsgRequestTimer = new QTimer();
	d->codecProvider = &defaultCodecProvider;

	connect( this, SIGNAL(redirectionFinished(Oscar::WORD)),
	         this, SLOT(checkRedirectionQueue(Oscar::WORD)) );
	connect( d->awayMsgRequestTimer, SIGNAL(timeout()),
	         this, SLOT(nextICQAwayMessageRequest()) );
}

Client::~Client()
{

	//delete the connections differently than in deleteConnections()
	//deleteLater() seems to cause destruction order issues
	deleteStaticTasks();
    delete d->settings;
	delete d->ssiManager;
	delete d->awayMsgRequestTimer;
	delete d;
}

Oscar::Settings* Client::clientSettings() const
{
	return d->settings;
}

void Client::connectToServer( const QString& host, quint16 port, bool encrypted, const QString &name )
{
	ClientStream* cs = createClientStream();
	Connection* c = new Connection( cs, "AUTHORIZER" );
	c->setClient( this );

	d->encrypted2 = encrypted;

	d->loginTask = new StageOneLoginTask( c->rootTask() );
	connect( d->loginTask, SIGNAL(finished()), this, SLOT(lt_loginFinished()) );
	connectToServer( c, host, port, encrypted, name );
}

void Client::start( const QString &host, const uint port, const QString &userId, const QString &pass )
{
	Q_UNUSED( host );
	Q_UNUSED( port );

	// Cleanup client
	close();

	d->user = userId;
	d->pass = pass;
	d->stage = ClientPrivate::StageOne;
	d->active = false;
}

void Client::close()
{
	QList<Connection*> cList = d->connections.connections();
	for ( int i = 0; i < cList.size(); i++ )
	{
		Connection* c = cList.at(i);
		(new CloseConnectionTask( c->rootTask() ))->go( Task::AutoDelete );
		
		foreach ( Oscar::MessageInfo info, c->messageInfoList() )
			emit messageError( info.contact, info.id );
	}

	d->active = false;
	d->awayMsgRequestTimer->stop();
	d->awayMsgRequestQueue.clear();
	d->connections.clear();

	deleteStaticTasks();

	//don't clear the stored status between stage one and two
	if ( d->stage == ClientPrivate::StageTwo )
	{
		d->status.status = 0x0;
		d->status.xtraz = -1;
		d->status.mood = -1;
		d->status.sent = false;
		d->status.message.clear();
		d->status.title.clear();
	}

	d->exchanges.clear();
	d->redirectRequested = false;
	d->currentRedirect = 0;
	d->redirectionServices.clear();
	d->ssiManager->clear();
	d->offlineMessagesRequested = false;
}

void Client::setStatus( Oscar::DWORD status, const QString &message, int xtraz, const QString &title, int mood )
{
	kDebug(OSCAR_RAW_DEBUG) << "Setting status message to "<< message;

	// remember the values to reply with, when requested
	bool xtrazChanged = (xtraz > -1 || d->status.xtraz != xtraz);
	bool moodChanged = (mood > -1 || d->status.mood != mood);
	bool statusInfoChanged = ( !d->status.sent || message != d->status.message || title != d->status.title );
	d->status.status = status;
	d->status.message = message;
	d->status.xtraz = xtraz;
	d->status.mood = mood;
	d->status.title = title;
	d->status.sent = false;

	if ( d->active )
	{
		if ( d->isIcq )
		{
			// Set invisible/visible flag
			Oscar::BYTE privacyByte = ( ( status & 0x0100 ) == 0x0100 ) ? 0x03 : 0x04;
			setPrivacyTLVs( privacyByte );
		}
		
		Connection* c = d->connections.connectionForFamily( 0x0002 );
		if ( !c )
			return;

		if ( d->isIcq && statusInfoChanged )
		{
			ICQFullInfo info( false );
			info.statusDescription.set( title.toUtf8() );
			
			ICQTlvInfoUpdateTask* infoUpdateTask = new ICQTlvInfoUpdateTask( c->rootTask() );
			infoUpdateTask->setInfo( info );
			infoUpdateTask->go( Task::AutoDelete );
		}

		SendDCInfoTask* sdcit = new SendDCInfoTask( c->rootTask(), status );
		if ( d->isIcq && moodChanged )
			sdcit->setIcqMood( mood );

		if ( d->isIcq && statusInfoChanged )
			sdcit->setStatusMessage( title );
		if ( !d->isIcq && (status & 0xFF) == 0x00 ) //not icq and status is online
			sdcit->setStatusMessage( message );

		QString msg;
		// AIM: you're away exactly when your away message isn't empty.
		// can't use QString() as a message either; ProfileTask
		// interprets null as "don't change".
		if ( (status & 0xFF) == 0x00 ) //is status online?
		{
			msg = QString::fromAscii("");
		}
		else
		{
			if ( message.isEmpty() )
				msg = QString::fromAscii(" ");
			else
				msg = message;
		}

		ProfileTask* pt = new ProfileTask( c->rootTask() );
		pt->setAwayMessage( msg );

		if ( d->isIcq && xtrazChanged )
			pt->setXtrazStatus( xtraz );

		pt->go( Task::AutoDelete );
		//Has to be sent after ProfileTask otherwise the EA, DND will be wrong
		sdcit->go( Task::AutoDelete );

		d->status.sent = true;
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

bool Client::encrypted()
{
	return d->encrypted;
}

QString Client::SSLName()
{
	return d->SSLName;
}

ContactManager* Client::ssiManager() const
{
	return d->ssiManager;
}

const Oscar::ClientVersion* Client::version() const
{
	return d->version;
}

Guid Client::versionCap() const
{
	return d->versionCap;
}

// SLOTS //

void Client::streamConnected()
{
	kDebug(OSCAR_RAW_DEBUG) ;
	if ( d->loginTaskTwo )
		d->loginTaskTwo->go( Task::AutoDelete );
}

void Client::lt_loginFinished()
{
	/* Check for stage two login first, since we create the stage two
	 * task when we finish stage one
	 */
	if ( d->stage == ClientPrivate::StageTwo )
	{
		//we've finished logging in. start the services setup
		kDebug(OSCAR_RAW_DEBUG) << "stage two done. setting up services";
		initializeStaticTasks();
		ServiceSetupTask* ssTask = new ServiceSetupTask( d->connections.defaultConnection()->rootTask() );
		connect( ssTask, SIGNAL(finished()), this, SLOT(serviceSetupFinished()) );
		ssTask->go( Task::AutoDelete ); //fire and forget
	}
	else if ( d->stage == ClientPrivate::StageOne )
	{
		kDebug(OSCAR_RAW_DEBUG) << "stage one login done";
		disconnect( d->loginTask, SIGNAL(finished()), this, SLOT(lt_loginFinished()) );

		if ( d->loginTask->statusCode() == 0 ) //we can start stage two
		{
			kDebug(OSCAR_RAW_DEBUG) << "no errors from stage one. moving to stage two";

			//cache these values since they'll be deleted when we close the connections (which deletes the tasks)
			d->host = d->loginTask->bosServer();
			d->port = d->loginTask->bosPort().toUInt();
			d->encrypted = d->loginTask->bosEncrypted();
			d->SSLName = d->loginTask->bosSSLName();
			d->cookie = d->loginTask->loginCookie();
			close();
			QTimer::singleShot( 100, this, SLOT(startStageTwo()) );
			d->stage = ClientPrivate::StageTwo;
		}
		else
		{
			kDebug(OSCAR_RAW_DEBUG) << "errors reported. not moving to stage two";
			close(); //deletes the connections for us
		}
		d->loginTask->deleteLater();
		d->loginTask = 0;
	}

}

void Client::startStageTwo()
{
	//create a new connection and set it up
	Connection* c = createConnection();
	new CloseConnectionTask( c->rootTask() );

	//create the new login task
	d->loginTaskTwo = new StageTwoLoginTask( c->rootTask() );
	d->loginTaskTwo->setCookie( d->cookie );
	QObject::connect( d->loginTaskTwo, SIGNAL(finished()), this, SLOT(lt_loginFinished()) );

	//connect
	QObject::connect( c, SIGNAL(connected()), this, SLOT(streamConnected()) );
	connectToServer( c, d->host, d->port, d->encrypted, d->SSLName ) ;

}

void Client::serviceSetupFinished()
{
	d->active = true;

	setStatus( d->status.status, d->status.message, d->status.xtraz, d->status.title, d->status.mood );
	d->ownStatusTask->go();

	emit haveContactList();
	emit loggedIn();
}

void Client::receivedIcqInfo( const QString& contact, unsigned int type )
{
	kDebug(OSCAR_RAW_DEBUG) << "received icq info for " << contact
		<< " of type " << type << endl;

	if ( type == ICQUserInfoRequestTask::Short )
		emit receivedIcqShortInfo( contact );
	else
		emit receivedIcqLongInfo( contact );
}

void Client::receivedInfo( Oscar::DWORD sequence )
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
	kDebug( OSCAR_RAW_DEBUG );
	UserDetails ud = d->ownStatusTask->getInfo();
	d->ourDetails = ud;
	emit haveOwnInfo();

	if ( !d->offlineMessagesRequested && d->active )
	{
		//retrieve offline messages
		Connection* c = d->connections.connectionForFamily( 0x0004 );
		if ( !c )
			return;

		OfflineMessagesTask *offlineMsgTask = new OfflineMessagesTask( c->rootTask() );
		offlineMsgTask->go( Task::AutoDelete );
		d->offlineMessagesRequested = true;
	}
}

void Client::setCodecProvider( Client::CodecProvider* codecProvider )
{
	d->codecProvider = codecProvider;
}

void Client::setVersion( const Oscar::ClientVersion* version )
{
	d->version = version;
}

void Client::setVersionCap( const QByteArray &cap )
{
	d->versionCap = Guid( cap );
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

int Client::statusXtraz() const
{
	return d->status.xtraz;
}

int Client::statusMood() const
{
	return d->status.mood;
}

QString Client::statusTitle() const
{
	return d->status.title;
}

QString Client::statusMessage() const
{
	return d->status.message;
}

void Client::setStatusMessage( const QString &message )
{
	d->status.message = message;
}

QByteArray Client::ipAddress() const
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
    Connection* c = 0L;
    if ( msg.channel() == 0x0003 )
    {
        c = d->connections.connectionForChatRoom( msg.exchange(), msg.chatRoom() );
        if ( !c )
            return;

	    kDebug(OSCAR_RAW_DEBUG) << "sending message to chat room: " << msg.chatRoom() << " on exchange " << msg.exchange();
        ChatServiceTask* cst = new ChatServiceTask( c->rootTask(), msg.exchange(), msg.chatRoom() );
        cst->setMessage( msg );
        cst->setEncoding( d->codecProvider->codecForAccount()->name() );
        cst->go( Task::AutoDelete );
    }
    else
    {
        c = d->connections.connectionForFamily( 0x0004 );
        if ( !c )
            return;
        SendMessageTask *sendMsgTask = new SendMessageTask( c->rootTask() );
        // Set whether or not the message is an automated response
        sendMsgTask->setAutoResponse( isAuto );
        sendMsgTask->setMessage( msg );
        sendMsgTask->go( Task::AutoDelete );
    }
}

void Client::receivedMessage( const Oscar::Message& msg )
{
	if ( msg.channel() == 2 && !msg.hasProperty( Oscar::Message::AutoResponse ) )
	{
		// channel 2 message needs an autoresponse, regardless of type
		Connection* c = d->connections.connectionForFamily( 0x0004 );
		if ( !c )
			return;

		Oscar::Message response ( msg );
		if ( msg.hasProperty( Oscar::Message::StatusMessageRequest ) )
		{
			kDebug( OSCAR_RAW_DEBUG ) << "Away message request";
			QTextCodec* codec = d->codecProvider->codecForContact( msg.sender() );
			response.setText( Oscar::Message::UserDefined, statusMessage(), codec );
			emit userReadsStatusMessage( msg.sender() );
		}
		else if ( msg.messageType() == Oscar::MessageType::Plugin )
		{
			Oscar::MessagePlugin::Types type = msg.plugin()->type();
			Oscar::WORD subType = msg.plugin()->subTypeId();
			if ( type == Oscar::MessagePlugin::XtrazScript )
			{
				if ( subType == Oscar::MessagePlugin::SubScriptNotify )
				{
					using namespace Xtraz;
					XtrazNotify xNotify;
					xNotify.handle( msg.plugin() );
					if ( xNotify.type() == XtrazNotify::Request && xNotify.pluginId() == "srvMng" )
					{
						if ( xNotify.findService( "cAwaySrv" ) )
						{
							XtrazNotify xNotifyResponse;
							xNotifyResponse.setSenderUni( userId() );
							response.setPlugin( xNotifyResponse.statusResponse( statusXtraz(), statusTitle(), statusMessage() ) );
							emit userReadsStatusMessage( msg.sender() );
						}
					}
				}
			}
			else if ( type == Oscar::MessagePlugin::StatusMsgExt )
			{
				Buffer buffer;

				buffer.addLEDBlock( statusMessage().toUtf8() );
				//TODO: Change this to text/x-aolrtf
				buffer.addLEDBlock( "text/plain" );

				msg.plugin()->setData( buffer.buffer() );
				emit userReadsStatusMessage( msg.sender() );
			}
		}
		else
		{
			response.setEncoding( Oscar::Message::UserDefined );
			response.setTextArray( QByteArray() );
		}
		response.setReceiver( msg.sender() );
		response.addProperty( Oscar::Message::AutoResponse );
		SendMessageTask *sendMsgTask = new SendMessageTask( c->rootTask() );
		sendMsgTask->setMessage( response );
		sendMsgTask->go( Task::AutoDelete );
	}

	if ( msg.hasProperty( Oscar::Message::AutoResponse ) )
	{
		if ( msg.hasProperty( Oscar::Message::StatusMessageRequest ) )
		{
			// we got a response to a status message request.
			QString awayMessage( msg.text( d->codecProvider->codecForContact( msg.sender() ) ) );
			kDebug( OSCAR_RAW_DEBUG ) << "Received an away message: " << awayMessage;
			emit receivedAwayMessage( msg.sender(), awayMessage );
		}
		else if ( msg.messageType() == Oscar::MessageType::Plugin )
		{
			kDebug( OSCAR_RAW_DEBUG ) << "Received an plugin message response.";

			Oscar::MessagePlugin::Types type = msg.plugin()->type();
			Oscar::WORD subType = msg.plugin()->subTypeId();
			if ( type == Oscar::MessagePlugin::XtrazScript )
			{
				if ( subType == Oscar::MessagePlugin::SubScriptNotify )
				{
					using namespace Xtraz;
					XtrazNotify xNotify;
					xNotify.handle( msg.plugin() );
					if ( xNotify.type() == XtrazNotify::Response )
					{
						const Xtraz::XAwayService* service = dynamic_cast<const XAwayService*>(xNotify.findService( "cAwaySrv" ));
						if ( service )
							emit receivedXStatusMessage( service->senderId(), service->iconIndex(),
							                             service->description(), service->message() );
					}
				}
			}
			else if ( type == Oscar::MessagePlugin::StatusMsgExt )
			{
				// we got a response to a status message request.
				Buffer buffer( msg.plugin()->data() );

				QString awayMessage = QString::fromUtf8( buffer.getLEDBlock() );
				kDebug( OSCAR_RAW_DEBUG ) << "Received an away message: " << awayMessage;
				emit receivedAwayMessage( msg.sender(), awayMessage );
			}
		}
	}
	else
	{
		if ( msg.messageType() == Oscar::MessageType::Plugin )
		{
			kDebug( OSCAR_RAW_DEBUG ) << "Received a plugin message.";
		}
		else if ( !msg.hasProperty( Oscar::Message::StatusMessageRequest ) )
		{
			// Filter out miranda's invisible check
			if ( msg.messageType() == 0x0004 && msg.textArray().isEmpty() )
				return;

			// let application handle it
			kDebug( OSCAR_RAW_DEBUG ) << "Emitting receivedMessage";
			emit messageReceived( msg );
		}
	}
}

void Client::fileMessage( const Oscar::Message& msg )
{
	Connection* c = d->connections.connectionForFamily( 0x0004 );
	if ( !c )
		return;

	kDebug( OSCAR_RAW_DEBUG ) << "internal ip: " << c->localAddress().toString();
	kDebug( OSCAR_RAW_DEBUG ) << "external ip: " << ourInfo().dcExternalIp().toString();
	
	SendMessageTask *sendMsgTask = new SendMessageTask( c->rootTask() );
	// Set whether or not the message is an automated response
	sendMsgTask->setAutoResponse( false );
	sendMsgTask->setMessage( msg );
	sendMsgTask->setIp( c->localAddress().toIPv4Address() );
	sendMsgTask->go( Task::AutoDelete );
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
//	qDebug( "CLIENT: %s", str.toAscii() );
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
	d->messageAckTask = new MessageAckTask( c->rootTask() );
	d->ssiAuthTask = new SSIAuthTask( c->rootTask() );
	d->icqInfoTask = new ICQUserInfoRequestTask( c->rootTask() );
	d->icqTlvInfoTask = new ICQTlvInfoRequestTask( c->rootTask() );
	d->userInfoTask = new UserInfoTask( c->rootTask() );
	d->typingNotifyTask = new TypingNotifyTask( c->rootTask() );
	d->ssiModifyTask = new SSIModifyTask( c->rootTask(), true );

	connect( d->onlineNotifier, SIGNAL(userIsOnline(QString,UserDetails)),
	         this, SIGNAL(receivedUserInfo(QString,UserDetails)) );
	connect( d->onlineNotifier, SIGNAL(userIsOffline(QString,UserDetails)),
	         this, SLOT(offlineUser(QString,UserDetails)) );

	connect( d->ownStatusTask, SIGNAL(gotInfo()), this, SLOT(haveOwnUserInfo()) );
	connect( d->ownStatusTask, SIGNAL(buddyIconUploadRequested()), this,
	         SIGNAL(iconNeedsUploading()) );

	connect( d->messageReceiverTask, SIGNAL(receivedMessage(Oscar::Message)),
	         this, SLOT(receivedMessage(Oscar::Message)) );
	connect( d->messageReceiverTask, SIGNAL(fileMessage(int,QString,QByteArray,Buffer)),
	         this, SLOT(gotFileMessage(int,QString,QByteArray,Buffer)) );
	connect( d->messageReceiverTask, SIGNAL(chatroomMessage(Oscar::Message,QByteArray)),
	         this, SLOT(gotChatRoomMessage(Oscar::Message,QByteArray)) );

	connect( d->messageAckTask, SIGNAL(messageAck(QString,uint)),
	         this, SIGNAL(messageAck(QString,uint)) );
	connect( d->errorTask, SIGNAL(messageError(QString,uint)),
	         this, SIGNAL(messageError(QString,uint)) );
	
	connect( d->ssiAuthTask, SIGNAL(authRequested(QString,QString)),
	         this, SIGNAL(authRequestReceived(QString,QString)) );
	connect( d->ssiAuthTask, SIGNAL(authReplied(QString,QString,bool)),
	         this, SIGNAL(authReplyReceived(QString,QString,bool)) );

	connect( d->icqInfoTask, SIGNAL(receivedInfoFor(QString,uint)),
	         this, SLOT(receivedIcqInfo(QString,uint)) );
	connect( d->icqTlvInfoTask, SIGNAL(receivedInfoFor(QString)),
	         this, SIGNAL(receivedIcqTlvInfo(QString)) );

	connect( d->userInfoTask, SIGNAL(receivedProfile(QString,QString)),
	         this, SIGNAL(receivedProfile(QString,QString)) );
	connect( d->userInfoTask, SIGNAL(receivedAwayMessage(QString,QString)),
	         this, SIGNAL(receivedAwayMessage(QString,QString)) );
	connect( d->typingNotifyTask, SIGNAL(typingStarted(QString)),
	         this, SIGNAL(userStartedTyping(QString)) );
	connect( d->typingNotifyTask, SIGNAL(typingFinished(QString)),
	         this, SIGNAL(userStoppedTyping(QString)) );
}

void Client::removeGroup( const QString& groupName )
{
	Connection* c = d->connections.connectionForFamily( 0x0013 );
	if ( !c )
		return;

	kDebug( OSCAR_RAW_DEBUG ) << "Removing group " << groupName << " from Contact";
	SSIModifyTask* ssimt = new SSIModifyTask( c->rootTask() );
	if ( ssimt->removeGroup( groupName ) )
		ssimt->go( Task::AutoDelete );
	else
		delete ssimt;
}

void Client::addGroup( const QString& groupName )
{
	Connection* c = d->connections.connectionForFamily( 0x0013 );
	if ( !c )
		return;

	kDebug( OSCAR_RAW_DEBUG ) << "Adding group " << groupName << " to Contact";
	SSIModifyTask* ssimt = new SSIModifyTask( c->rootTask() );
	if ( ssimt->addGroup( groupName ) )
		ssimt->go( Task::AutoDelete );
	else
		delete ssimt;
}

void Client::addContact( const QString& contactName, const QString& groupName )
{
	Connection* c = d->connections.connectionForFamily( 0x0013 );
	if ( !c )
		return;

	kDebug( OSCAR_RAW_DEBUG ) << "Adding contact " << contactName << " to ssi in group " << groupName;
	SSIModifyTask* ssimt = new SSIModifyTask( c->rootTask() );
	if ( ssimt->addContact( contactName, groupName )  )
		ssimt->go( Task::AutoDelete );
	else
		delete ssimt;
}

void Client::removeContact( const QString& contactName )
{
	Connection* c = d->connections.connectionForFamily( 0x0013 );
	if ( !c )
		return;

	kDebug( OSCAR_RAW_DEBUG ) << "Removing contact " << contactName << " from ssi";
	SSIModifyTask* ssimt = new SSIModifyTask( c->rootTask() );
	if ( ssimt->removeContact( contactName ) )
		ssimt->go( Task::AutoDelete );
	else
		delete ssimt;
}

void Client::renameGroup( const QString & oldGroupName, const QString & newGroupName )
{
	Connection* c = d->connections.connectionForFamily( 0x0013 );
	if ( !c )
		return;

	kDebug( OSCAR_RAW_DEBUG ) << "Renaming group " << oldGroupName << " to " << newGroupName;
	SSIModifyTask* ssimt = new SSIModifyTask( c->rootTask() );
	if ( ssimt->renameGroup( oldGroupName, newGroupName ) )
		ssimt->go( Task::AutoDelete );
	else
		delete ssimt;
}

void Client::modifyContactItem( const OContact& oldItem, const OContact& newItem )
{
	int action = 0; //0 modify, 1 add, 2 remove TODO cleanup!
	Connection* c = d->connections.connectionForFamily( 0x0013 );
	if ( !c )
		return;

	if ( !oldItem && newItem )
		action = 1;
	if ( oldItem && !newItem )
		action = 2;

	kDebug(OSCAR_RAW_DEBUG) << "Add/Mod/Del item on server";
	SSIModifyTask* ssimt = new SSIModifyTask( c->rootTask() );
	switch ( action )
	{
	case 0:
		if ( ssimt->modifyItem( oldItem, newItem ) )
			ssimt->go( Task::AutoDelete );
		else
			delete ssimt;
		break;
	case 1:
		if ( ssimt->addItem( newItem ) )
			ssimt->go( Task::AutoDelete );
		else
			delete ssimt;
		break;
	case 2:
		if ( ssimt->removeItem( oldItem ) )
			ssimt->go( Task::AutoDelete );
		else
			delete ssimt;
		break;
	}
}

void Client::changeContactGroup( const QString& contact, const QString& newGroupName )
{
	Connection* c = d->connections.connectionForFamily( 0x0013 );
	if ( !c )
		return;

	kDebug(OSCAR_RAW_DEBUG) << "Changing " << contact << "'s group to "
		<< newGroupName << endl;
	SSIModifyTask* ssimt = new SSIModifyTask( c->rootTask() );
	if ( ssimt->changeGroup( contact, newGroupName ) )
		ssimt->go( Task::AutoDelete );
	else
		delete ssimt;
}

void Client::changeContactAlias( const QString& contact, const QString& alias )
{
	Connection* c = d->connections.connectionForFamily( 0x0013 );
	if ( !c )
		return;
	
	OContact item = ssiManager()->findContact( contact );
	if ( item )
	{
		OContact oldItem(item);

		if ( alias.isEmpty() )
		{
			QList<TLV> tList( item.tlvList() );
			TLV tlv = Oscar::findTLV( tList, 0x0131 );
			if ( !tlv )
				return;

			tList.removeAll( tlv );
			item.setTLVList( tList );
		}
		else
		{
			QList<TLV> tList;

			QByteArray data = alias.toUtf8();
			tList.append( TLV( 0x0131, data.size(), data ) );

			if ( !Oscar::updateTLVs( item, tList ) )
				return;
		}

		kDebug( OSCAR_RAW_DEBUG ) << "Changing " << contact << "'s alias to " << alias;
		SSIModifyTask* ssimt = new SSIModifyTask( c->rootTask() );
		if ( ssimt->modifyContact( oldItem, item ) )
			ssimt->go( Task::AutoDelete );
		else
			delete ssimt;
	}
}

void Client::setPrivacyTLVs( Oscar::BYTE privacy, Oscar::DWORD userClasses )
{
	OContact item = ssiManager()->findItem( QString(), ROSTER_VISIBILITY );

	QList<Oscar::TLV> tList;
	tList.append( TLV( 0x00CA, 1, (char *)&privacy ) );
	tList.append( TLV( 0x00CB, sizeof(userClasses), (char *)&userClasses ) );

	if ( !item )
	{
		kDebug( OSCAR_RAW_DEBUG ) << "Adding new privacy TLV item";
		QString empty;
		OContact s( empty, 0, ssiManager()->nextContactId(), ROSTER_VISIBILITY, tList );
		modifyContactItem( item, s );
	}
	else
	{ //found an item
		OContact s(item);

		if ( Oscar::updateTLVs( s, tList ) == true )
		{
			kDebug( OSCAR_RAW_DEBUG ) << "Updating privacy TLV item";
			modifyContactItem( item, s );
		}
	}
}

void Client::requestShortTlvInfo( const QString& contactId, const QByteArray &metaInfoId )
{
	Connection* c = d->connections.connectionForFamily( 0x0015 );
	if ( !c )
		return;

	d->icqTlvInfoTask->setUser( Oscar::normalize( contactId ) );
	d->icqTlvInfoTask->setMetaInfoId( metaInfoId );
	d->icqTlvInfoTask->setType( ICQTlvInfoRequestTask::Short );
	d->icqTlvInfoTask->go();
}

void Client::requestMediumTlvInfo( const QString& contactId, const QByteArray &metaInfoId )
{
	Connection* c = d->connections.connectionForFamily( 0x0015 );
	if ( !c )
		return;

	d->icqTlvInfoTask->setUser( Oscar::normalize( contactId ) );
	d->icqTlvInfoTask->setMetaInfoId( metaInfoId );
	d->icqTlvInfoTask->setType( ICQTlvInfoRequestTask::Medium );
	d->icqTlvInfoTask->go();
}

void Client::requestLongTlvInfo( const QString& contactId, const QByteArray &metaInfoId )
{
	Connection* c = d->connections.connectionForFamily( 0x0015 );
	if ( !c )
		return;

	d->icqTlvInfoTask->setUser( Oscar::normalize( contactId ) );
	d->icqTlvInfoTask->setMetaInfoId( metaInfoId );
	d->icqTlvInfoTask->setType( ICQTlvInfoRequestTask::Long );
	d->icqTlvInfoTask->go();
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
	QObject::connect( warnTask, SIGNAL(userWarned(QString,quint16,quint16)),
	                  this, SIGNAL(userWarned(QString,quint16,quint16)) );
	warnTask->go( Task::AutoDelete );
}

bool Client::changeICQPassword( const QString& password )
{
	Connection* c = d->connections.connectionForFamily( 0x0015 );
	if ( !c )
		return false;

	ICQChangePasswordTask* task = new ICQChangePasswordTask( c->rootTask() );
	QObject::connect( task, SIGNAL(finished()), this, SLOT(changeICQPasswordFinished()) );
	task->setPassword( password );
	task->go( Task::AutoDelete );
	return true;
}

void Client::changeICQPasswordFinished()
{
	ICQChangePasswordTask* task = (ICQChangePasswordTask*)sender();
	if ( task->success() )
		d->pass = task->password();

	emit icqPasswordChanged( !task->success() );
}

ICQFullInfo Client::getFullInfo( const QString& contact )
{
	return d->icqTlvInfoTask->fullInfoFor( contact );
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

ICQNotesInfo Client::getNotesInfo( const QString& contact )
{
	return d->icqInfoTask->notesInfoFor( contact );
}

ICQMoreUserInfo Client::getMoreInfo( const QString& contact )
{
	return d->icqInfoTask->moreInfoFor( contact );
}

ICQInterestInfo Client::getInterestInfo( const QString& contact )
{
	return d->icqInfoTask->interestInfoFor( contact );
}

ICQOrgAffInfo Client::getOrgAffInfo( const QString& contact )
{
	return d->icqInfoTask->orgAffInfoFor( contact );
}

ICQShortInfo Client::getShortInfo( const QString& contact )
{
	return d->icqInfoTask->shortInfoFor( contact );
}

QList<int> Client::chatExchangeList() const
{
    return d->exchanges;
}

void Client::setChatExchangeList( const QList<int>& exchanges )
{
    d->exchanges = exchanges;
}

void Client::requestAIMProfile( const QString& contact )
{
	d->userInfoTask->requestInfoFor( contact, UserInfoTask::Profile );
}

void Client::requestAIMAwayMessage( const QString& contact )
{
	d->userInfoTask->requestInfoFor( contact, UserInfoTask::AwayMessage );
}

void Client::requestICQAwayMessage( const QString& contact, ICQStatus contactStatus )
{
	kDebug(OSCAR_RAW_DEBUG) << "requesting away message for " << contact;
	Oscar::Message msg;
	msg.setChannel( 2 );
	msg.setReceiver( contact );

	if ( (contactStatus & ICQXStatus) == ICQXStatus )
	{
		Xtraz::XtrazNotify xNotify;
		xNotify.setSenderUni( userId() );

		msg.setMessageType( Oscar::MessageType::Plugin ); // plugin message
		msg.setPlugin( xNotify.statusRequest() );
	}
	else if ( (contactStatus & ICQPluginStatus) == ICQPluginStatus )
	{
		Oscar::WORD subTypeId = 0xFFFF;
		QByteArray subTypeText;

		switch ( contactStatus & ICQStatusMask )
		{
		case ICQOnline:
		case ICQFreeForChat:
		case ICQAway:
			subTypeId = 1;
			subTypeText = "Away Status Message";
			break;
		case ICQOccupied:
		case ICQDoNotDisturb:
			subTypeId = 2;
			subTypeText = "Busy Status Message";
			break;
		case ICQNotAvailable:
			subTypeId = 3;
			subTypeText = "N/A Status Message";
			break;
		default:
			// may be a good way to deal with possible error and lack of online status message?
			emit receivedAwayMessage( contact, "Sorry, this protocol does not support this type of status message" );
			return;
		}

		Oscar::MessagePlugin *plugin = new Oscar::MessagePlugin();
		plugin->setType( Oscar::MessagePlugin::StatusMsgExt );
		plugin->setSubTypeId( subTypeId );
		plugin->setSubTypeText( subTypeText );

		Buffer buffer;
		buffer.addLEDWord( 0x00000000 );
		//TODO: Change this to text/x-aolrtf
		buffer.addLEDBlock( "text/plain" );
		plugin->setData( buffer.buffer() );

		msg.setMessageType( Oscar::MessageType::Plugin ); // plugin message
		msg.setPlugin( plugin );
	}
	else
	{
		msg.addProperty( Oscar::Message::StatusMessageRequest );
		switch ( contactStatus & ICQStatusMask )
		{
		case ICQAway:
			msg.setMessageType( Oscar::MessageType::AutoAway ); // away
			break;
		case ICQOccupied:
			msg.setMessageType( Oscar::MessageType::AutoBusy ); // occupied
			break;
		case ICQNotAvailable:
			msg.setMessageType( Oscar::MessageType::AutoNA ); // not awailable
			break;
		case ICQDoNotDisturb:
			msg.setMessageType( Oscar::MessageType::AutoDND ); // do not disturb
			break;
		case ICQFreeForChat:
			msg.setMessageType( Oscar::MessageType::AutoFFC ); // free for chat
			break;
		default:
			// may be a good way to deal with possible error and lack of online status message?
			emit receivedAwayMessage( contact, "Sorry, this protocol does not support this type of status message" );
			return;
		}
	}
	sendMessage( msg );
}

void Client::addICQAwayMessageRequest( const QString& contact, ICQStatus contactStatus )
{
	kDebug(OSCAR_RAW_DEBUG) << "adding away message request for "
	                         << contact << " to queue" << endl;

	//remove old request if still exists
	removeICQAwayMessageRequest( contact );

	ClientPrivate::AwayMsgRequest amr = { contact, contactStatus };
	d->awayMsgRequestQueue.prepend( amr );

	if ( !d->awayMsgRequestTimer->isActive() )
		d->awayMsgRequestTimer->start( 1000 );
}

void Client::removeICQAwayMessageRequest( const QString& contact )
{
	kDebug(OSCAR_RAW_DEBUG) << "removing away message request for "
	                         << contact << " from queue" << endl;

	QList<ClientPrivate::AwayMsgRequest>::iterator it = d->awayMsgRequestQueue.begin();
	while ( it != d->awayMsgRequestQueue.end() )
	{
		if ( (*it).contact == contact )
			it = d->awayMsgRequestQueue.erase( it );
		else
			it++;
	}
}

void Client::nextICQAwayMessageRequest()
{
	kDebug(OSCAR_RAW_DEBUG) << "request queue count " << d->awayMsgRequestQueue.count();

	if ( d->awayMsgRequestQueue.empty() )
	{
		d->awayMsgRequestTimer->stop();
		return;
	}
	else
	{
		Connection* c = d->connections.connectionForFamily( 0x0004 );
		if ( !c )
			return;

		SNAC s = { 0x0004, 0x0006, 0x0000, 0x00000000 };
		//get time needed to restore level to initial
		//for some reason when we are long under initial level
		//icq server will start to block our messages
		int time = c->rateManager()->timeToInitialLevel( s );
		if ( time > 0 )
		{
			d->awayMsgRequestTimer->start( time );
			return;
		}
		else
		{
			d->awayMsgRequestTimer->start( 5000 );
		}
	}

	ClientPrivate::AwayMsgRequest amr;

	amr = d->awayMsgRequestQueue.back();
	d->awayMsgRequestQueue.pop_back();
	requestICQAwayMessage( amr.contact, amr.contactStatus );
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
	connect( ust, SIGNAL(foundUser(ICQSearchResult)),
	         this, SIGNAL(gotSearchResults(ICQSearchResult)) );
	connect( ust, SIGNAL(searchFinished(int)), this, SIGNAL(endOfSearch(int)) );
	ust->go( Task::AutoDelete ); //onGo does nothing in this task. This is just here so autodelete works
	ust->searchWhitePages( info );
}

void Client::uinSearch( const QString& uin )
{
	Connection* c = d->connections.connectionForFamily( 0x0015 );
	if ( !c )
		return;
	UserSearchTask* ust = new UserSearchTask( c->rootTask() );
	connect( ust, SIGNAL(foundUser(ICQSearchResult)),
	         this, SIGNAL(gotSearchResults(ICQSearchResult)) );
	connect( ust, SIGNAL(searchFinished(int)), this, SIGNAL(endOfSearch(int)) );
	ust->go( Task::AutoDelete ); //onGo does nothing in this task. This is just here so autodelete works
	ust->searchUserByUIN( uin );
}

void Client::updateProfile( const QString& profile )
{
	Connection* c = d->connections.connectionForFamily( 0x0002 );
	if ( !c )
		return;
	ProfileTask* pt = new ProfileTask( c->rootTask() );
	pt->setProfileText( profile );
	pt->go( Task::AutoDelete );
}

bool Client::updateProfile( const QList<ICQInfoBase*>& infoList )
{
	Connection* c = d->connections.connectionForFamily( 0x0015 );
	if ( !c )
		return false;

	ICQUserInfoUpdateTask* ui = new ICQUserInfoUpdateTask( c->rootTask() );
	ui->setInfo( infoList );
	ui->go( Task::AutoDelete );
	return true;
}

void Client::sendTyping( const QString & contact, bool typing )
{
	Connection* c = d->connections.connectionForFamily( 0x0004 );
	if ( !c || !d->active )
		return;
	d->typingNotifyTask->setParams( contact, ( typing ? TypingNotifyTask::Begin : TypingNotifyTask::Finished ) );
	d->typingNotifyTask->go(); 	// don't delete the task after sending
}

void Client::connectToIconServer()
{
	Connection* c = d->connections.connectionForFamily( 0x0010 );
	if ( c )
		return;

	requestServerRedirect( 0x0010 );
}

void Client::setIgnore( const QString& user, bool ignore )
{
	OContact item = ssiManager()->findItem( user,  ROSTER_IGNORE );
	if ( item && !ignore )
	{
		kDebug(OSCAR_RAW_DEBUG) << "Removing " << user << " from ignore list";
		this->modifyContactItem( item, OContact() );
	}
	else if ( !item && ignore )
	{
		kDebug(OSCAR_RAW_DEBUG) << "Adding " << user << " to ignore list";
		OContact s( user, 0, ssiManager()->nextContactId(), ROSTER_IGNORE, QList<TLV>() );
		this->modifyContactItem( OContact(), s );
	}
}

void Client::setVisibleTo( const QString& user, bool visible )
{
	OContact item = ssiManager()->findItem( user,  ROSTER_VISIBLE );
	if ( item && !visible )
	{
		kDebug(OSCAR_RAW_DEBUG) << "Removing " << user << " from visible list";
		this->modifyContactItem( item, OContact() );
	}
	else if ( !item && visible )
	{
		kDebug(OSCAR_RAW_DEBUG) << "Adding " << user << " to visible list";
		OContact s( user, 0, ssiManager()->nextContactId(), ROSTER_VISIBLE, QList<TLV>() );
		this->modifyContactItem( OContact(), s );
	}
}

void Client::setInvisibleTo( const QString& user, bool invisible )
{
	OContact item = ssiManager()->findItem( user,  ROSTER_INVISIBLE );
	if ( item && !invisible )
	{
		kDebug(OSCAR_RAW_DEBUG) << "Removing " << user << " from invisible list";
		this->modifyContactItem( item, OContact() );
	}
	else if ( !item && invisible )
	{
		kDebug(OSCAR_RAW_DEBUG) << "Adding " << user << " to invisible list";
		OContact s( user, 0, ssiManager()->nextContactId(), ROSTER_INVISIBLE, QList<TLV>() );
		this->modifyContactItem( OContact(), s );
	}
}

void Client::requestBuddyIcon( const QString& user, const QByteArray& hash, Oscar::WORD iconType, Oscar::BYTE hashType )
{
	Connection* c = d->connections.connectionForFamily( 0x0010 );
	if ( !c )
		return;

	BuddyIconTask* bit = new BuddyIconTask( c->rootTask() );
	connect( bit, SIGNAL(haveIcon(QString,QByteArray)),
	         this, SIGNAL(haveIconForContact(QString,QByteArray)) );
	bit->requestIconFor( user );
	bit->setIconType( iconType );
	bit->setHashType( hashType );
	bit->setHash( hash );
	bit->go( Task::AutoDelete );
}

void Client::requestServerRedirect( Oscar::WORD family, Oscar::WORD exchange,
                                    QByteArray cookie, Oscar::WORD instance,
                                    const QString& room )
{
	//making the assumption that family 2 will always be the BOS connection
	//use it instead since we can't query for family 1
	Connection* c = d->connections.connectionForFamily( family );
	if ( c && family != 0x000E )
		return; //we already have the connection

	c = d->connections.connectionForFamily( 0x0002 );
	if ( !c )
		return;

    if ( d->redirectionServices.indexOf( family ) == -1 )
        d->redirectionServices.append( family ); //don't add families twice

    if ( d->currentRedirect != 0 )
        return; //we're already doing one redirection

	d->currentRedirect = family;

    //FIXME. this won't work if we have to defer the connection because we're
    //already connecting to something
	ServerRedirectTask* srt = new ServerRedirectTask( c->rootTask() );
    if ( family == 0x000E )
    {
        srt->setChatParams( exchange, cookie, instance );
        srt->setChatRoom( room );
    }

	connect( srt, SIGNAL(haveServer(QString,QByteArray,Oscar::WORD)),
	         this, SLOT(haveServerForRedirect(QString,QByteArray,Oscar::WORD)) );
	srt->setService( family );
	srt->go( Task::AutoDelete );
}

void Client::haveServerForRedirect( const QString& host, const QByteArray& cookie, Oscar::WORD )
{
    //nasty sender() usage to get the task with chat room info
	QObject* o = const_cast<QObject*>( sender() );
    ServerRedirectTask* srt = dynamic_cast<ServerRedirectTask*>( o );

	//create a new connection and set it up
	int colonPos = host.indexOf(':');
	QString realHost;
	uint realPort;
	if ( colonPos != -1 )
	{
		realHost = host.left( colonPos );
		realPort = host.right(4).toUInt(); //we only need 4 bytes
	}
	else
	{
		realHost = host;
		realPort = d->port;
	}

	bool encrypted = d->encrypted2;

	Connection* c = createConnection();
	//create the new login task
	d->loginTaskTwo = new StageTwoLoginTask( c->rootTask() );
	d->loginTaskTwo->setCookie( cookie );
	QObject::connect( d->loginTaskTwo, SIGNAL(finished()), this, SLOT(serverRedirectFinished()) );

	//connect
	connectToServer( c, realHost, realPort, encrypted, QString() );
  	QObject::connect( c, SIGNAL(connected()), this, SLOT(streamConnected()) );

    if ( srt )
        d->connections.addChatInfoForConnection( c, srt->chatExchange(), srt->chatRoomName() );
}

void Client::serverRedirectFinished()
{
	StageTwoLoginTask* loginTaskTwo = qobject_cast<StageTwoLoginTask*>( sender() );

	if ( loginTaskTwo && loginTaskTwo->statusCode() == 0 )
	{ //stage two was successful
		Connection* c = d->connections.connectionForFamily( d->currentRedirect );
		if ( !c )
			return;
		ClientReadyTask* crt = new ClientReadyTask( c->rootTask() );
		crt->setFamilies( c->supportedFamilies() );
		crt->go( Task::AutoDelete );
	}

	kDebug(OSCAR_RAW_DEBUG) << "redirection finished for service "
	                         << d->currentRedirect << endl;

	if ( d->currentRedirect == 0x0010 )
		emit iconServerConnected();

	if ( d->currentRedirect == 0x000D )
	{
		connect( this, SIGNAL(chatNavigationConnected()),
				 this, SLOT(requestChatNavLimits()) );
		emit chatNavigationConnected();
	}

	if ( d->currentRedirect == 0x000E )
	{
		//HACK! such abuse! think of a better way
		if ( !loginTaskTwo )
		{
			kWarning(OSCAR_RAW_DEBUG) << "no login task to get connection from!";
			emit redirectionFinished( d->currentRedirect );
			return;
		}
	
		Connection* c = loginTaskTwo->client();
		QString roomName = d->connections.chatRoomForConnection( c );
		Oscar::WORD exchange = d->connections.exchangeForConnection( c );
		if ( c )
		{
			kDebug(OSCAR_RAW_DEBUG) << "setting up chat connection";
			ChatServiceTask* cst = new ChatServiceTask( c->rootTask(), exchange, roomName );
			connect( cst, SIGNAL(userJoinedChat(Oscar::WORD,QString,QString)),
			         this, SIGNAL(userJoinedChat(Oscar::WORD,QString,QString)) );
			connect( cst, SIGNAL(userLeftChat(Oscar::WORD,QString,QString)),
			         this, SIGNAL(userLeftChat(Oscar::WORD,QString,QString)) );
			connect( cst, SIGNAL(newChatMessage(Oscar::Message)),
			         this, SIGNAL(messageReceived(Oscar::Message)) );
		}
		emit chatRoomConnected( exchange, roomName );
	}
	
	emit redirectionFinished( d->currentRedirect );

}

void Client::checkRedirectionQueue( Oscar::WORD family )
{
	kDebug(OSCAR_RAW_DEBUG) << "checking redirection queue";
	d->redirectionServices.removeAll( family );
    d->currentRedirect = 0;
	if ( !d->redirectionServices.isEmpty() )
	{
		kDebug(OSCAR_RAW_DEBUG) << "scheduling new redirection";
		requestServerRedirect( d->redirectionServices.front() );
	}
}


void Client::requestChatNavLimits()
{
	Connection* c = d->connections.connectionForFamily( 0x000D );
	if ( !c )
		return;

	kDebug(OSCAR_RAW_DEBUG) << "requesting chat nav service limits";
	ChatNavServiceTask* cnst = new ChatNavServiceTask( c->rootTask() );
    cnst->setRequestType( ChatNavServiceTask::Limits );
    QObject::connect( cnst, SIGNAL(haveChatExchanges(QList<int>)),
                      this, SLOT(setChatExchangeList(QList<int>)) );
	cnst->go( Task::AutoDelete ); //autodelete

}

void Client::determineDisconnection( int code, const QString& string )
{
    if ( !sender() )
        return;

    //yay for the sender() hack!
    QObject* obj = const_cast<QObject*>( sender() );
    Connection* c = dynamic_cast<Connection*>( obj );
    if ( !c )
        return;

	if ( c->isSupported( 0x0002 ) ||
	     d->stage == ClientPrivate::StageOne ) //emit on login
	{
		emit socketError( code, string );
	}

	foreach ( Oscar::MessageInfo info, c->messageInfoList() )
		emit messageError( info.contact, info.id );

    //connection is deleted. deleteLater() is used
    d->connections.remove( c );
    c = 0;
}

void Client::sendBuddyIcon( const QByteArray& iconData )
{
	Connection* c = d->connections.connectionForFamily( 0x0010 );
	if ( !c )
		return;

	kDebug(OSCAR_RAW_DEBUG) << "icon length is " << iconData.size();
	BuddyIconTask* bit = new BuddyIconTask( c->rootTask() );
	bit->uploadIcon( iconData.size(), iconData );
	bit->go( Task::AutoDelete );
}

void Client::joinChatRoom( const QString& roomName, int exchange )
{
    Connection* c = d->connections.connectionForFamily( 0x000D );
    if ( !c )
        return;

    kDebug(OSCAR_RAW_DEBUG) << "joining the chat room '" << roomName
                             << "' on exchange " << exchange << endl;
    ChatNavServiceTask* cnst = new ChatNavServiceTask( c->rootTask() );
    connect( cnst, SIGNAL(connectChat(Oscar::WORD,QByteArray,Oscar::WORD,QString)),
             this, SLOT(setupChatConnection(Oscar::WORD,QByteArray,Oscar::WORD,QString)) );
    cnst->createRoom( exchange, roomName );

}

void Client::setupChatConnection( Oscar::WORD exchange, QByteArray cookie, Oscar::WORD instance, const QString& room )
{
    kDebug(OSCAR_RAW_DEBUG) << "cookie is:" << cookie;
    QByteArray realCookie( cookie );
    kDebug(OSCAR_RAW_DEBUG) << "connection to chat room";
    requestServerRedirect( 0x000E, exchange, realCookie, instance, room );
}

void Client::disconnectChatRoom( Oscar::WORD exchange, const QString& room )
{
    Connection* c = d->connections.connectionForChatRoom( exchange, room );
    if ( !c )
        return;

    d->connections.remove( c );
    c = 0;
}

void Client::connectToServer( Connection* c, const QString& host, quint16 port, bool encrypted, const QString &name )
{
	d->connections.append( c );
	connect( c, SIGNAL(socketError(int,QString)), this, SLOT(determineDisconnection(int,QString)) );
	c->connectToServer( host, port, encrypted, name );
}

ClientStream* Client::createClientStream()
{
	ClientStream* cs = 0;
	emit createClientStream( &cs );
	if ( !cs )
		cs = new ClientStream( new QSslSocket(), 0 );

	return cs;
}

Connection* Client::createConnection()
{
	ClientStream* cs = createClientStream();
	cs->setNoopTime( 60000 );
	Connection* c = new Connection( cs, "BOS" );
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
	delete d->messageAckTask;
	delete d->ssiAuthTask;
	delete d->icqInfoTask;
	delete d->icqTlvInfoTask;
	delete d->userInfoTask;
	delete d->typingNotifyTask;
	delete d->ssiModifyTask;

	d->errorTask = 0;
	d->onlineNotifier = 0;
	d->ownStatusTask = 0;
	d->messageReceiverTask = 0;
	d->messageAckTask = 0;
	d->ssiAuthTask = 0;
	d->icqInfoTask = 0;
	d->icqTlvInfoTask = 0;
	d->userInfoTask = 0;
	d->typingNotifyTask = 0;
	d->ssiModifyTask = 0;
}

bool Client::hasIconConnection( ) const
{
	Connection* c = d->connections.connectionForFamily( 0x0010 );
	return c;
}

FileTransferHandler* Client::createFileTransfer( const QString& contact, const QStringList& files )
{
	Connection* c = d->connections.connectionForFamily( 0x0004 );
	if ( !c )
		return 0;

	FileTransferTask *ft = new FileTransferTask( c->rootTask(), contact, ourInfo().userId(), files );
	connect( ft, SIGNAL(sendMessage(Oscar::Message)),
	         this, SLOT(fileMessage(Oscar::Message)) );

	return new FileTransferHandler(ft);
}

void Client::inviteToChatRoom( const QString& contact, Oscar::WORD exchange, const QString& room, QString msg)
{
	Connection* c = d->connections.connectionForFamily( 0x0004 );
	ChatRoomTask *chatRoomTask = new ChatRoomTask( c->rootTask(), contact, ourInfo().userId(), msg, exchange, room);
	chatRoomTask->go( Task::AutoDelete );
	chatRoomTask->doInvite();
}

void Client::gotChatRoomMessage( const Oscar::Message & msg, const QByteArray & cookie )
{
	Connection* c = d->connections.connectionForFamily( 0x0004 );
	if ( msg.requestType() == 0 )
	{
		ChatRoomTask *task = new ChatRoomTask( c->rootTask(), msg.sender(), msg.receiver(), cookie, msg.text( QTextCodec::codecForName( "UTF-8" ) ), msg.exchange(), msg.chatRoom() );
		task->go( Task::AutoDelete );
		emit chatroomRequest( new ChatRoomHandler( task ) );
	}
}

void Client::gotFileMessage( int type, const QString from, const QByteArray cookie, Buffer buf)
{
	Connection* c = d->connections.connectionForFamily( 0x0004 );
	if ( !c )
		return;
	//pass the message to the matching task if we can
	const QList<FileTransferTask*> p = c->rootTask()->findChildren<FileTransferTask*>();
	foreach( FileTransferTask *t, p)
	{
		if ( t->take( type, cookie, buf ) )
		{
			return;
		}
	}
	//maybe it's a new request!
	if ( type == 0 )
	{
		kDebug(14151) << "new request :)";
		FileTransferTask *ft = new FileTransferTask( c->rootTask(), from, ourInfo().userId(), cookie, buf );
		connect( ft, SIGNAL(sendMessage(Oscar::Message)),
				this, SLOT(fileMessage(Oscar::Message)) );
		ft->go( Task::AutoDelete );
		
		emit incomingFileTransfer( new FileTransferHandler(ft) );
	}

	kDebug(14151) << "nobody wants it :(";
}

}

#include "client.moc"
//kate: tab-width 4; indent-mode csands; space-indent off; replace-tabs off;
