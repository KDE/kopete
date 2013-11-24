/*
    yahooaccount.cpp - Manages a single Yahoo account

    Copyright (c) 2003 by Gav Wood               <gav@kde.org>
    Copyright (c) 2003-2004 by Matt Rogers       <matt.rogers@kdemail.net>
    Based on code by Olivier Goffart             <ogoffart@kde.org>

    Kopete    (c) 2002-2004 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/
// Own Header
#include "yahooaccount.h"

//Standard Header
#include <ctime>
#include <stdlib.h>

//QT
#include <QFont>
#include <QDateTime>
#include <QColor>
#include <QRegExp>
#include <QImage>
#include <QFile>
#include <QPixmap>
#include <QDir>
#include <QFileInfo>
#include <QPointer>

// KDE
#include <klocale.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kaction.h>
#include <kactionmenu.h>
#include <kmenu.h>
#include <kmessagebox.h>
#include <krun.h>
#include <kstandarddirs.h>
#include <ktoolinvocation.h>
#include <kicon.h>

// Kopete
#include <kopetechatsession.h>
#include <kopetemessage.h>
#include <kopetepassword.h>
#include <kopeteuiglobal.h>
#include <knotification.h>
#include <kopetemetacontact.h>
#include <kopetecontactlist.h>
#include <kopetetransfermanager.h>
#include <kopeteview.h>
#include <kopeteaddedinfoevent.h>

// Yahoo
#include "yahoocontact.h"
#include "yahooconnector.h"
#include "yahooclientstream.h"
#include "client.h"
#include "yahooverifyaccount.h"
#include "yahoowebcam.h"
#include "yahooconferencemessagemanager.h"
#include "yahooinvitelistimpl.h"
#include "yabentry.h"
#include "yahoouserinfodialog.h"
#include "yahoochatselectordialog.h"
#include "yahoochatchatsession.h"

using namespace KYahoo;

YahooAccount::YahooAccount(YahooProtocol *parent, const QString& accountId)
 : Kopete::PasswordedAccount(parent, accountId, false)
{

	// first things first - initialise internals
	stateOnConnection = 0;
	theHaveContactList = false;
	m_protocol = parent;
	m_session = new Client( this );
	m_lastDisconnectCode = 0;
	m_currentMailCount = 0;
	m_webcam = 0;
	m_chatChatSession = 0;

	m_openInboxAction = new KAction( KIcon("mail-folder-inbox"), i18n( "Open Inbo&x..." ), this );
        //, "m_openInboxAction" );
	QObject::connect(m_openInboxAction, SIGNAL(triggered(bool)), this, SLOT(slotOpenInbox()) );
	m_openYABAction = new KAction( KIcon("x-office-address-book"), i18n( "Open &Address book..." ), this );
        //, "m_openYABAction" );
	QObject::connect(m_openYABAction, SIGNAL(triggered(bool)), this, SLOT(slotOpenYAB()) );
	m_editOwnYABEntry = new KAction( KIcon("document-properties"), i18n( "&Edit my contact details..."), this );
        //, "m_editOwnYABEntry" );
	QObject::connect(m_editOwnYABEntry, SIGNAL(triggered(bool)), this, SLOT(slotEditOwnYABEntry()) );
	m_joinChatAction = new KAction( KIcon("im-chat-room-join"), i18n( "&Join chat room..."), this );
        //, "m_joinChatAction" );
	QObject::connect(m_joinChatAction, SIGNAL(triggered(bool)), this, SLOT(slotJoinChatRoom()) );

	YahooContact* _myself=new YahooContact( this, accountId.toLower(), accountId, Kopete::ContactList::self()->myself() );
	setMyself( _myself );
	_myself->setOnlineStatus( parent->Offline );
	myself()->setProperty( YahooProtocol::protocol()->iconRemoteUrl, configGroup()->readEntry( "iconRemoteUrl", "" ) );
	myself()->setProperty( Kopete::Global::Properties::self()->photo(), configGroup()->readEntry( "iconLocalUrl", "" ) );
	myself()->setProperty( YahooProtocol::protocol()->iconCheckSum, configGroup()->readEntry( "iconCheckSum", 0 ) );
	myself()->setProperty( YahooProtocol::protocol()->iconExpire, configGroup()->readEntry( "iconExpire", 0 ) );

// 	initConnectionSignals( MakeConnections );

	QString displayName = configGroup()->readEntry(QLatin1String("displayName"), QString());
	if(!displayName.isEmpty())
		_myself->setNickName(displayName);

	m_YABLastMerge = configGroup()->readEntry( "YABLastMerge", 0 );
	m_YABLastRemoteRevision = configGroup()->readEntry( "YABLastRemoteRevision", 0 );

	m_session->setUserId( accountId.toLower() );
	m_session->setPictureChecksum( myself()->property( YahooProtocol::protocol()->iconCheckSum ).value().toInt() );

	setupActions( false );
}

YahooAccount::~YahooAccount()
{
	if( m_webcam )
		m_webcam->stopTransmission();
}

void YahooAccount::setServer( const QString &server )
{
	configGroup()->writeEntry( QLatin1String( "Server" ), server );
}

void YahooAccount::setPort( int port )
{
	configGroup()->writeEntry( QLatin1String( "Port" ), port );
}

void YahooAccount::slotGoStatus( int status, const QString &awayMessage)
{
	kDebug(YAHOO_GEN_DEBUG) << "GoStatus: " << status << " msg: " << awayMessage;
	if( !isConnected() )
	{
		connect( m_protocol->statusFromYahoo( status ) );
		stateOnConnection = status;
	}
	else
	{
		m_session->changeStatus( Yahoo::Status( status ), awayMessage,
			(status == Yahoo::StatusAvailable)? Yahoo::StatusTypeAvailable : Yahoo::StatusTypeAway );

		//sets the awayMessage property for the owner of the account. shows up in the statusbar icon's tooltip. the property is unset when awayMessage is null
		myself()->setStatusMessage( Kopete::StatusMessage(awayMessage) );

		myself()->setOnlineStatus( m_protocol->statusFromYahoo( status ) );
	}
}

Client *YahooAccount::yahooSession()
{
	return m_session ? m_session : 0L;
}

QString YahooAccount::stripMsgColorCodes(const QString& msg)
{
	QString filteredMsg = msg;

	//Handle bold, underline and italic messages
	filteredMsg.replace( "\033[1m", "<b>" );
	filteredMsg.replace( "\033[x1m", "</b>" );
	filteredMsg.replace( "\033[2m", "<i>" );
	filteredMsg.replace( "\033[x2m", "</i>" );
	filteredMsg.replace( "\033[4m", "<u>" );
	filteredMsg.replace( "\033[x4m", "</u>" );

	//GAIM doesn't check for ^[[3m. Does this ever get sent?
	filteredMsg.replace( "\033[3m", "<i>" );
	filteredMsg.replace( "\033[x3m", "</i>" );

	//Strip link tags
	filteredMsg.remove( "\033[lm" );
	filteredMsg.remove( "\033[xlm" );

	//Remove color codes and other residual formatting
	filteredMsg.remove( QRegExp("\033\\[[^m]*m") );

	return filteredMsg;
}

QColor YahooAccount::getMsgColor(const QString& msg)
{
	/* Yahoo sends a message either with color or without color
	 * so we have to use this really hacky method to get colors
	 */
	//kDebug(YAHOO_GEN_DEBUG) << "msg is " << msg;
	//Please note that some of the colors are hard-coded to
	//match the yahoo colors
	if ( msg.indexOf("\033[38m") != -1 )
		return Qt::red;
	if ( msg.indexOf("\033[34m") != -1 )
		return Qt::green;
	if ( msg.indexOf("\033[31m") != -1 )
		return Qt::blue;
	if ( msg.indexOf("\033[39m") != -1 )
		return Qt::yellow;
	if ( msg.indexOf("\033[36m") != -1 )
		return Qt::darkMagenta;
	if ( msg.indexOf("\033[32m") != -1 )
		return Qt::cyan;
	if ( msg.indexOf("\033[37m") != -1 )
		return QColor("#FFAA39");
	if ( msg.indexOf("\033[35m") != -1 )
		return QColor("#FFD8D8");
	if ( msg.indexOf("\033[#") != -1 )
	{
		kDebug(YAHOO_GEN_DEBUG) << "Custom color is " << msg.mid(msg.indexOf("\033[#")+2,7);
		return QColor(msg.mid(msg.indexOf("\033[#")+2,7));
	}

	//return a default value just in case
	return Qt::black;
}

void YahooAccount::initConnectionSignals( enum SignalConnectionType sct )
{
	if ( !m_session )
		return;

	if ( sct == MakeConnections )
	{
		QObject::connect(m_session, SIGNAL(loggedIn(int,QString)),
		                 this, SLOT(slotLoginResponse(int,QString)) );

		QObject::connect(m_session, SIGNAL(disconnected()),
		                 this, SLOT(slotDisconnected()) );

		QObject::connect(m_session, SIGNAL(loginFailed()),
		                 this, SLOT(slotLoginFailed()) );

		QObject::connect(m_session, SIGNAL(error(int)),
		                 this, SLOT(slotError(int)));

		QObject::connect(m_session, SIGNAL(gotBuddy(QString,QString,QString)),
		                 this, SLOT(slotGotBuddy(QString,QString,QString)));

		QObject::connect(m_session, SIGNAL(buddyAddResult(QString,QString,bool)),
				 this, SLOT(slotBuddyAddResult(QString,QString,bool)));

		QObject::connect(m_session, SIGNAL(buddyRemoveResult(QString,QString,bool)),
				 this, SLOT(slotBuddyRemoveResult(QString,QString,bool)));

		QObject::connect(m_session, SIGNAL(buddyChangeGroupResult(QString,QString,bool)),
				 this, SLOT(slotBuddyChangeGroupResult(QString,QString,bool)));

		QObject::connect(m_session, SIGNAL(authorizationAccepted(QString)),
		                 this, SLOT(slotAuthorizationAccepted(QString)) );

		QObject::connect(m_session, SIGNAL(authorizationRejected(QString,QString)),
		                 this, SLOT(slotAuthorizationRejected(QString,QString)) );

		QObject::connect(m_session, SIGNAL(gotAuthorizationRequest(QString,QString,QString)),
		                 this, SLOT(slotgotAuthorizationRequest(QString,QString,QString)) );

		QObject::connect(m_session, SIGNAL(statusChanged(QString,int,QString,int,int,int)),
		                 this, SLOT(slotStatusChanged(QString,int,QString,int,int,int)));

		QObject::connect(m_session, SIGNAL(stealthStatusChanged(QString,Yahoo::StealthStatus)),
		                 this, SLOT(slotStealthStatusChanged(QString,Yahoo::StealthStatus)) );

		QObject::connect(m_session, SIGNAL(gotIm(QString,QString,long,int)),
		                 this, SLOT(slotGotIm(QString,QString,long,int)));

		QObject::connect(m_session, SIGNAL(gotBuzz(QString,long)),
		                 this, SLOT(slotGotBuzz(QString,long)));

		QObject::connect(m_session, SIGNAL( gotConferenceInvite( const QString&, const QString&,
		                                                   const QString&, const QStringList&) ),
		                 this,
		                 SLOT( slotGotConfInvite( const QString&, const QString&,
		                                          const QString&, const QStringList& ) ) );

		QObject::connect(m_session, SIGNAL(confUserDeclined(QString,QString,QString)),
		                 this,
		                 SLOT(slotConfUserDecline(QString,QString,QString)) );

		QObject::connect(m_session , SIGNAL(confUserJoined(QString,QString)), this,
		                 SLOT(slotConfUserJoin(QString,QString)) );

		QObject::connect(m_session , SIGNAL(confUserLeft(QString,QString)), this,
		                 SLOT(slotConfUserLeave(QString,QString)) );

		QObject::connect(m_session , SIGNAL(gotConferenceMessage(QString,QString,QString)), this,
		                 SLOT(slotConfMessage(QString,QString,QString)) );

		QObject::connect(m_session,
		                 SIGNAL(incomingFileTransfer(QString,QString,long,QString,QString,ulong,QPixmap)),
		                 this,
		                 SLOT(slotGotFile(QString,QString,long,QString,QString,ulong,QPixmap)));

		QObject::connect(m_session, SIGNAL(fileTransferComplete(uint)), this,
		                 SLOT(slotFileTransferComplete(uint)) );

		QObject::connect(m_session, SIGNAL(fileTransferBytesProcessed(uint,uint)), this,
		                 SLOT(slotFileTransferBytesProcessed(uint,uint)) );

		QObject::connect(m_session, SIGNAL(fileTransferError(uint,int,QString)), this,
		                 SLOT(slotFileTransferError(uint,int,QString)) );

		QObject::connect(m_session, SIGNAL(typingNotify(QString,int)), this ,
		                 SLOT(slotTypingNotify(QString,int)));

// 		QObject::connect(m_session, SIGNAL(gameNotify(QString,int)), this,
// 		                 SLOT(slotGameNotify(QString,int)));

		QObject::connect(m_session, SIGNAL(mailNotify(QString,QString,int)), this,
		                 SLOT(slotMailNotify(QString,QString,int)));

		QObject::connect(m_session, SIGNAL(systemMessage(QString)), this,
		                 SLOT(slotSystemMessage(QString)));

// 		QObject::connect(m_session, SIGNAL(gotIdentities(QStringList)), this,
// 		                 SLOT(slotGotIdentities(QStringList)));

		QObject::connect(m_session, SIGNAL(gotWebcamInvite(QString)), this, SLOT(slotGotWebcamInvite(QString)));

		QObject::connect(m_session, SIGNAL(webcamNotAvailable(QString)), this, SLOT(slotWebcamNotAvailable(QString)));

		QObject::connect(m_session, SIGNAL(webcamImageReceived(QString,QPixmap)), this, SLOT(slotGotWebcamImage(QString,QPixmap)));

		QObject::connect(m_session, SIGNAL(webcamClosed(QString,int)), this, SLOT(slotWebcamClosed(QString,int)));

		QObject::connect(m_session, SIGNAL(webcamPaused(QString)), this, SLOT(slotWebcamPaused(QString)));

		QObject::connect(m_session, SIGNAL(webcamReadyForTransmission()), this, SLOT(slotWebcamReadyForTransmission()));

		QObject::connect(m_session, SIGNAL(webcamStopTransmission()), this, SLOT(slotWebcamStopTransmission()));

		QObject::connect(m_session, SIGNAL(webcamViewerJoined(QString)), this, SLOT(slotWebcamViewerJoined(QString)));

		QObject::connect(m_session, SIGNAL(webcamViewerLeft(QString)), this, SLOT(slotWebcamViewerLeft(QString)));

		QObject::connect(m_session, SIGNAL(webcamViewerRequest(QString)), this, SLOT(slotWebcamViewerRequest(QString)));

		QObject::connect(m_session, SIGNAL(pictureStatusNotify(QString,int)), SLOT(slotPictureStatusNotify(QString,int)));

		QObject::connect(m_session, SIGNAL(pictureDownloaded(QString,QByteArray,int)), this, SLOT(slotGotBuddyIcon(QString,QByteArray,int)) );

		QObject::connect(m_session, SIGNAL(pictureInfoNotify(QString,KUrl,int)), this, SLOT(slotGotBuddyIconInfo(QString,KUrl,int)));

		QObject::connect(m_session, SIGNAL(pictureChecksumNotify(QString,int)), this, SLOT(slotGotBuddyIconChecksum(QString,int)));

		QObject::connect(m_session, SIGNAL(pictureRequest(QString)), this, SLOT(slotGotBuddyIconRequest(QString)) );

		QObject::connect(m_session, SIGNAL(pictureUploaded(QString,int)), this, SLOT(slotBuddyIconChanged(QString,int)));

		QObject::connect(m_session, SIGNAL(gotYABEntry(YABEntry*)), this, SLOT(slotGotYABEntry(YABEntry*)));

		QObject::connect(m_session, SIGNAL(modifyYABEntryError(YABEntry*,QString)), this, SLOT(slotModifyYABEntryError(YABEntry*,QString)));

		QObject::connect(m_session, SIGNAL(gotYABRevision(long,bool)), this, SLOT(slotGotYABRevision(long,bool)) );

		QObject::connect(m_session, SIGNAL(chatRoomJoined(int,int,QString,QString)), this, SLOT(slotChatJoined(int,int,QString,QString)));

		QObject::connect(m_session, SIGNAL(chatBuddyHasJoined(QString,QString,bool)), this, SLOT(slotChatBuddyHasJoined(QString,QString,bool)));

		QObject::connect(m_session, SIGNAL(chatBuddyHasLeft(QString,QString)), this, SLOT(slotChatBuddyHasLeft(QString,QString)));

		QObject::connect(m_session, SIGNAL(chatMessageReceived(QString,QString,QString)), this, SLOT(slotChatMessageReceived(QString,QString,QString)));
	}

	if ( sct == DeleteConnections )
	{
		QObject::disconnect(m_session, SIGNAL(loggedIn(int,QString)),
		                    this, SLOT(slotLoginResponse(int,QString)) );

		QObject::disconnect(m_session, SIGNAL(disconnected()),
		                    this, SLOT(slotDisconnected()) );

		QObject::disconnect(m_session, SIGNAL(loginFailed()),
		                    this, SLOT(slotLoginFailed()) );

		QObject::disconnect(m_session, SIGNAL(error(int)),
		                 this, SLOT(slotError(int)));

		QObject::disconnect(m_session, SIGNAL(gotBuddy(QString,QString,QString)),
		                    this, SLOT(slotGotBuddy(QString,QString,QString)));

		QObject::disconnect(m_session, SIGNAL(buddyAddResult(QString,QString,bool)),
		                    this, SLOT(slotBuddyAddResult(QString,QString,bool)));

		QObject::disconnect(m_session, SIGNAL(buddyRemoveResult(QString,QString,bool)),
		                    this, SLOT(slotBuddyRemoveResult(QString,QString,bool)));

		QObject::disconnect(m_session, SIGNAL(buddyChangeGroupResult(QString,QString,bool)),
				 this, SLOT(slotBuddyChangeGroupResult(QString,QString,bool)));

		QObject::disconnect(m_session, SIGNAL(authorizationAccepted(QString)),
		                 this, SLOT(slotAuthorizationAccepted(QString)) );

		QObject::disconnect(m_session, SIGNAL(authorizationRejected(QString,QString)),
		                    this, SLOT(slotAuthorizationRejected(QString,QString)) );

		QObject::disconnect(m_session, SIGNAL(gotAuthorizationRequest(QString,QString,QString)),
		                 this, SLOT(slotgotAuthorizationRequest(QString,QString,QString)) );

		QObject::disconnect(m_session, SIGNAL(statusChanged(QString,int,QString,int,int,int)),
		                    this, SLOT(slotStatusChanged(QString,int,QString,int,int,int)));

		QObject::disconnect(m_session, SIGNAL(stealthStatusChanged(QString,Yahoo::StealthStatus)),
		                 this, SLOT(slotStealthStatusChanged(QString,Yahoo::StealthStatus)) );

		QObject::disconnect(m_session, SIGNAL(gotIm(QString,QString,long,int)),
		                    this, SLOT(slotGotIm(QString,QString,long,int)));

		QObject::disconnect(m_session, SIGNAL(gotBuzz(QString,long)),
		                    this, SLOT(slotGotBuzz(QString,long)));

		QObject::disconnect(m_session,
		                    SIGNAL( gotConferenceInvite( const QString&, const QString&,
		                                           const QString&, const QStringList&) ),
		                    this,
		                    SLOT( slotGotConfInvite( const QString&, const QString&,
		                                             const QString&, const QStringList&) ) );

		QObject::disconnect(m_session,
		                    SIGNAL(confUserDeclined(QString,QString,QString)),
		                    this,
		                    SLOT(slotConfUserDecline(QString,QString,QString)) );

		QObject::disconnect(m_session , SIGNAL(confUserJoined(QString,QString)),
		                    this, SLOT(slotConfUserJoin(QString,QString)) );

		QObject::disconnect(m_session , SIGNAL(confUserLeft(QString,QString)),
		                    this, SLOT(slotConfUserLeave(QString,QString)) );

		QObject::disconnect(m_session , SIGNAL(gotConferenceMessage(QString,QString,QString)), this,
		                    SLOT(slotConfMessage(QString,QString,QString)) );

		QObject::disconnect(m_session,
		                    SIGNAL(incomingFileTransfer(const QString &, const QString &,
			                    long, const QString &, const QString &, unsigned long, const QPixmap &)),
		                    this,
		                    SLOT(slotGotFile(const QString&, const QString&,
		                                     long, const QString&, const QString&, unsigned long, const QPixmap &)));

		QObject::disconnect(m_session, SIGNAL(fileTransferComplete(uint)), this,
		                 SLOT(slotFileTransferComplete(uint)) );

		QObject::disconnect(m_session, SIGNAL(fileTransferBytesProcessed(uint,uint)), this,
		                 SLOT(slotFileTransferBytesProcessed(uint,uint)) );

		QObject::disconnect(m_session, SIGNAL(fileTransferError(uint,int,QString)), this,
		                 SLOT(slotFileTransferError(uint,int,QString)) );

		QObject::disconnect(m_session, SIGNAL(typingNotify(QString,int)), this ,
		                    SLOT(slotTypingNotify(QString,int)));

// 		QObject::disconnect(m_session, SIGNAL(gameNotify(QString,int)), this,
// 		                    SLOT(slotGameNotify(QString,int)));

		QObject::disconnect(m_session, SIGNAL(mailNotify(QString,QString,int)), this,
		                    SLOT(slotMailNotify(QString,QString,int)));

		QObject::disconnect(m_session, SIGNAL(systemMessage(QString)), this,
		                    SLOT(slotSystemMessage(QString)));

// 		QObject::disconnect(m_session, SIGNAL(gotIdentities(QStringList)), this,
// 		                    SLOT(slotGotIdentities(QStringList)));

		QObject::disconnect(m_session, SIGNAL(gotWebcamInvite(QString)), this, SLOT(slotGotWebcamInvite(QString)));

		QObject::disconnect(m_session, SIGNAL(webcamNotAvailable(QString)), this, SLOT(slotWebcamNotAvailable(QString)));

		QObject::disconnect(m_session, SIGNAL(webcamImageReceived(QString,QPixmap)), this, SLOT(slotGotWebcamImage(QString,QPixmap)));

		QObject::disconnect(m_session, SIGNAL(webcamClosed(QString,int)), this, SLOT(slotWebcamClosed(QString,int)));

		QObject::disconnect(m_session, SIGNAL(webcamPaused(QString)), this, SLOT(slotWebcamPaused(QString)));

		QObject::disconnect(m_session, SIGNAL(webcamReadyForTransmission()), this, SLOT(slotWebcamReadyForTransmission()));

		QObject::disconnect(m_session, SIGNAL(webcamStopTransmission()), this, SLOT(slotWebcamStopTransmission()));

		QObject::disconnect(m_session, SIGNAL(webcamViewerJoined(QString)), this, SLOT(slotWebcamViewerJoined(QString)));

		QObject::disconnect(m_session, SIGNAL(webcamViewerLeft(QString)), this, SLOT(slotWebcamViewerLeft(QString)));

		QObject::disconnect(m_session, SIGNAL(webcamViewerRequest(QString)), this, SLOT(slotWebcamViewerRequest(QString)));

		QObject::disconnect(m_session, SIGNAL(pictureDownloaded(QString,QByteArray,int)), this, SLOT(slotGotBuddyIcon(QString,QByteArray,int)));

		QObject::disconnect(m_session, SIGNAL(pictureInfoNotify(QString,KUrl,int)), this, SLOT(slotGotBuddyIconInfo(QString,KUrl,int)));

		QObject::disconnect(m_session, SIGNAL(pictureRequest(QString)), this, SLOT(slotGotBuddyIconRequest(QString)) );

		QObject::disconnect(m_session, SIGNAL(pictureUploaded(QString,int)), this, SLOT(slotBuddyIconChanged(QString,int)));

		QObject::disconnect(m_session, SIGNAL(pictureStatusNotify(QString,int)), this, SLOT(slotPictureStatusNotify(QString,int)));

		QObject::disconnect(m_session, SIGNAL(pictureChecksumNotify(QString,int)), this, SLOT(slotGotBuddyIconChecksum(QString,int)));

		QObject::disconnect(m_session, SIGNAL(gotYABEntry(YABEntry*)), this, SLOT(slotGotYABEntry(YABEntry*)));

		QObject::disconnect(m_session, SIGNAL(modifyYABEntryError(YABEntry*,QString)), this, SLOT(slotModifyYABEntryError(YABEntry*,QString)));

		QObject::disconnect(m_session, SIGNAL(gotYABRevision(long,bool)), this, SLOT(slotGotYABRevision(long,bool)) );

		QObject::disconnect(m_session, SIGNAL(chatRoomJoined(int,int,QString,QString)), this, SLOT(slotChatJoined(int,int,QString,QString)));

		QObject::disconnect(m_session, SIGNAL(chatBuddyHasJoined(QString,QString,bool)), this, SLOT(slotChatBuddyHasJoined(QString,QString,bool)));

		QObject::disconnect(m_session, SIGNAL(chatBuddyHasLeft(QString,QString)), this, SLOT(slotChatBuddyHasLeft(QString,QString)));

		QObject::disconnect(m_session, SIGNAL(chatMessageReceived(QString,QString,QString)), this, SLOT(slotChatMessageReceived(QString,QString,QString)));
	}
}

void YahooAccount::connectWithPassword( const QString &passwd )
{
	kDebug(YAHOO_GEN_DEBUG) ;
	if ( isAway() )
	{
		slotGoOnline();
		return;
	}

	if ( isConnected() ||
	     myself()->onlineStatus() == m_protocol->Connecting )
	{
		kDebug(YAHOO_GEN_DEBUG) << "Yahoo plugin: Ignoring connect request (already connected).";
		return;

	}

	if ( passwd.isNull() )
	{ //cancel the connection attempt
		static_cast<YahooContact*>( myself() )->setOnlineStatus( m_protocol->Offline );
		return;
	}

	QString server = configGroup()->readEntry( "Server", "scsa.msg.yahoo.com" );
	int port = configGroup()->readEntry( "Port", 5050 );

	initConnectionSignals( MakeConnections );

	//YahooSessionManager::manager()->setPager( server, port );
	//m_session = YahooSessionManager::manager()->createSession( accountId(), passwd );
	kDebug(YAHOO_GEN_DEBUG) << "Attempting to connect to Yahoo on <" << server << ":"
		<< port << ">. user <" << accountId() << ">"  << endl;
	static_cast<YahooContact *>( myself() )->setOnlineStatus( m_protocol->Connecting );
	m_session->setStatusOnConnect( Yahoo::Status( initialStatus().internalStatus() ) );
	m_session->connect( server, port, accountId().toLower(), passwd );
}

void YahooAccount::disconnect()
{
	kDebug(YAHOO_GEN_DEBUG) ;

	m_currentMailCount = 0;
	if ( isConnected() )
	{
		kDebug(YAHOO_GEN_DEBUG) <<  "Attempting to disconnect from Yahoo server ";

		disconnected( Manual );
		m_session->close();
		static_cast<YahooContact *>( myself() )->setOnlineStatus( m_protocol->Offline );

		QHash<QString,Kopete::Contact*>::ConstIterator it, itEnd = contacts().constEnd();
		for ( it = contacts().constBegin(); it != itEnd; ++it )
			static_cast<YahooContact *>( it.value() )->setOnlineStatus( m_protocol->Offline );

		static_cast<YahooContact*>( myself() )->setOnlineStatus( m_protocol->Offline );
	}
	else
	{       //make sure we set everybody else offline explicitly, just for cleanup
		kDebug(YAHOO_GEN_DEBUG) << "Cancelling active login attempts (not fully connected).";
		m_session->cancelConnect();

		QHash<QString,Kopete::Contact*>::ConstIterator it, itEnd = contacts().constEnd();
		for ( it = contacts().constBegin(); it != itEnd; ++it )
			static_cast<YahooContact*>( it.value() )->setOnlineStatus( m_protocol->Offline );

		static_cast<YahooContact*>( myself() )->setOnlineStatus( m_protocol->Offline );
	}

	initConnectionSignals( DeleteConnections );
	setupActions( false );
	theHaveContactList = false;
}

void YahooAccount::verifyAccount( const QString &word )
{
	kDebug(YAHOO_GEN_DEBUG) << "Word: s" << word;
	m_session->setVerificationWord( word );
	disconnected( BadPassword );
}

void YahooAccount::setAway(bool status, const QString &awayMessage)
{
	kDebug(YAHOO_GEN_DEBUG) ;

	if( awayMessage.isEmpty() )
		slotGoStatus( status ? 2 : 0 );
	else
		slotGoStatus( status ? 99 : 0, awayMessage );
}

void YahooAccount::slotConnected()
{
	kDebug(YAHOO_GEN_DEBUG) << "Moved to slotLoginResponse for the moment";
}

void YahooAccount::slotGoOnline()
{
	kDebug(YAHOO_GEN_DEBUG) ;
	if( !isConnected() )
		connect( m_protocol->Online );
	else
		slotGoStatus(0);
}

void YahooAccount::slotGoOffline()
{
	if ( isConnected() )
		disconnect();
	else
		static_cast<YahooContact *>( myself() )->setOnlineStatus( m_protocol->Offline );
}

void YahooAccount::fillActionMenu( KActionMenu *actionMenu )
{
//	kDebug(YAHOO_GEN_DEBUG) ;

	Kopete::Account::fillActionMenu( actionMenu );

	actionMenu->addSeparator();
	actionMenu->addAction( m_openInboxAction );
	actionMenu->addAction( m_openYABAction );
	actionMenu->addAction( m_editOwnYABEntry );
	actionMenu->addAction( m_joinChatAction );
}

YahooContact *YahooAccount::contact( const QString &id )
{
	return static_cast<YahooContact *>(contacts().value( id ));
}

bool YahooAccount::createContact(const QString &contactId, Kopete::MetaContact *parentContact )
{
//	kDebug(YAHOO_GEN_DEBUG) << " contactId: " << contactId;

	if(!contact(contactId))
	{
		// FIXME: New Contacts are NOT added to KABC, because:
		// How on earth do you tell if a contact is being deserialised or added brand new here?
			// -- actualy (oct 2004) this method is only called when new contact are added.  but this will
			//    maybe change and you will be noticed   --Olivier
		YahooContact *newContact = new YahooContact( this, contactId,
		                                             parentContact->displayName(), parentContact );
		return newContact != 0;
	}
	else
		kDebug(YAHOO_GEN_DEBUG) << "Contact already exists";

	return false;
}

bool YahooAccount::createChatContact(const QString &nick)
{
	Kopete::MetaContact *m = new Kopete::MetaContact;
	m->setTemporary( true );
	return createContact( nick, m );
}

void YahooAccount::sendFile( YahooContact *to, const KUrl &url )
{
	QFile file( url.toLocalFile() );

	Kopete::Transfer *transfer = Kopete::TransferManager::transferManager()->addTransfer ( to,
		url.fileName(), file.size(), to->userId(), Kopete::FileTransferInfo::Outgoing );
	m_session->sendFile( transfer->info().transferId(), to->userId(), QString(), url );

	QObject::connect( transfer, SIGNAL(result(KJob*)), this, SLOT(slotFileTransferResult(KJob*)) );

	m_fileTransfers.insert( transfer->info().transferId(), transfer );
}

void YahooAccount::setupActions( bool connected )
{
	m_joinChatAction->setEnabled( connected );
	m_editOwnYABEntry->setEnabled( connected );
}

/***************************************************************************
 *                                                                         *
 *   Slot for KYahoo signals                                               *
 *                                                                         *
 ***************************************************************************/

void YahooAccount::slotLoginResponse( int succ , const QString &url )
{
	kDebug(YAHOO_GEN_DEBUG) << succ << ", " << url << ")]";
	QString errorMsg;
	setupActions( succ == Yahoo::LoginOk );
	if ( succ == Yahoo::LoginOk || (succ == Yahoo::LoginDupl && m_lastDisconnectCode == 2) )
	{
		if ( initialStatus().internalStatus() )
		{
			static_cast<YahooContact *>( myself() )->setOnlineStatus( initialStatus() );
		}
		else
		{
			static_cast<YahooContact *>( myself() )->setOnlineStatus( m_protocol->Online );
		}


		setBuddyIcon( myself()->property( Kopete::Global::Properties::self()->photo() ).value().toString() );
		m_session->getYABEntries( m_YABLastMerge, m_YABLastRemoteRevision );
		IDs.clear();
		m_lastDisconnectCode = 0;
		theHaveContactList = true;
		return;
	}
	else if(succ == Yahoo::LoginPasswd)
	{
		initConnectionSignals( DeleteConnections );
		password().setWrong();
		static_cast<YahooContact *>( myself() )->setOnlineStatus( m_protocol->Offline );
		disconnected( BadPassword );
		return;
	}
	else if(succ == Yahoo::LoginLock)
	{
		initConnectionSignals( DeleteConnections );
		errorMsg = i18n("Could not log into the Yahoo service: your account has been locked.\nVisit %1 to reactivate it.", url);
		KMessageBox::queuedMessageBox(Kopete::UI::Global::mainWidget(), KMessageBox::Error, errorMsg);
		static_cast<YahooContact *>( myself() )->setOnlineStatus( m_protocol->Offline );
		disconnected( BadUserName ); // FIXME: add a more appropriate disconnect reason
		return;
	}
	else if( succ == Yahoo::LoginUname )
	{
		initConnectionSignals( DeleteConnections );
		errorMsg = i18n("Could not log into the Yahoo service: the username specified was invalid.");
		KMessageBox::queuedMessageBox(Kopete::UI::Global::mainWidget(), KMessageBox::Error, errorMsg);
		static_cast<YahooContact *>( myself() )->setOnlineStatus( m_protocol->Offline );
		disconnected( BadUserName );
		return;
	}
	else if( succ == Yahoo::LoginDupl && m_lastDisconnectCode != 2 )
	{
		initConnectionSignals( DeleteConnections );
		errorMsg = i18n("You have been logged out of the Yahoo service, possibly due to a duplicate login.");
		KMessageBox::queuedMessageBox(Kopete::UI::Global::mainWidget(), KMessageBox::Error, errorMsg);
		static_cast<YahooContact *>( myself() )->setOnlineStatus( m_protocol->Offline );
		disconnected( Manual ); // cannot use ConnectionReset since that will auto-reconnect
		return;
	}
	else if( succ == Yahoo::LoginVerify )
	{
		initConnectionSignals( DeleteConnections );
		static_cast<YahooContact *>( myself() )->setOnlineStatus( m_protocol->Offline );
		YahooVerifyAccount *verifyDialog = new YahooVerifyAccount( this );
		verifyDialog->setUrl( KUrl(url) );
		verifyDialog->show();
		return;
	}

	//If we get here, something went wrong, so set ourselves to offline
	initConnectionSignals( DeleteConnections );
	errorMsg = i18nc("@info", "Could not log into the Yahoo service. Error code: <message><numid>%1</numid></message>.", succ);
	KMessageBox::queuedMessageBox(Kopete::UI::Global::mainWidget(), KMessageBox::Error, errorMsg);
	static_cast<YahooContact *>( myself() )->setOnlineStatus( m_protocol->Offline );
	disconnected( Unknown );
}

void YahooAccount::slotDisconnected()
{
	kDebug(YAHOO_GEN_DEBUG) ;
	initConnectionSignals( DeleteConnections );
	setupActions( false );
	if( !isConnected() )
		return;
	static_cast<YahooContact *>( myself() )->setOnlineStatus( m_protocol->Offline );
	disconnected( ConnectionReset );	// may reconnect

	if ( isBusy() )
		return;

	QString message;
	message = i18n( "%1 has been disconnected.\nError message:\n%2 - %3" ,
		  accountId(), m_session->error(), m_session->errorString() );
	KNotification::event( "connection_lost", message, myself()->onlineStatus().protocolIcon(KIconLoader::SizeMedium) );
}

void YahooAccount::slotLoginFailed()
{
	kDebug(YAHOO_GEN_DEBUG) ;
	initConnectionSignals( DeleteConnections );
	static_cast<YahooContact *>( myself() )->setOnlineStatus( m_protocol->Offline );
	disconnected( Manual );			// don't reconnect

	if ( isBusy() )
		return;

	QString message;
	message = i18n( "There was an error while connecting %1 to the Yahoo server.\nError message:\n%2 - %3" ,
		  accountId(), m_session->error(), m_session->errorString() );
	KNotification::event( "cannot_connect", message, myself()->onlineStatus().protocolIcon(KIconLoader::SizeMedium) );
}

void YahooAccount::slotError( int level )
{
	// enum LogLevel { Debug, Info, Notice, Warning, Error, Critical };
	if( level <= Client::Notice )
		return;
	else if( level <= Client::Warning )
		KMessageBox::information( Kopete::UI::Global::mainWidget(), i18n( "%1\n\nReason: %2", 
			m_session->errorInformation(),  m_session->errorString() ), i18n( "Yahoo Plugin" ) );
	else
		KMessageBox::error( Kopete::UI::Global::mainWidget(), i18n( "%1\n\nReason: %2", 
			m_session->errorInformation(),  m_session->errorString() ), i18n( "Yahoo Plugin" ) );
}

void YahooAccount::slotGotBuddy( const QString &userid, const QString &alias, const QString &group )
{
	kDebug(YAHOO_GEN_DEBUG) ;
	IDs[userid] = QPair<QString, QString>(group, alias);

	// Serverside -> local
	if ( !contact( userid ) )
	{
		kDebug(YAHOO_GEN_DEBUG) << "SS Contact " << userid << " is not in the contact list. Adding...";
		Kopete::Group *g=Kopete::ContactList::self()->findGroup(group);
		addContact(userid, alias.isEmpty() ? userid : alias, g, Kopete::Account::ChangeKABC);
	}

	kDebug(YAHOO_GEN_DEBUG) << IDs;
}

void YahooAccount::slotBuddyAddResult( const QString &userid, const QString &group, bool success )
{
     kDebug(YAHOO_GEN_DEBUG) << success;

     if(success)
	  IDs[userid] = QPair<QString, QString>(group, QString());

     kDebug(YAHOO_GEN_DEBUG) << IDs;
}

void YahooAccount::slotBuddyRemoveResult( const QString &userid, const QString & /*group*/, bool /*success*/ )
{
     kDebug(YAHOO_GEN_DEBUG);

     // Ignore success here, the only reason this will fail is because the
     // contact isn't on the server's list, so we shouldn't have them in our
     // list either.
     IDs.remove(userid);

     kDebug(YAHOO_GEN_DEBUG) << IDs;
}

void YahooAccount::slotBuddyChangeGroupResult(const QString &userid, const QString &group, bool success)
{
     kDebug(YAHOO_GEN_DEBUG);
     
     if(success)
	  IDs[userid] = QPair<QString, QString>(group, QString());

     kDebug(YAHOO_GEN_DEBUG) << IDs;
}

void YahooAccount::slotAuthorizationAccepted( const QString &who )
{
	kDebug(YAHOO_GEN_DEBUG) ;

	if ( isBusy() )
		return;

	QString message;
	message = i18n( "User %1 has granted your authorization request." ,
		  who );
	KNotification::event( QLatin1String("kopete_authorization"), message );

	if( contact( who ) )
		contact( who )->setOnlineStatus( m_protocol->Online );
}

void YahooAccount::slotAuthorizationRejected( const QString &who, const QString &msg )
{
	kDebug(YAHOO_GEN_DEBUG) ;

	if ( isBusy() )
		return;

	QString message;
	message = i18n( "User %1 has rejected your authorization request.\n%2" ,
		  who, msg );
	KNotification::event( QLatin1String("kopete_authorization"), message );
}

void YahooAccount::slotgotAuthorizationRequest( const QString &user, const QString &msg, const QString &name )
{
	kDebug(YAHOO_GEN_DEBUG) ;
	Q_UNUSED( msg );
	Q_UNUSED( name );
	YahooContact *kc = contact( user );
	Kopete::MetaContact *metaContact=0L;
	if(kc)
		metaContact=kc->metaContact();

	Kopete::AddedInfoEvent::ShowActionOptions actions = Kopete::AddedInfoEvent::AuthorizeAction;
	actions |= Kopete::AddedInfoEvent::BlockAction;
	if( !metaContact || metaContact->isTemporary() )
		actions |= Kopete::AddedInfoEvent::AddAction;

	Kopete::AddedInfoEvent* event = new Kopete::AddedInfoEvent( user, this );
	QObject::connect( event, SIGNAL(actionActivated(uint)),
	                  this, SLOT(slotAddedInfoEventActionActivated(uint)) );
	
	event->showActions( actions );
	event->sendEvent();
}

void YahooAccount::slotAddedInfoEventActionActivated( uint actionId )
{
	const Kopete::AddedInfoEvent *event = dynamic_cast<const Kopete::AddedInfoEvent *>(sender());
	if( !event || !isConnected() )
		return;

	switch ( actionId )
	{
	case Kopete::AddedInfoEvent::AuthorizeAction:
		m_session->sendAuthReply( event->contactId(), true, QString() );
		break;
	case Kopete::AddedInfoEvent::BlockAction:
		m_session->sendAuthReply( event->contactId(), false, QString() );
		break;
	case Kopete::AddedInfoEvent::AddContactAction:
		event->addContact();
		break;
	}
}

void YahooAccount::slotGotIgnore( const QStringList & /* igns */ )
{
	//kDebug(YAHOO_GEN_DEBUG) ;
}

void YahooAccount::slotGotIdentities( const QStringList & /* ids */ )
{
	//kDebug(YAHOO_GEN_DEBUG) ;
}

void YahooAccount::slotStatusChanged( const QString &who, int stat, const QString &msg, int away, int idle, int pictureChecksum )
{
	kDebug(YAHOO_GEN_DEBUG) << who << " status: " << stat << " msg: " << msg << " away: " << away << " idle: " << idle;
	YahooContact *kc = contact( who );

	if( contact( who ) == myself() )
		return;

	if ( kc )
	{
		Kopete::OnlineStatus newStatus = m_protocol->statusFromYahoo( stat );
		Kopete::OnlineStatus oldStatus = kc->onlineStatus();

		if( newStatus == m_protocol->Custom ) {
			if( away == 0 )
				newStatus =m_protocol->Online;
			kc->setStatusMessage( msg );
		}
		else
			kc->setStatusMessage( Kopete::StatusMessage() );

		//if( newStatus == static_cast<YahooProtocol*>( m_protocol )->Idle ) {
		if( newStatus == m_protocol->Idle )
			kc->setIdleTime( idle ? idle : 1 );
		else
			kc->setIdleTime( 0 );

		kc->setOnlineStatus( newStatus );

		slotGotBuddyIconChecksum( who, pictureChecksum );
	}
}

void YahooAccount::slotStealthStatusChanged( const QString &who, Yahoo::StealthStatus /*state*/ )
{
	//kDebug(YAHOO_GEN_DEBUG) << "Stealth Status of " << who << "changed to " << state;

	YahooContact* kc = contact( who );
	if ( kc == NULL ) {
		kDebug(YAHOO_GEN_DEBUG) << "contact " << who << " doesn't exist.";
		return;
	}
	kc->updateStealthed();
}

QString YahooAccount::prepareIncomingMessage( const QString &messageText )
{
	QString newMsgText( messageText );
	QRegExp regExp;
	int pos = 0;
	newMsgText = stripMsgColorCodes( newMsgText );

	kDebug(YAHOO_GEN_DEBUG) << "Message after stripping color codes '" << newMsgText << "'";

	newMsgText.replace( QLatin1String( "&" ), QString::fromLatin1( "&amp;" ) );

	// Replace Font tags
	regExp.setMinimal( true );
	regExp.setPattern( "<font([^>]*)size=\"([^>]*)\"([^>]*)>" );
	pos = 0;
	while ( pos >= 0 ) {
		pos = regExp.indexIn( newMsgText, pos );
		if ( pos >= 0 ) {
			pos += regExp.matchedLength();
		newMsgText.replace( regExp, QLatin1String("<font\\1style=\"font-size:\\2pt\">" ) );
		}
	}

	// Remove FADE and ALT tags
	regExp.setPattern( "<[/]*FADE([^>]*)>" );
	pos = 0;
	while ( pos >= 0 ) {
		pos = regExp.indexIn( newMsgText, pos );
		if ( pos >= 0 ) {
			pos += regExp.matchedLength();
			newMsgText.remove( regExp );

		}
	}
	regExp.setPattern( "<[/]*ALT([^>]*)>" );
	pos = 0;
	while ( pos >= 0 ) {
		pos = regExp.indexIn( newMsgText, pos );
		if ( pos >= 0 ) {
			pos += regExp.matchedLength();
			newMsgText.remove( regExp );
		}
	}

	// Replace < and > in text
	regExp.setPattern( "<(?!(/*(font.*|[\"fbui])>))" );
	pos = 0;
	while ( pos >= 0 ) {
		pos = regExp.indexIn( newMsgText, pos );
		if ( pos >= 0 ) {
			pos += regExp.matchedLength();
			newMsgText.replace( regExp, QLatin1String("&lt;" ) );
		}
	}
	regExp.setPattern( "([^\"bui])>" );
	pos = 0;
	while ( pos >= 0 ) {
		pos = regExp.indexIn( newMsgText, pos );
		if ( pos >= 0 ) {
			pos += regExp.matchedLength();
			newMsgText.replace( regExp, QLatin1String("\\1&gt;" ) );
		}
	}

	// add closing tags when needed
	regExp.setMinimal( false );
	regExp.setPattern( "(<b>.*)(?!</b>)" );
	newMsgText.replace( regExp, QLatin1String("\\1</b>" ) );
	regExp.setPattern( "(<i>.*)(?!</i>)" );
	newMsgText.replace( regExp, QLatin1String("\\1</i>" ) );
	regExp.setPattern( "(<u>.*)(?!</u>)" );
	newMsgText.replace( regExp, QLatin1String("\\1</u>" ) );
	regExp.setPattern( "(<font.*)(?!</font>)" );
	newMsgText.replace( regExp, QLatin1String("\\1</font>" ) );

	newMsgText.replace( QLatin1String( "\r" ), QLatin1String( "<br/>" ) );

	return newMsgText;
}

void YahooAccount::slotGotIm( const QString &who, const QString &msg, long tm, int /*stat*/)
{
	QFont msgFont;
	QDateTime msgDT;
	Kopete::ContactPtrList justMe;

	if( !contact( who ) )
	{
		kDebug(YAHOO_GEN_DEBUG) << "Adding contact " << who;
		addContact( who,who,  0L, Kopete::Account::Temporary );
	}

	//Parse the message for it's properties
	kDebug(YAHOO_GEN_DEBUG) << "Original message is '" << msg << "'";
	//kDebug(YAHOO_GEN_DEBUG) << "Message color is " << getMsgColor(msg);
	QColor fgColor = getMsgColor( msg );

	if (tm == 0)
		msgDT = QDateTime( QDate::currentDate(), QTime::currentTime(), Qt::LocalTime );
	else
		msgDT = QDateTime::fromTime_t(tm);

	QString newMsgText = prepareIncomingMessage( msg );

	kDebug(YAHOO_GEN_DEBUG) << "Message after fixing font tags '" << newMsgText << "'";

	Kopete::ChatSession *mm = contact(who)->manager(Kopete::Contact::CanCreate);

	// Tell the message manager that the buddy is done typing
	mm->receivedTypingMsg(contact(who), false);

	justMe.append(myself());

	Kopete::Message kmsg(contact(who), justMe);
	kmsg.setTimestamp( msgDT );
	kmsg.setHtmlBody( newMsgText );
	kmsg.setDirection( Kopete::Message::Inbound );

	kmsg.setForegroundColor( fgColor );
	mm->appendMessage(kmsg);
}

void YahooAccount::slotGotBuzz( const QString &who, long tm )
{
	QFont msgFont;
	QDateTime msgDT;
	Kopete::ContactPtrList justMe;

	if( !contact( who ) )
	{
		kDebug(YAHOO_GEN_DEBUG) << "Adding contact " << who;
		addContact( who,who,  0L, Kopete::Account::Temporary );
	}

	if (tm == 0)
		msgDT = QDateTime( QDate::currentDate(), QTime::currentTime(), Qt::LocalTime );
	else
		msgDT = QDateTime::fromTime_t(tm);

	justMe.append(myself());

	QString buzzMsgText = i18nc("This string is shown when the user is buzzed by a contact", "Buzz");

	Kopete::Message kmsg(contact(who), justMe);
	kmsg.setTimestamp( msgDT );
	kmsg.setDirection( Kopete::Message::Inbound );
	kmsg.setPlainBody( buzzMsgText );
	kmsg.setType( Kopete::Message::TypeAction );
	
	QColor fgColor( "gold" );
	kmsg.setForegroundColor( fgColor );

	Kopete::ChatSession *mm = contact(who)->manager(Kopete::Contact::CanCreate);
	mm->appendMessage(kmsg);
	// Emit the buzz notification.
	mm->emitNudgeNotification();
}

void YahooAccount::slotGotConfInvite( const QString & who, const QString & room, const QString &msg, const QStringList &members )
{
	kDebug(YAHOO_GEN_DEBUG) << who << " has invited you to join the conference \"" << room << "\" : " << msg;
	kDebug(YAHOO_GEN_DEBUG) << "Members: " << members;

	if( !m_pendingConfInvites.contains( room ) )	// We have to keep track of the invites as the server will send the same invite twice if it gets canceled by the host
		m_pendingConfInvites.push_back( room );
	else
	{
		return;
	}

	QString m = who;
	QStringList myMembers;
	myMembers.push_back( who );
	for( QStringList::const_iterator it = ++members.constBegin(); it != members.constEnd(); ++it )
	{
		if( *it != m_session->userId() )
		{
			m.append( QString(", %1").arg( *it ) );
			myMembers.push_back( *it );
		}
	}
	if( KMessageBox::Yes == KMessageBox::questionYesNo( Kopete::UI::Global::mainWidget(),
		i18n("%1 has invited you to join a conference with %2.\n\nHis/her message: %3\n\nAccept?",
				who, m, msg), QString(), KGuiItem( i18nc("@action","Accept") ), KGuiItem( i18nc("@action","Close") ) ) )
	{
		m_session->joinConference( room, myMembers );
		if( !m_conferences[room] )
		{
			Kopete::ContactPtrList others;
			YahooConferenceChatSession *session = new YahooConferenceChatSession( room, protocol(), myself(), others );
			m_conferences[room] = session;

			QObject::connect( session, SIGNAL(leavingConference(YahooConferenceChatSession*)), this, SLOT(slotConfLeave(YahooConferenceChatSession*)) );

			for ( QStringList::ConstIterator it = myMembers.constBegin(); it != myMembers.constEnd(); ++it )
			{
				YahooContact * c = contact( *it );
				if ( !c )
				{
					kDebug(YAHOO_GEN_DEBUG) << "Adding contact " << *it << " to conference.";
					addContact( *it,*it,  0L, Kopete::Account::Temporary );
					c = contact( *it );
				}
				session->joined( c );
			}
			session->view( true )->raise( false );
		}
	}
	else
		m_session->declineConference( room, myMembers, QString() );

	m_pendingConfInvites.removeAll( room );
}

void YahooAccount::prepareConference( const QString &who )
{
	QString room;
	for( int i = 0; i < 22; i++ )
	{
		char c = rand()%52;
		room += (c > 25)  ? c + 71 : c + 65;
	}
	room = QString("%1-%2--").arg(accountId()).arg(room);
	kDebug(YAHOO_GEN_DEBUG) << "The generated roomname is: " << room;

	QStringList buddies;
	QHash<QString,Kopete::Contact*>::ConstIterator it, itEnd = contacts().constEnd();
	for( it = contacts().constBegin(); it != itEnd; ++it )
	{
		buddies.push_back( it.value()->contactId() );
	}

	YahooInviteListImpl *dlg = new YahooInviteListImpl( Kopete::UI::Global::mainWidget() );
	QObject::connect( dlg, SIGNAL(readyToInvite(QString,QStringList,QStringList,QString)),
			this, SLOT(slotInviteConference(QString,QStringList,QStringList,QString)) );
	dlg->setRoom( room );
	dlg->fillFriendList( buddies );
	dlg->addInvitees( QStringList( who ) );
	dlg->show();
}

void YahooAccount::slotInviteConference( const QString &room, const QStringList &members, const QStringList &participants, const QString &msg )
{
	Q_UNUSED( participants );
kDebug(YAHOO_GEN_DEBUG) << "Inviting " << members << " to the conference " << room << ". Message: " << msg;
	m_session->inviteConference( room, members, msg );

	Kopete::ContactPtrList others;
	YahooConferenceChatSession *session = new YahooConferenceChatSession( room, protocol(), myself(), others );
	m_conferences[room] = session;

	QObject::connect( session, SIGNAL(leavingConference(YahooConferenceChatSession*)), this, SLOT(slotConfLeave(YahooConferenceChatSession*)) );

	session->joined( static_cast< YahooContact *>(myself()) );
	session->view( true )->raise( false );
}

void YahooAccount::slotAddInviteConference( const QString &room, const QStringList &who, const QStringList &members, const QString &msg )
{
	kDebug(YAHOO_GEN_DEBUG) << "Inviting " << who << " to the conference " << room << ". Message: " << msg;
	m_session->addInviteConference( room, who, members, msg );
}

void YahooAccount::slotConfUserDecline( const QString &who, const QString &room, const QString &msg)
{
	kDebug(YAHOO_GEN_DEBUG) ;

	if( !m_conferences.contains( room ) )
	{
		kDebug(YAHOO_GEN_DEBUG) << "Error. No chatsession for this conference found.";
		return;
	}

	YahooConferenceChatSession *session = m_conferences[room];

	QString body = i18n( "%1 has declined to join the conference: \"%2\"", who, msg );
	Kopete::Message message = Kopete::Message( contact( who ), myself() );
	message.setPlainBody( body );
	message.setDirection( Kopete::Message::Internal );

	session->appendMessage( message );
}

void YahooAccount::slotConfUserJoin( const QString &who, const QString &room )
{
	kDebug(YAHOO_GEN_DEBUG) ;
	if( !m_conferences.contains( room ) )
	{
		kDebug(YAHOO_GEN_DEBUG) << "Error. No chatsession for this conference found.";
		return;
	}

	YahooConferenceChatSession *session = m_conferences[room];
	if( !contact( who ) )
	{
		addContact( who, who,  0L, Kopete::Account::Temporary );
	}
	session->joined( contact( who ) );
}

void YahooAccount::slotConfUserLeave( const QString & who, const QString &room )
{
	kDebug(YAHOO_GEN_DEBUG) ;
	if( !m_conferences.contains( room ) )
	{
		kDebug(YAHOO_GEN_DEBUG) << "Error. No chatsession for this conference found.";
		return;
	}

	YahooConferenceChatSession *session = m_conferences[room];
	if( !contact( who ) )
	{
		addContact( who, who,  0L, Kopete::Account::Temporary );
	}
	session->left( contact( who ) );
}

void YahooAccount::slotConfLeave( YahooConferenceChatSession *s )
{
	kDebug(YAHOO_GEN_DEBUG) ;
	if( !s )
		return;
	QStringList members;
	for( Kopete::ContactPtrList::ConstIterator it = s->members().constBegin(); it != s->members().constEnd(); ++it )
	{
		if( (*it) == myself() )
			continue;
		kDebug(YAHOO_GEN_DEBUG) << "Member: " << (*it)->contactId();
		members.append( (*it)->contactId() );
	}
	m_session->leaveConference( s->room(), members );
	m_conferences.remove( s->room() );
}

void YahooAccount::slotConfMessage( const QString &who, const QString &room, const QString &msg )
{
	kDebug(YAHOO_GEN_DEBUG) ;

	if( !m_conferences.contains( room ) )
	{
		kDebug(YAHOO_GEN_DEBUG) << "Error. No chatsession for this conference found.";
		return;
	}

	YahooConferenceChatSession *session = m_conferences[room];

	QFont msgFont;
	QDateTime msgDT;
	Kopete::ContactPtrList justMe;

	if( !contact( who ) )
	{
		kDebug(YAHOO_GEN_DEBUG) << "Adding contact " << who;
		addContact( who,who,  0L, Kopete::Account::Temporary );
	}
	kDebug(YAHOO_GEN_DEBUG) << "Original message is '" << msg << "'";

	QColor fgColor = getMsgColor( msg );
	msgDT.setTime_t(time(0L));

	QString newMsgText = prepareIncomingMessage( msg );

	kDebug(YAHOO_GEN_DEBUG) << "Message after fixing font tags '" << newMsgText << "'";
	session->receivedTypingMsg(contact(who), false);

	justMe.append(myself());

	Kopete::Message kmsg( contact(who), justMe );
	kmsg.setTimestamp( msgDT );
	kmsg.setHtmlBody( newMsgText );
	kmsg.setDirection( Kopete::Message::Inbound );

	kmsg.setForegroundColor( fgColor );
	session->appendMessage(kmsg);
}

void YahooAccount::sendConfMessage( YahooConferenceChatSession *s, const Kopete::Message &message )
{
	kDebug(YAHOO_GEN_DEBUG) ;
	QStringList members;
	for( Kopete::ContactPtrList::ConstIterator it = s->members().constBegin(); it != s->members().constEnd(); ++it )
	{
		if( (*it) == myself() )
			continue;
		kDebug(YAHOO_GEN_DEBUG) << "Member: " << (*it)->contactId();
		members.append( (*it)->contactId() );
	}
	m_session->sendConferenceMessage( s->room(), members, YahooContact::prepareMessage( message.escapedBody() ) );
}

void YahooAccount::slotGotYABRevision( long rev, bool merged )
{
	if( merged )
	{
		kDebug(YAHOO_GEN_DEBUG) << "Merge Revision received: " << rev;
		configGroup()->writeEntry( "YABLastMerge", (qlonglong)rev );
		m_YABLastMerge = rev;
	}
	else
	{
		kDebug(YAHOO_GEN_DEBUG) << "Remote Revision received: " << rev;
		configGroup()->writeEntry( "YABLastRemoteRevision", (qlonglong)rev );
		m_YABLastRemoteRevision = rev;
	}
}

void YahooAccount::slotGotYABEntry( YABEntry *entry )
{
	YahooContact* kc = contact( entry->yahooId );
	if( !kc )
	{
		kDebug(YAHOO_GEN_DEBUG) << "YAB entry received for a contact not on our buddylist: " << entry->yahooId;
		delete entry;
	}
	else
	{
		kDebug(YAHOO_GEN_DEBUG) << "YAB entry received for: " << entry->yahooId;
		if( entry->source == YABEntry::SourceYAB )
		{
			kc->setYABEntry( entry );
		}
		else if( entry->source == YABEntry::SourceContact )
		{
			entry->YABId = kc->yabEntry()->YABId;
			YahooUserInfoDialog *dlg = new YahooUserInfoDialog( kc, Kopete::UI::Global::mainWidget() );
			dlg->setData( *entry );
			dlg->setAccountConnected( isConnected() );
			dlg->show();
			QObject::connect( dlg, SIGNAL(saveYABEntry(YABEntry&)), this, SLOT(slotSaveYABEntry(YABEntry&)));
			delete entry;
		}
	}
}

void YahooAccount::slotSaveYABEntry( YABEntry &entry )
{
	kDebug(YAHOO_GEN_DEBUG) << "YABId: " << entry.YABId;
	if( entry.YABId > 0 )
		m_session->saveYABEntry( entry );
	else
		m_session->addYABEntry( entry );
}

void YahooAccount::slotModifyYABEntryError( YABEntry *entry, const QString &msg )
{
	YahooContact* kc = contact( entry->yahooId );
	if( kc )
		kc->setYABEntry( entry, true );
	KMessageBox::queuedMessageBox( Kopete::UI::Global::mainWidget(), KMessageBox::Sorry, msg, i18n( "Yahoo Plugin" ) );
}

void YahooAccount::slotGotFile( const QString &  who, const QString &  url , long /* expires */, const QString &  msg , const QString &  fname, unsigned long  fesize, const QPixmap &preview )
{
	kDebug(YAHOO_GEN_DEBUG) << "Received File from " << who << ": " << msg;
	kDebug(YAHOO_GEN_DEBUG) << "Filename :" << fname << " size:" << fesize;

	Kopete::TransferManager::transferManager()->askIncomingTransfer( contact( who ) , fname, fesize, msg, url, preview );

	if( m_pendingFileTransfers.empty() )
	{
	QObject::connect( Kopete::TransferManager::transferManager(), SIGNAL(accepted(Kopete::Transfer*,QString)),
					this, SLOT(slotReceiveFileAccepted(Kopete::Transfer*,QString)) );
	QObject::connect( Kopete::TransferManager::transferManager(), SIGNAL(refused(Kopete::FileTransferInfo)),
	                  this, SLOT(slotReceiveFileRefused(Kopete::FileTransferInfo)) );
	}
	m_pendingFileTransfers.append( url );
}

void YahooAccount::slotReceiveFileAccepted(Kopete::Transfer *transfer, const QString& fileName)
{
	kDebug(YAHOO_GEN_DEBUG) ;
	if( !m_pendingFileTransfers.contains( transfer->info().internalId() ) )
		return;

	m_pendingFileTransfers.removeAll( transfer->info().internalId() );

	//Create directory if it doesn't already exist
	QDir dir;
	QString path = QFileInfo( fileName ).path();
	if( !dir.exists( path ) )
	{
		dir.mkpath( path );
	}

	m_session->receiveFile( transfer->info().transferId(), transfer->info().contact()->contactId(), transfer->info().internalId(), fileName );
	m_fileTransfers.insert( transfer->info().transferId(), transfer );
	QObject::connect( transfer, SIGNAL(result(KJob*)), this, SLOT(slotFileTransferResult(KJob*)) );

	if( m_pendingFileTransfers.empty() )
	{
		QObject::disconnect( Kopete::TransferManager::transferManager(), SIGNAL(accepted(Kopete::Transfer*,QString)),
							this, SLOT(slotReceiveFileAccepted(Kopete::Transfer*,QString)) );
		QObject::disconnect( Kopete::TransferManager::transferManager(), SIGNAL(refused(Kopete::FileTransferInfo)),
						this, SLOT(slotReceiveFileRefused(Kopete::FileTransferInfo)) );
	}
}

void YahooAccount::slotReceiveFileRefused( const Kopete::FileTransferInfo& info )
{
	if( !m_pendingFileTransfers.contains( info.internalId() ) )
		return;

	m_pendingFileTransfers.removeAll( info.internalId() );
	m_session->rejectFile( info.contact()->contactId(), info.internalId() );

	if( m_pendingFileTransfers.empty() )
	{
		QObject::disconnect( Kopete::TransferManager::transferManager(), SIGNAL(accepted(Kopete::Transfer*,QString)),
							this, SLOT(slotReceiveFileAccepted(Kopete::Transfer*,QString)) );
		QObject::disconnect( Kopete::TransferManager::transferManager(), SIGNAL(refused(Kopete::FileTransferInfo)),
						this, SLOT(slotReceiveFileRefused(Kopete::FileTransferInfo)) );
	}
}

void YahooAccount::slotFileTransferBytesProcessed( unsigned int transferId, unsigned int bytes )
{
	kDebug(YAHOO_GEN_DEBUG) << "Transfer: " << transferId << " Bytes:" << bytes;
	Kopete::Transfer *t = m_fileTransfers[transferId];
	if( !t )
		return;

	t->slotProcessed( bytes );
}

void YahooAccount::slotFileTransferComplete( unsigned int transferId )
{
	kDebug(YAHOO_GEN_DEBUG) ;
	Kopete::Transfer *t = m_fileTransfers[transferId];
	if( !t )
		return;

	t->slotComplete();
	m_fileTransfers.remove( transferId );
}

void YahooAccount::slotFileTransferError( unsigned int transferId, int error, const QString &desc )
{
	kDebug(YAHOO_GEN_DEBUG) ;
	Kopete::Transfer *t = m_fileTransfers[transferId];
	if( !t )
		return;

	t->slotError( error, desc );
	m_fileTransfers.remove( transferId );
}

void YahooAccount::slotFileTransferResult( KJob *job )
{
	kDebug(YAHOO_GEN_DEBUG) ;
	const Kopete::Transfer *t = dynamic_cast< const Kopete::Transfer * >( job );

	if( !t )
		return;

	if( t->error() == KIO::ERR_USER_CANCELED )
	{
		m_session->cancelFileTransfer( t->info().transferId() );
		m_fileTransfers.remove( t->info().transferId() );
	}
}

void YahooAccount::slotContactAdded( const QString & /* myid */, const QString & /* who */, const QString & /* msg */ )
{
//	kDebug(YAHOO_GEN_DEBUG) << myid << " " << who << " " << msg;
}

void YahooAccount::slotRejected( const QString & /* who */, const QString & /* msg */ )
{
//	kDebug(YAHOO_GEN_DEBUG) ;
}

void YahooAccount::slotTypingNotify( const QString &who, int what )
{
	emit receivedTypingMsg(who, what);
}

void YahooAccount::slotGameNotify( const QString & /* who */, int /* stat */ )
{
//	kDebug(YAHOO_GEN_DEBUG) ;
}

void YahooAccount::slotMailNotify( const QString& from, const QString&  subject , int cnt )
{
	kDebug(YAHOO_GEN_DEBUG) << "Mail count: " << cnt;

	if ( isBusy() )
		return;

	if ( cnt > 0 && from.isEmpty() )
	{
		QObject::connect(KNotification::event( QLatin1String("yahoo_mail"), i18np( "You have one unread message in your Yahoo inbox.",
			"You have %1 unread messages in your Yahoo inbox.", cnt ), QPixmap() , Kopete::UI::Global::mainWidget() ),
		                 SIGNAL(activated(uint)) , this, SLOT(slotOpenInbox()) );

		m_currentMailCount = cnt;
	}
	else if ( cnt > 0 )
	{	kDebug(YAHOO_GEN_DEBUG) << "attempting to trigger event";
	
		QObject::connect(KNotification::event( QLatin1String("yahoo_mail"), i18n( "%1 has a message from %2 in your Yahoo inbox. <br><br>Subject: %3", m_session->userId(), from, subject )
		                                       , QPixmap() , Kopete::UI::Global::mainWidget() ), SIGNAL(activated(uint)) , this, SLOT(slotOpenInbox()) );
		m_currentMailCount = cnt;
	}
}

void YahooAccount::slotSystemMessage( const QString & /* msg */ )
{
//	kDebug(YAHOO_GEN_DEBUG) << msg;
}

void YahooAccount::slotRemoveHandler( int /* fd */ )
{
//	kDebug(YAHOO_GEN_DEBUG) ;
}

void YahooAccount::slotGotWebcamInvite( const QString& who )
{
	YahooContact* kc = contact( who );
	if ( kc == NULL ) {
		kDebug(YAHOO_GEN_DEBUG) << "contact " << who << " doesn't exist.";
		return;
	}

	if( m_pendingWebcamInvites.contains( who ) )
		return;

	m_pendingWebcamInvites.append( who );

	if( KMessageBox::Yes == KMessageBox::questionYesNo( Kopete::UI::Global::mainWidget(), i18n("%1 has invited you to view his/her webcam. Accept?", who),
                            QString(), KGuiItem( i18nc("@action","Accept") ), KGuiItem( i18nc("@action","Close") ) ) )
	{
		m_pendingWebcamInvites.removeAll( who );
		m_session->requestWebcam( who );
	}
}
void YahooAccount::slotWebcamNotAvailable( const QString &who )
{
	KMessageBox::queuedMessageBox( Kopete::UI::Global::mainWidget(), KMessageBox::Sorry, i18n("Webcam for %1 is not available.", who), i18n( "Yahoo Plugin" ) );
}

void YahooAccount::slotGotWebcamImage( const QString& who, const QPixmap& image )
{
	YahooContact* kc = contact( who );
	if ( kc == NULL ) {
		kDebug(YAHOO_GEN_DEBUG) << "contact " << who << " doesn't exist.";
		return;
	}
	kc->receivedWebcamImage( image );
}

void YahooAccount::slotPictureStatusNotify( const QString &who, int status)
{
	YahooContact *kc = contact( who );
	if ( kc == NULL ) {
		kDebug(YAHOO_GEN_DEBUG) << "contact " << who << " doesn't exist.";
		return;
	}

	kDebug(YAHOO_GEN_DEBUG) << "contact " << who << " changed picture status to" << status;
}

void YahooAccount::slotGotBuddyIconChecksum(const QString &who, int checksum)
{
	YahooContact *kc = contact( who );
	if ( kc == NULL ) {
		kDebug(YAHOO_GEN_DEBUG) << "contact " << who << " doesn't exist.";
		return;
	}

	if ( checksum == kc->property( YahooProtocol::protocol()->iconCheckSum ).value().toInt() &&
	     QFile::exists( KStandardDirs::locateLocal( "appdata", "yahoopictures/"+ who.toLower().replace(QRegExp("[./~]"),"-")  +".png" ) ) )
	{
		kDebug(YAHOO_GEN_DEBUG) << "Icon already exists. I will not request it again.";
		return;
	} else
		m_session->requestPicture( who );
}

void YahooAccount::slotGotBuddyIconInfo(const QString &who, KUrl url, int checksum)
{
	kDebug(YAHOO_GEN_DEBUG) ;
	YahooContact *kc = contact( who );
	if ( kc == NULL ) {
		kDebug(YAHOO_GEN_DEBUG) << "contact " << who << " doesn't exist.";
		return;
	}

	if ( checksum == kc->property( YahooProtocol::protocol()->iconCheckSum ).value().toInt()  &&
	     QFile::exists( KStandardDirs::locateLocal( "appdata", "yahoopictures/"+ who.toLower().replace(QRegExp("[./~]"),"-")  +".png" ) ))
	{
		kDebug(YAHOO_GEN_DEBUG) << "Icon already exists. I will not download it again.";
		return;
	} else
		m_session->downloadPicture( who, url, checksum );
}

void YahooAccount::slotGotBuddyIcon( const QString &who, const QByteArray &data, int checksum )
{
	kDebug(YAHOO_GEN_DEBUG) ;
	YahooContact *kc = contact( who );
	if ( kc == NULL ) {
		kDebug(YAHOO_GEN_DEBUG) << "contact " << who << " doesn't exist.";
		return;
	}
	kc->setDisplayPicture( data, checksum );
}
void YahooAccount::slotGotBuddyIconRequest( const QString & who )
{
	kDebug(YAHOO_GEN_DEBUG) ;
	m_session->sendPictureInformation( who, myself()->property( YahooProtocol::protocol()->iconRemoteUrl ).value().toString(),
	                                   myself()->property( YahooProtocol::protocol()->iconCheckSum ).value().toInt() );
}

void YahooAccount::setBuddyIcon( const KUrl &url )
{
	kDebug(YAHOO_GEN_DEBUG) << "Url: " << url.toLocalFile();
	QString s = url.toLocalFile();
	if ( url.toLocalFile().isEmpty() )
	{
		myself()->removeProperty( Kopete::Global::Properties::self()->photo() );
		myself()->removeProperty( YahooProtocol::protocol()->iconRemoteUrl );
		myself()->removeProperty( YahooProtocol::protocol()->iconExpire );

		if ( m_session )
			m_session->setPictureStatus( Yahoo::NoPicture );
	}
	else
	{
		QImage image( url.toLocalFile() );
		QString newlocation( KStandardDirs::locateLocal( "appdata", "yahoopictures/"+ url.fileName().toLower() ) ) ;
		QFile iconFile( newlocation );
		QByteArray data;
		uint expire = myself()->property( YahooProtocol::protocol()->iconExpire ).value().toInt();

		if ( image.isNull() ) {
			KMessageBox::queuedMessageBox( Kopete::UI::Global::mainWidget(), KMessageBox::Sorry, i18n( "<qt>The selected buddy icon could not be opened. <br />Please set a new buddy icon.</qt>" ), i18n( "Yahoo Plugin" ) );
			return;
		}
		image = image.scaled( 96, 96, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation );
		if(image.width() < image.height())
		{
			image = image.copy((image.width()-image.height())/2, 0, 96, 96);
		}
		else if(image.height() < image.width())
		{
			image = image.copy(0, (image.height()-image.width())/2, 96, 96);
		}

		if( !image.save( newlocation, "PNG" ) || !iconFile.open(QIODevice::ReadOnly) )
		{
			KMessageBox::queuedMessageBox( Kopete::UI::Global::mainWidget(), KMessageBox::Sorry, i18n( "An error occurred when trying to change the display picture." ), i18n( "Yahoo Plugin" ) );
			return;
		}

		data = iconFile.readAll();
		iconFile.close();

		// create checksum - taken from qhash.cpp of qt4
		const uchar *p = reinterpret_cast<const uchar *>(data.data());
		int n = data.size();
		uint checksum = 0;
		uint g;
		while (n--)
		{
			checksum = (checksum << 4) + *p++;
			if ((g = (checksum & 0xf0000000)) != 0)
				checksum ^= g >> 23;
			checksum &= ~g;
		}

		myself()->setProperty( Kopete::Global::Properties::self()->photo() , newlocation );
		configGroup()->writeEntry( "iconLocalUrl", newlocation );

		if ( checksum != static_cast<uint>(myself()->property( YahooProtocol::protocol()->iconCheckSum ).value().toInt()) ||
		     QDateTime::currentDateTime().toTime_t() > expire )
		{
			myself()->setProperty( YahooProtocol::protocol()->iconCheckSum, checksum );
			configGroup()->writeEntry( "iconCheckSum", checksum );
			if ( m_session != 0 )
				m_session->uploadPicture( newlocation );
		}
	}
}

void YahooAccount::slotBuddyIconChanged( const QString &url, int expires )
{
	kDebug(YAHOO_GEN_DEBUG) ;
	int checksum = myself()->property( YahooProtocol::protocol()->iconCheckSum ).value().toInt();

	if( !url.isEmpty() )
	{
		myself()->setProperty( YahooProtocol::protocol()->iconRemoteUrl, url );
		myself()->setProperty( YahooProtocol::protocol()->iconExpire , expires );
		configGroup()->writeEntry( "iconRemoteUrl", url );
		configGroup()->writeEntry( "iconExpire", expires );
		m_session->setPictureStatus( Yahoo::Picture );
		m_session->sendPictureChecksum( QString(), checksum );
	}
}

void YahooAccount::slotWebcamReadyForTransmission()
{
	kDebug(YAHOO_GEN_DEBUG) ;
	if( !m_webcam )
	{
		m_webcam = new YahooWebcam( this );
		QObject::connect( m_webcam, SIGNAL(webcamClosing()), this, SLOT(slotOutgoingWebcamClosing()) );
	}

	m_webcam->startTransmission();
}

void YahooAccount::slotWebcamStopTransmission()
{
	kDebug(YAHOO_GEN_DEBUG) ;

	if( m_webcam )
	{
		m_webcam->stopTransmission();
	}
}

void YahooAccount::slotOutgoingWebcamClosing()
{
	m_session->closeOutgoingWebcam();
	m_webcam->deleteLater();
	m_webcam = 0L;
}

void YahooAccount::slotWebcamViewerJoined( const QString &viewer )
{
	if( m_webcam )
	{
		m_webcam->addViewer( viewer );
	}
}

void YahooAccount::slotWebcamViewerRequest( const QString &viewer )
{
	if( KMessageBox::Yes == KMessageBox::questionYesNo( Kopete::UI::Global::mainWidget(), i18n("%1 wants to view your webcam. Grant access?",
		viewer), QString(), KGuiItem( i18nc("@action","Accept") ), KGuiItem( i18nc("@action","Close") ) ) )
		m_session->grantWebcamAccess( viewer );
}

void YahooAccount::slotWebcamViewerLeft( const QString &viewer )
{
	if( m_webcam )
	{
		m_webcam->removeViewer( viewer );
	}
}

void YahooAccount::slotWebcamClosed( const QString& who, int reason )
{
	YahooContact* kc = contact( who );
	if ( kc == NULL ) {
		kDebug(YAHOO_GEN_DEBUG) << "contact " << who << " doesn't exist.";
		return;
	}
	kc->webcamClosed( reason );
}

void YahooAccount::slotWebcamPaused( const QString &who )
{
	YahooContact* kc = contact( who );
	if ( kc == NULL ) {
		kDebug(YAHOO_GEN_DEBUG) << "contact " << who << " doesn't exist.";
		return;
	}
	kc->webcamPaused();
}

void YahooAccount::setOnlineStatus( const Kopete::OnlineStatus& status, const Kopete::StatusMessage &reason, const OnlineStatusOptions& /*options*/ )
{
	kDebug(YAHOO_GEN_DEBUG) ;
	if ( myself()->onlineStatus().status() == Kopete::OnlineStatus::Offline &&
	     status.status() != Kopete::OnlineStatus::Offline )
	{
		if( !reason.message().isEmpty() )
			m_session->setStatusMessageOnConnect( reason.message() );
		connect( status );
	}
	else if ( myself()->onlineStatus().status() != Kopete::OnlineStatus::Offline &&
	          status.status() == Kopete::OnlineStatus::Offline )
	{
		disconnect();
	}
	else if ( myself()->onlineStatus().status() != Kopete::OnlineStatus::Offline &&
	          status.internalStatus() == 2 && !reason.message().isEmpty())
	{
		slotGoStatus( 99, reason.message() );
	}
	else if ( myself()->onlineStatus().status() != Kopete::OnlineStatus::Offline &&
	          status.internalStatus() == 99 && reason.message().isEmpty())
	{
		slotGoStatus( 2, reason.message() );
	}
	else if ( myself()->onlineStatus().status() != Kopete::OnlineStatus::Offline )
	{
		slotGoStatus( status.internalStatus(), reason.message() );
	}
}

void YahooAccount::setStatusMessage(const Kopete::StatusMessage &statusMessage)
{
	int currentStatus = myself()->onlineStatus().internalStatus();
	m_session->changeStatus( Yahoo::Status( currentStatus ), statusMessage.message(),
	                         (currentStatus == Yahoo::StatusAvailable)? Yahoo::StatusTypeAvailable : Yahoo::StatusTypeAway );
	myself()->setStatusMessage( statusMessage );
}

void YahooAccount::slotOpenInbox()
{
	KToolInvocation::invokeBrowser( QLatin1String( "http://mail.yahoo.com/") );
}

void YahooAccount::slotOpenYAB()
{
	KToolInvocation::invokeBrowser( QLatin1String( "http://address.yahoo.com/") );
}

void YahooAccount::slotEditOwnYABEntry()
{
	myself()->slotUserInfo();
}

void YahooAccount::slotJoinChatRoom()
{
	QPointer <YahooChatSelectorDialog> chatDialog = new YahooChatSelectorDialog( Kopete::UI::Global::mainWidget() );

	QObject::connect( m_session, SIGNAL(gotYahooChatCategories(QDomDocument)), chatDialog,
					SLOT(slotSetChatCategories(QDomDocument)) );
	QObject::connect( m_session, SIGNAL(gotYahooChatRooms(Yahoo::ChatCategory,QDomDocument)),
					chatDialog, SLOT(slotSetChatRooms(Yahoo::ChatCategory,QDomDocument)) );
	QObject::connect( chatDialog, SIGNAL(chatCategorySelected(Yahoo::ChatCategory)),
					this, SLOT(slotChatCategorySelected(Yahoo::ChatCategory)) );
	m_session->getYahooChatCategories();

	if( chatDialog->exec() == QDialog::Accepted && chatDialog )
	{
		kDebug() << chatDialog->selectedRoom().topic << " " << chatDialog->selectedRoom().topic << " " << chatDialog->selectedRoom().id;
		m_session->joinYahooChatRoom( chatDialog->selectedRoom() );
	}

	delete chatDialog;
}

void YahooAccount::slotLeavChat()
{
	m_chatChatSession = 0;
	m_session->leaveChat();
}

void YahooAccount::slotChatCategorySelected( const Yahoo::ChatCategory &category )
{
	m_session->getYahooChatRooms( category );
}


void YahooAccount::slotChatJoined( int /*roomId*/, int /*categoryId*/, const QString &comment, const QString &handle )
{
	Kopete::ContactPtrList others;
	others.append(myself());

	if( !m_chatChatSession )
	{
		m_chatChatSession = new YahooChatChatSession( protocol(), myself(), others );
		QObject::connect( m_chatChatSession, SIGNAL(closing(Kopete::ChatSession*)), this,
					SLOT(slotLeavChat()) );
	}
	m_chatChatSession->removeAllContacts();
	m_chatChatSession->setHandle( handle );
	m_chatChatSession->setTopic( handle );

	m_chatChatSession->view( true )->raise( false );

	Kopete::Message msg( myself(), m_chatChatSession->members() );
	msg.setHtmlBody( i18n("You are now in %1 (%2)", handle, comment) );
	msg.setDirection( Kopete::Message::Internal );
	
	m_chatChatSession->appendMessage( msg );
}

void YahooAccount::slotChatBuddyHasJoined( const QString &nick, const QString &handle, bool suppressNotification )
{
	if(!m_chatChatSession)
		return;

	if( !m_chatChatSession->handle().startsWith( handle ) )
		return;

	YahooContact *c = contact( nick );
	if ( !c )
	{
		kDebug(YAHOO_GEN_DEBUG) << "Adding contact " << nick << " to chat.";
// 		addContact( nick, nick, 0, Kopete::Account::Temporary );
		if( !createChatContact( nick ) )
			return;
		c = contact( nick );
		c->setOnlineStatus( m_protocol->Online );
	}
	m_chatChatSession->joined( c, suppressNotification );
}

void YahooAccount::slotChatBuddyHasLeft( const QString &nick, const QString &handle )
{
	kDebug(YAHOO_GEN_DEBUG) ;

	if(!m_chatChatSession)
		return;

	if( !m_chatChatSession->handle().startsWith( handle ) )
		return;

	YahooContact *c = contact( nick );
	if( !c )
		return;
	m_chatChatSession->left( c );
}

void YahooAccount::slotChatMessageReceived( const QString &nick, const QString &message, const QString &handle )
{
	if(!m_chatChatSession)
		return;

	if( !m_chatChatSession->handle().startsWith( handle ) )
		return;

	QFont msgFont;
	QDateTime msgDT;
	Kopete::ContactPtrList justMe;

	if( !contact( nick ) )
	{
		kDebug(YAHOO_GEN_DEBUG) << "Adding contact " << nick;
		addContact( nick, nick, 0, Kopete::Account::DontChangeKABC );
		if( !createChatContact( nick ) )
			return;
	}
	kDebug(YAHOO_GEN_DEBUG) << "Original message is '" << message << "'";

	QColor fgColor = getMsgColor( message );
	msgDT.setTime_t(time(0L));

	QString newMsgText = prepareIncomingMessage( message );

	kDebug(YAHOO_GEN_DEBUG) << "Message after fixing font tags '" << newMsgText << "'";

	justMe.append(myself());

	Kopete::Message kmsg( contact(nick), justMe );
	kmsg.setTimestamp( msgDT );
	kmsg.setHtmlBody( newMsgText );
	kmsg.setDirection( Kopete::Message::Inbound );

	kmsg.setForegroundColor( fgColor );
	m_chatChatSession->appendMessage(kmsg);
}

void YahooAccount::sendChatMessage( const Kopete::Message &msg, const QString &handle )
{
	m_session->sendYahooChatMessage( YahooContact::prepareMessage( msg.escapedBody() ), handle );
}


#include "yahooaccount.moc"

// vim: set noet ts=4 sts=4 sw=4:
//kate: indent-mode csands; tab-width 4;
