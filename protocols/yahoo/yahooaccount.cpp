/*
    yahooaccount.cpp - Manages a single Yahoo account

    Copyright (c) 2003 by Gav Wood               <gav@kde.org>
    Copyright (c) 2003-2004 by Matt Rogers       <matt.rogers@kdemail.net>
    Based on code by Olivier Goffart             <ogoffart @ kde.org>
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
//Standard Header
#include <ctime>

//QT
#include <qfont.h>
#include <qdatetime.h>
#include <qcolor.h>
#include <qregexp.h>
#include <qimage.h>
#include <qtimer.h>
//Added by qt3to4:
#include <QPixmap>

// KDE
#include <klocale.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kaction.h>
#include <kmenu.h>
#include <kmessagebox.h>
#include <kapplication.h>
#include <krun.h>
#include <kurl.h>
#include <kstandarddirs.h>
#include <kstandarddirs.h>

// Kopete
#include <kopetechatsession.h>
#include <kopetemessage.h>
#include <kopetepassword.h>
#include <kopeteuiglobal.h>
#include <knotification.h>
#include <kopetemetacontact.h>
#include <kopetecontactlist.h>
#include <kopetetransfermanager.h>

// Yahoo
#include "yahooaccount.h"
#include "yahoocontact.h"

YahooAwayDialog::YahooAwayDialog(YahooAccount* account, QWidget *parent, const char *name) :
	KopeteAwayDialog(parent, name)
{
	theAccount = account;
}

void YahooAwayDialog::setAway(int awayType)
{
	awayType = 0;
	theAccount->setAway(awayType, getSelectedAwayMessage());
}


YahooAccount::YahooAccount(YahooProtocol *parent, const QString& accountId, const char *name)
 : Kopete::PasswordedAccount(parent, accountId, 0, name)
{

	// first things first - initialise internals
	theHaveContactList = false;
	stateOnConnection = 0;
	theAwayDialog = new YahooAwayDialog( this );
	m_protocol = parent;
	m_session = 0;
	m_lastDisconnectCode = 0;
	m_currentMailCount = 0;
	m_pictureFlag = 0;
	m_waitingForResponse = false;
	m_keepaliveTimer = new QTimer( this, "keepaliveTimer" );
	
	m_openInboxAction = new KAction( i18n( "Open Inbo&x..." ), "mail_generic", 0, this, SLOT( slotOpenInbox() ), this, "m_openInboxAction" );
	m_openYABAction = new KAction( i18n( "Open &Addressbook..." ), "contents", 0, this, SLOT( slotOpenYAB() ), this, "m_openYABAction" );

	YahooContact* _myself=new YahooContact( this, accountId.toLower(), accountId, Kopete::ContactList::self()->myself() );
	setMyself( _myself );
	_myself->setOnlineStatus( parent->Offline );
	myself()->setProperty( YahooProtocol::protocol()->iconRemoteUrl, configGroup()->readEntry( "iconRemoteUrl", "" ) );
	myself()->setProperty( Kopete::Global::Properties::self()->photo(), configGroup()->readEntry( "iconLocalUrl", "" ) );
	myself()->setProperty( YahooProtocol::protocol()->iconCheckSum, configGroup()->readNumEntry( "iconCheckSum", 0 ) );
	myself()->setProperty( YahooProtocol::protocol()->iconExpire, configGroup()->readNumEntry( "iconExpire", 0 ) );
	
	QObject::connect( Kopete::ContactList::self(), SIGNAL( globalIdentityChanged(const QString&, const QVariant& ) ), SLOT( slotGlobalIdentityChanged(const QString&, const QVariant& ) ));
	QObject::connect( m_keepaliveTimer, SIGNAL( timeout() ), this, SLOT( slotKeepalive() ) );

	QString displayName = configGroup()->readEntry(QString::fromLatin1("displayName"));
	if(!displayName.isEmpty())
		_myself->setNickName(displayName);
}

YahooAccount::~YahooAccount()
{
	delete theAwayDialog;
}

void YahooAccount::setServer( const QString &server )
{
	configGroup()->writeEntry( QString::fromLatin1( "Server" ), server );	
}

void YahooAccount::setPort( int port )
{
	configGroup()->writeEntry( QString::fromLatin1( "Port" ), port );	
}

void YahooAccount::slotGoStatus( int status, const QString &awayMessage)
{
	if( !isConnected() )
	{
		connect( m_protocol->statusFromYahoo( status ) );
		stateOnConnection = status;
	}
	else
	{
		m_session->setAway( yahoo_status( status ), awayMessage, status? 1 : 0 );
		
		//sets the awayMessage property for the owner of the account. shows up in the statusbar icon's tooltip. the property is unset when awayMessage is null
		myself()->setProperty( m_protocol->awayMessage, awayMessage );

		myself()->setOnlineStatus( m_protocol->statusFromYahoo( status ) );
	}
}

YahooSession *YahooAccount::yahooSession()
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
	//kdDebug(14180) << k_funcinfo << "msg is " << msg << endl;
	//Please note that some of the colors are hard-coded to
	//match the yahoo colors
	if ( msg.find("\033[38m") != -1 )
		return Qt::red;
	if ( msg.find("\033[34m") != -1 )
		return Qt::green;
	if ( msg.find("\033[31m") != -1 )
		return Qt::blue;
	if ( msg.find("\033[39m") != -1 )
		return Qt::yellow;
	if ( msg.find("\033[36m") != -1 )
		return Qt::darkMagenta;
	if ( msg.find("\033[32m") != -1 )
		return Qt::cyan;
	if ( msg.find("\033[37m") != -1 )
		return QColor("#FFAA39");
	if ( msg.find("\033[35m") != -1 )
		return QColor("#FFD8D8");
	if ( msg.find("\033[#") != -1 )
	{
		kdDebug(14180) << "Custom color is " << msg.mid(msg.find("\033[#")+2,7) << endl;
		return QColor(msg.mid(msg.find("\033[#")+2,7));
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
		QObject::connect(m_session, SIGNAL(loginResponse(int, const QString &)),
		                 this, SLOT(slotLoginResponse(int, const QString &)) );
		
		QObject::connect(m_session, SIGNAL(gotBuddy(const QString &, const QString &, const QString &)),
		                 this, SLOT(slotGotBuddy(const QString &, const QString &, const QString &)));
		
		QObject::connect(m_session, SIGNAL( buddyListFetched( int ) ),
		                 this, SLOT(slotBuddyListFetched( int ) ) );
		
		QObject::connect(m_session, SIGNAL(statusChanged(const QString&, int, const QString&, int)),
		                 this, SLOT(slotStatusChanged(const QString&, int, const QString&, int)));
		
		QObject::connect(m_session, SIGNAL(gotIm(const QString&, const QString&, long, int)),
		                 this, SLOT(slotGotIm(const QString &, const QString&, long, int)));
		
		QObject::connect(m_session, SIGNAL(gotBuzz(const QString&, long)),
		                 this, SLOT(slotGotBuzz(const QString &, long)));

		QObject::connect(m_session, SIGNAL( gotConfInvite( const QString&, const QString&,
		                                                   const QString&, const QStringList&) ),
		                 this,
		                 SLOT( slotGotConfInvite( const QString&, const QString&,
		                                          const QString&, const QStringList& ) ) );
		
		QObject::connect(m_session, SIGNAL(confUserDecline(const QString&, const QString &, const QString &)),
		                 this,
		                 SLOT(slotConfUserDecline( const QString &, const QString &, const QString &)) );
		
		QObject::connect(m_session , SIGNAL(confUserJoin( const QString &, const QString &)), this,
		                 SLOT(slotConfUserJoin( const QString &, const QString &)) );
		
		QObject::connect(m_session , SIGNAL(confUserLeave( const QString &, const QString &)), this,
		                 SLOT(slotConfUserLeave( const QString &, const QString &)) );
		
		QObject::connect(m_session , SIGNAL(confMessage( const QString &, const QString &, const QString &)), this,
		                 SLOT(slotConfMessage( const QString &, const QString &, const QString &)) );
		
		QObject::connect(m_session,
		                 SIGNAL(gotFile(const QString &, const QString &, long, const QString &, const QString &, unsigned long)),
		                 this,
		                 SLOT(slotGotFile(const QString&, const QString&, long, const QString&, const QString&, unsigned long)));
		
		QObject::connect(m_session , SIGNAL(contactAdded(const QString &, const QString &, const QString &)), this,
		                 SLOT(slotContactAdded(const QString &, const QString &, const QString &)));
		
		QObject::connect(m_session , SIGNAL(rejected(const QString &, const QString &)), this,
		                 SLOT(slotRejected(const QString&, const QString&)));
		
		QObject::connect(m_session, SIGNAL(typingNotify(const QString &, int)), this ,
		                 SLOT(slotTypingNotify(const QString &, int)));
		
		QObject::connect(m_session, SIGNAL(gameNotify(const QString &, int)), this,
		                 SLOT(slotGameNotify( const QString &, int)));
		
		QObject::connect(m_session, SIGNAL(mailNotify(const QString&, const QString&, int)), this,
		                 SLOT(slotMailNotify(const QString &, const QString&, int)));
		
		QObject::connect(m_session, SIGNAL(systemMessage(const QString&)), this,
		                 SLOT(slotSystemMessage(const QString &)));
		
		QObject::connect(m_session, SIGNAL(error(const QString&, int)), this,
		                 SLOT(slotError(const QString &, int )));
		
		QObject::connect(m_session, SIGNAL(gotIdentities(const QStringList &)), this,
		                 SLOT(slotGotIdentities( const QStringList&)));
		
		QObject::connect(m_session, SIGNAL(gotWebcamInvite(const QString&)), this, SLOT(slotGotWebcamInvite(const QString&)));
				
		QObject::connect(m_session, SIGNAL(webcamImageReceived(const QString&, const QPixmap& )), this, SLOT(slotGotWebcamImage(const QString&, const QPixmap& )));
		
		QObject::connect(m_session, SIGNAL(remoteWebcamClosed(const QString&, int )), this, SLOT(slotWebcamClosed(const QString&, int )));

		QObject::connect(m_session, SIGNAL(gotBuddyIcon(const QString&, KTempFile*, int)), this, SLOT(slotGotBuddyIcon(const QString&, KTempFile*, int)) );

		QObject::connect(m_session, SIGNAL(gotBuddyIconInfo(const QString&, KURL, int)), this, SLOT(slotGotBuddyIconInfo(const QString&, KURL, int )));

		QObject::connect(m_session, SIGNAL(gotBuddyIconChecksum(const QString&, int)), this, SLOT(slotGotBuddyIconChecksum(const QString&, int )));
		QObject::connect(m_session, SIGNAL(gotBuddyIconRequest(const QString&)), this, SLOT(slotGotBuddyIconRequest(const QString&)) );

		QObject::connect(m_session, SIGNAL(buddyIconUploaded( const QString &)), this, SLOT(slotBuddyIconChanged(const QString&)));
		
	}

	if ( sct == DeleteConnections )
	{
		QObject::disconnect(m_session, SIGNAL(loginResponse(int, const QString &)),
		                    this, SLOT(slotLoginResponse(int, const QString &)) );
		
		QObject::disconnect(m_session, SIGNAL(gotBuddy(const QString &, const QString &, const QString &)),
		                    this, SLOT(slotGotBuddy(const QString &, const QString &, const QString &)));
		
		QObject::disconnect(m_session, SIGNAL( buddyListFetched( int ) ),
		                    this, SLOT(slotBuddyListFetched( int ) ) );
		
		QObject::disconnect(m_session, SIGNAL(statusChanged(const QString&, int, const QString&, int)),
		                    this, SLOT(slotStatusChanged(const QString&, int, const QString&, int)));
		
		QObject::disconnect(m_session, SIGNAL(gotIm(const QString&, const QString&, long, int)),
		                    this, SLOT(slotGotIm(const QString &, const QString&, long, int)));

		QObject::disconnect(m_session, SIGNAL(gotBuzz(const QString&, long)),
		                    this, SLOT(slotGotBuzz(const QString &, long)));
		
		QObject::disconnect(m_session,
		                    SIGNAL( gotConfInvite( const QString&, const QString&,
		                                           const QString&, const QStringList&) ),
		                    this, 
		                    SLOT( slotGotConfInvite( const QString&, const QString&,
		                                             const QString&, const QStringList&) ) );
		
		QObject::disconnect(m_session,
		                    SIGNAL(confUserDecline(const QString&, const QString &, const QString &)),
		                    this,
		                    SLOT(slotConfUserDecline( const QString &, const QString &, const QString& ) ) );
		
		QObject::disconnect(m_session , SIGNAL(confUserJoin( const QString &, const QString &)),
		                    this, SLOT(slotConfUserJoin( const QString &, const QString &)) );
		
		QObject::disconnect(m_session , SIGNAL(confUserLeave( const QString &, const QString &)),
		                    this, SLOT(slotConfUserLeave( const QString &, const QString &)) );
		
		QObject::disconnect(m_session , SIGNAL(confMessage( const QString &, const QString &, const QString &)), this,
		                    SLOT(slotConfMessage( const QString &, const QString &, const QString &)) );
		
		QObject::disconnect(m_session,
		                    SIGNAL(gotFile(const QString &, const QString &,
		                                   long, const QString &, const QString &, unsigned long)),
		                    this,
		                    SLOT(slotGotFile(const QString&, const QString&,
		                                     long, const QString&, const QString&, unsigned long)));
		
		QObject::disconnect(m_session , SIGNAL(contactAdded(const QString &, const QString &, const QString &)), this,
		                    SLOT(slotContactAdded(const QString &, const QString &, const QString &)));
		
		QObject::disconnect(m_session , SIGNAL(rejected(const QString &, const QString &)), this,
		                    SLOT(slotRejected(const QString&, const QString&)));
		
		QObject::disconnect(m_session, SIGNAL(typingNotify(const QString &, int)), this ,
		                    SLOT(slotTypingNotify(const QString &, int)));
		
		QObject::disconnect(m_session, SIGNAL(gameNotify(const QString &, int)), this,
		                    SLOT(slotGameNotify( const QString &, int)));
		
		QObject::disconnect(m_session, SIGNAL(mailNotify(const QString&, const QString&, int)), this,
		                    SLOT(slotMailNotify(const QString &, const QString&, int)));
		
		QObject::disconnect(m_session, SIGNAL(systemMessage(const QString&)), this,
		                    SLOT(slotSystemMessage(const QString &)));
		
		QObject::disconnect(m_session, SIGNAL(error(const QString&, int)), this,
		                    SLOT(slotError(const QString &, int )));
		
		QObject::disconnect(m_session, SIGNAL(gotIdentities(const QStringList &)), this,
		                    SLOT(slotGotIdentities( const QStringList&)));
		
		QObject::disconnect(m_session, SIGNAL(gotWebcamInvite(const QString&)), this, SLOT(slotGotWebcamInvite(const QString&)));
		
		QObject::disconnect(m_session, SIGNAL(webcamImageReceived(const QString&, const QPixmap& )), this, SLOT(slotGotWebcamImage(const QString&, const QPixmap& )));
		
		QObject::disconnect(m_session, SIGNAL(remoteWebcamClosed(const QString&, int )), this, SLOT(slotWebcamClosed(const QString&, int )));
		
		QObject::disconnect(m_session, SIGNAL(gotBuddyIcon(const QString&, KTempFile*, int )), this, SLOT(slotGotBuddyIcon(const QString&, KTempFile*,int )));

		QObject::disconnect(m_session, SIGNAL(gotBuddyIconInfo(const QString&, KURL, int)), this, SLOT(slotGotBuddyIconInfo(const QString&, KURL, int )));
		
		
		QObject::disconnect(m_session, SIGNAL(gotBuddyIconRequest(const QString&)), this, SLOT(slotGotBuddyIconRequest(const QString&)) );
		
		QObject::disconnect(m_session, SIGNAL(buddyIconUploaded( const QString & )), this, SLOT(slotBuddyIconChanged(const QString&)));

		
		QObject::disconnect(m_session, SIGNAL(gotBuddyIconChecksum(const QString&, int)), this, SLOT(slotGotBuddyIconChecksum(const QString&, int )));
	}
}

void YahooAccount::connectWithPassword( const QString &passwd )
{
	if ( isAway() )
	{
		slotGoOnline();
		return;
	}

	if ( isConnected() || 
	     myself()->onlineStatus() == m_protocol->Connecting )
	{
		kdDebug(14180) << "Yahoo plugin: Ignoring connect request (already connected)." <<endl;
		return;
	
	}

	if ( passwd.isNull() )
	{ //cancel the connection attempt
		static_cast<YahooContact*>( myself() )->setOnlineStatus( m_protocol->Offline );
		return;
	}
	
	QString server = configGroup()->readEntry( "Server", "scs.msg.yahoo.com" );
	int port = configGroup()->readNumEntry( "Port", 5050 );

	YahooSessionManager::manager()->setPager( server, port );
	m_session = YahooSessionManager::manager()->createSession( accountId(), passwd );
	kdDebug(14180) << "Attempting to connect to Yahoo on <" << server << ":" 
		<< port << ">. user <" << accountId() << ">" << endl;
	
	static_cast<YahooContact*>( myself() )->setOnlineStatus( m_protocol->Connecting );
	if ( m_session && m_session->sessionId() > 0 )
	{
		initConnectionSignals( MakeConnections );
		kdDebug(14180) << "Starting the login connection" << endl;
		m_session->login( initialStatus().internalStatus() );
	}
	else
	{
		kdDebug(14180) << "Couldn't connect!" << endl;
			// TODO: message box saying can't connect?
	}
	
}

void YahooAccount::disconnect()
{
//	kdDebug(14180) << k_funcinfo << endl;

	m_currentMailCount = 0;
	if ( isConnected() )
	{
		kdDebug(14180) <<  "Attempting to disconnect from Yahoo server " << endl;

		m_keepaliveTimer->stop();
		m_session->logOff();
		static_cast<YahooContact *>( myself() )->setOnlineStatus( m_protocol->Offline );

		for ( Q3DictIterator<Kopete::Contact> i( contacts() ); i.current(); ++i )
			static_cast<YahooContact *>( i.current() )->setOnlineStatus( m_protocol->Offline );
		
		disconnected( Manual );
	}
	else
	{       //make sure we set everybody else offline explicitly, just for cleanup
		kdDebug(14180) << "Ignoring disconnect request (not fully connected)." << endl;

		for ( Q3DictIterator<Kopete::Contact> i(contacts()); i.current(); ++i )
			static_cast<YahooContact*>( i.current() )->setOnlineStatus( m_protocol->Offline );
	}

	initConnectionSignals( DeleteConnections );
	theHaveContactList = false;
}

void YahooAccount::slotKeepalive()
{
	if( m_waitingForResponse )
	{
		m_waitingForResponse = false;
		slotError( QString::null, 1 );
		return;
	}
	if( isConnected() && m_session )
	{	
		m_session->keepalive();
		m_session->sendIm( accountId(), accountId(), QString("<ping>"), pictureFlag() );
		kdDebug(14180) << "Ping packet sent." << endl;
	}
	m_waitingForResponse = true;
}

void YahooAccount::setAway(bool status, const QString &awayMessage)
{
	kdDebug(14180) << k_funcinfo << endl;

	if( awayMessage.isEmpty() )
		slotGoStatus( status ? 2 : 0 );
	else
		slotGoStatus( status ? 99 : 0, awayMessage );
}

void YahooAccount::slotConnected()
{
	kdDebug(14180) << k_funcinfo << "Moved to slotLoginResponse for the moment" << endl;
}

void YahooAccount::slotGoOnline()
{
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

void YahooAccount::slotBuddyListFetched( int numBuddies )
{
	kdDebug(14180) << "Number of buddies: " << numBuddies << endl;
	theHaveContactList = true;
}

KActionMenu *YahooAccount::actionMenu()
{
//	kdDebug(14180) << k_funcinfo << endl;
	
	KActionMenu *theActionMenu = Kopete::Account::actionMenu();
	
	theActionMenu->popupMenu()->insertSeparator();
	theActionMenu->insert( m_openInboxAction );
	theActionMenu->insert( m_openYABAction );
	
	return theActionMenu;
}

void YahooAccount::slotGotBuddies( const YList */*theList*/ )
{
	kdDebug(14180) << k_funcinfo << endl;
}

YahooContact *YahooAccount::contact( const QString &id )
{
	return static_cast<YahooContact *>(contacts()[id]);
}

bool YahooAccount::createContact(const QString &contactId, Kopete::MetaContact *parentContact )
{
//	kdDebug(14180) << k_funcinfo << " contactId: " << contactId << endl;

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
		kdDebug(14180) << k_funcinfo << "Contact already exists" << endl;

	return false;
}

void YahooAccount::slotGlobalIdentityChanged( const QString &key, const QVariant &value )
{
	if ( key == Kopete::Global::Properties::self()->photo().key() )
	{
		setBuddyIcon( KURL( value.toString() ) );
	}
}

void YahooAccount::setPictureFlag( int flag )
{
	kdDebug(14180) << k_funcinfo << " PictureFlag: " << flag << endl;
	m_pictureFlag = flag;
}

int YahooAccount::pictureFlag()
{
	return m_pictureFlag;
}

/***************************************************************************
 *                                                                         *
 *   Slot for KYahoo signals                                               *
 *                                                                         *
 ***************************************************************************/

void YahooAccount::slotLoginResponse( int succ , const QString &url )
{
	kdDebug(14180) << k_funcinfo << succ << ", " << url << ")]" << endl;
	QString errorMsg;
	if ( succ == YAHOO_LOGIN_OK || (succ == YAHOO_LOGIN_DUPL && m_lastDisconnectCode == 2) )
	{
		slotGotBuddies(yahooSession()->getLegacyBuddyList());

		//Yahoo only supports connecting as invisible and online, nothing else
		if ( initialStatus() == m_protocol->Invisible )
			static_cast<YahooContact *>( myself() )->setOnlineStatus( initialStatus() );
		else
			static_cast<YahooContact *>( myself() )->setOnlineStatus( m_protocol->Online );

		 
		setBuddyIcon( myself()->property( Kopete::Global::Properties::self()->photo() ).value().toString() );
		m_lastDisconnectCode = 0;
		m_keepaliveTimer->start( 60 * 1000 );
		return;
	}
	else if(succ == YAHOO_LOGIN_PASSWD)
	{
		password().setWrong();
		static_cast<YahooContact *>( myself() )->setOnlineStatus( m_protocol->Offline );
		disconnected( BadPassword );
		return;
	}
	else if(succ == YAHOO_LOGIN_LOCK)
	{
		errorMsg = i18n("Could not log into Yahoo service: your account has been locked.\nVisit %1 to reactivate it.").arg(url);
		KMessageBox::queuedMessageBox(Kopete::UI::Global::mainWidget(), KMessageBox::Error, errorMsg);
		static_cast<YahooContact *>( myself() )->setOnlineStatus( m_protocol->Offline );
		disconnected( BadUserName ); // FIXME: add a more appropriate disconnect reason
		return;
	}
	else if ( succ == YAHOO_LOGIN_UNAME )
	{
		errorMsg = i18n("Could not log into the Yahoo service: the username specified was invalid.");
		KMessageBox::queuedMessageBox(Kopete::UI::Global::mainWidget(), KMessageBox::Error, errorMsg);
		static_cast<YahooContact *>( myself() )->setOnlineStatus( m_protocol->Offline );
		disconnected( BadUserName );
	}
	else if ( succ == YAHOO_LOGIN_DUPL && m_lastDisconnectCode != 2 )
	{
		errorMsg = i18n("You have been logged out of the Yahoo service, possibly due to a duplicate login.");
		KMessageBox::queuedMessageBox(Kopete::UI::Global::mainWidget(), KMessageBox::Error, errorMsg);
		static_cast<YahooContact *>( myself() )->setOnlineStatus( m_protocol->Offline );
		disconnected( Manual ); // cannot use ConnectionReset since that will auto-reconnect
		return;
	}

	//If we get here, something went wrong, so set ourselves to offline
	static_cast<YahooContact *>( myself() )->setOnlineStatus( m_protocol->Offline );
	disconnected( Unknown );

}

void YahooAccount::slotGotBuddy( const QString &userid, const QString &alias, const QString &group )
{
	kdDebug(14180) << k_funcinfo << endl;
	IDs[userid] = QPair<QString, QString>(group, alias);

	// Serverside -> local
	if ( !contact( userid ) )
	{
		kdDebug(14180) << "SS Contact " << userid << " is not in the contact list. Adding..." << endl;
		Kopete::Group *g=Kopete::ContactList::self()->findGroup(group);
		addContact(userid, alias.isEmpty() ? userid : alias, g, Kopete::Account::ChangeKABC);
	}
}

void YahooAccount::slotGotIgnore( const QStringList & /* igns */ )
{
	//kdDebug(14180) << k_funcinfo << endl;
}

void YahooAccount::slotGotIdentities( const QStringList & /* ids */ )
{
	//kdDebug(14180) << k_funcinfo << endl;
}

void YahooAccount::slotStatusChanged( const QString &who, int stat, const QString &msg, int  away )
{
	//kdDebug(14180) << k_funcinfo << endl;
	Kopete::Contact *kc = contact( who );
	
	if( contact( who ) == myself() )
		return;
	
	if ( kc )
	{
		Kopete::OnlineStatus newStatus = static_cast<YahooProtocol*>( m_protocol )->statusFromYahoo( stat );
		Kopete::OnlineStatus oldStatus = kc->onlineStatus();

		if( newStatus == static_cast<YahooProtocol*>( m_protocol )->Custom ) {
			if( away == 0 )
				newStatus = static_cast<YahooProtocol*>( m_protocol )->Online;
			kc->setProperty( m_protocol->awayMessage, msg);
		}
		else
			kc->removeProperty( m_protocol->awayMessage );

		if( newStatus != static_cast<YahooProtocol*>( m_protocol )->Offline &&
		    oldStatus == static_cast<YahooProtocol*>( m_protocol )->Offline && contact(who) != myself() )
		{
			m_session->requestBuddyIcon( who );		// Try to get Buddy Icon

			if ( !myself()->property( Kopete::Global::Properties::self()->photo() ).isNull() )
			{
				static_cast< YahooContact* >( contact( who ) )->sendBuddyIconUpdate( pictureFlag() );
				static_cast< YahooContact* >( contact( who ) )->sendBuddyIconChecksum(
					myself()->property( YahooProtocol::protocol()->iconCheckSum ).value().toInt() );
			}
		}
		
		if( newStatus == static_cast<YahooProtocol*>( m_protocol )->Idle ) {
			// TODO: Use the argument 'away' to set the idleTime
		}
		
		kc->setOnlineStatus( newStatus );
	}
}

void YahooAccount::slotGotIm( const QString &who, const QString &msg, long tm, int /*stat*/)
{
	QFont msgFont;
	QDateTime msgDT;
	Kopete::ContactPtrList justMe;
	QRegExp regExp;
	int pos = 0;
	
	// Check for ping-messages
	if( contact( who ) == myself() && msg.startsWith("<ping>") )
	{
		kdDebug(14180) << "Ping packet received." << endl;
		m_waitingForResponse = false;
		return;
	}
	
	if( !contact( who ) )
	{
		kdDebug(14180) << "Adding contact " << who << endl;
		addContact( who,who,  0L, Kopete::Account::Temporary );
	}
	
	//Parse the message for it's properties
	kdDebug(14180) << "Original message is '" << msg << "'" << endl;
	//kdDebug(14180) << "Message color is " << getMsgColor(msg) << endl;
	QColor fgColor = getMsgColor( msg );
	if (tm == 0)
		msgDT.setTime_t(time(0L));
	else
		msgDT.setTime_t(tm, Qt::LocalTime);
	
	QString newMsgText = stripMsgColorCodes( msg );
	
	kdDebug(14180) << "Message after stripping color codes '" << newMsgText << "'" << endl;

	newMsgText.replace( QString::fromLatin1( "&" ), QString::fromLatin1( "&amp;" ) );
	
	// Replace Font tags
	regExp.setMinimal( true );
	regExp.setPattern( "<font([^>]*)size=\"([^>]*)\"([^>]*)>" );
	pos = 0;
	while ( pos >= 0 ) {
		pos = regExp.search( newMsgText, pos );
		if ( pos >= 0 ) {
			pos += regExp.matchedLength();
			newMsgText.replace( regExp, QString::fromLatin1("<font\\1style=\"font-size:\\2pt\">" ) );
		}
	}
	
	// Replace < and > in text
	regExp.setPattern( "<(?![\"/fbui])" );
	pos = 0;
	while ( pos >= 0 ) {
		pos = regExp.search( newMsgText, pos );
		if ( pos >= 0 ) {
			pos += regExp.matchedLength();
			newMsgText.replace( regExp, QString::fromLatin1("&lt;" ) );
		}
	}
	regExp.setPattern( "([^\"bui])>" );
	pos = 0;
	while ( pos >= 0 ) {
		pos = regExp.search( newMsgText, pos );
		if ( pos >= 0 ) {
			pos += regExp.matchedLength();
			newMsgText.replace( regExp, QString::fromLatin1("\\1&gt;" ) );
		}
	}
	
	// add closing tags when needed
	regExp.setMinimal( false );
	regExp.setPattern( "(<b>.*)(?!</b>)" );
	newMsgText.replace( regExp, QString::fromLatin1("\\1</b>" ) );
	regExp.setPattern( "(<i>.*)(?!</i>)" );
	newMsgText.replace( regExp, QString::fromLatin1("\\1</i>" ) );
	regExp.setPattern( "(<u>.*)(?!</u>)" );
	newMsgText.replace( regExp, QString::fromLatin1("\\1</u>" ) );
	regExp.setPattern( "(<font.*)(?!</font>)" );
	newMsgText.replace( regExp, QString::fromLatin1("\\1</font>" ) );
	
	newMsgText.replace( QString::fromLatin1( "\r" ), QString::fromLatin1( "<br/>" ) );
	
	kdDebug(14180) << "Message after fixing font tags '" << newMsgText << "'" << endl;
	
	Kopete::ChatSession *mm = contact(who)->manager(Kopete::Contact::CanCreate);
	
	// Tell the message manager that the buddy is done typing
	mm->receivedTypingMsg(contact(who), false);
	
	justMe.append(myself());
	
	Kopete::Message kmsg(msgDT, contact(who), justMe, newMsgText,
	                     Kopete::Message::Inbound , Kopete::Message::RichText);
	
	kmsg.setFg( fgColor );
	mm->appendMessage(kmsg);
}

void YahooAccount::slotGotBuzz( const QString &who, long tm )
{
	QFont msgFont;
	QDateTime msgDT;
	Kopete::ContactPtrList justMe;
	
	if( !contact( who ) )
	{
		kdDebug(14180) << "Adding contact " << who << endl;
		addContact( who,who,  0L, Kopete::Account::Temporary );
	}
	
	if (tm == 0)
		msgDT.setTime_t(time(0L));
	else
		msgDT.setTime_t(tm, Qt::LocalTime);
	
	justMe.append(myself());
	
	QString buzzMsgText = i18n("This string is shown when the user is buzzed by a contact", "Buzz!!");
	
	Kopete::Message kmsg(msgDT, contact(who), justMe, buzzMsgText,
	                     Kopete::Message::Inbound , Kopete::Message::PlainText);
	QColor fgColor( "gold" );
	kmsg.setFg( fgColor );
	
	Kopete::ChatSession *mm = contact(who)->manager(Kopete::Contact::CanCreate);
	mm->appendMessage(kmsg);
	// Emit the buzz notification.
	mm->emitNudgeNotification();
}

void YahooAccount::slotGotConfInvite( const QString & /* who */, const QString & /* room */, const QString & /* msg */, const QStringList & /* members */ )
{
//	kdDebug(14180) << k_funcinfo << endl;
}

void YahooAccount::slotConfUserDecline( const QString & /* who */, const QString & /* room */, const QString & /* msg */ )
{
//	kdDebug(14180) << k_funcinfo << endl;
}

void YahooAccount::slotConfUserJoin( const QString & /* who */, const QString & /* room */ )
{
//	kdDebug(14180) << k_funcinfo << endl;
}

void YahooAccount::slotConfUserLeave( const QString & /* who */, const QString & /* room */ )
{
//	kdDebug(14180) << k_funcinfo << endl;
}

void YahooAccount::slotConfMessage( const QString & /* who */, const QString & /* room */, const QString & /* msg */ )
{
//	kdDebug(14180) << k_funcinfo << endl;
}

void YahooAccount::slotGotFile( const QString &  who, const QString &  url , long /* expires */, const QString &  msg ,
	const QString &  fname, unsigned long  fesize  )
{
	kdDebug(14180) << k_funcinfo << "Received File from " << who << ": " << msg << endl;
	kdDebug(14180) << k_funcinfo << "Filename :" << fname << " size:" << fesize << endl;
	
	Kopete::TransferManager::transferManager()->askIncomingTransfer( contact( who ) , fname, fesize, msg, url );	
	QObject::connect( Kopete::TransferManager::transferManager(), SIGNAL( accepted( Kopete::Transfer *, const QString& ) ),
					this, SLOT( slotReceiveFileAccepted( Kopete::Transfer *, const QString& ) ) );
}

void YahooAccount::slotReceiveFileAccepted(Kopete::Transfer *trans, const QString& /*fileName*/)
{	
	m_session->getUrlHandle( trans );
	QObject::disconnect( Kopete::TransferManager::transferManager(), SIGNAL( accepted( Kopete::Transfer *, const QString& ) ),
					this, SLOT( slotReceiveFileAccepted( Kopete::Transfer *, const QString& ) ) );
}

void YahooAccount::slotContactAdded( const QString & /* myid */, const QString & /* who */, const QString & /* msg */ )
{
//	kdDebug(14180) << k_funcinfo << myid << " " << who << " " << msg << endl;
}

void YahooAccount::slotRejected( const QString & /* who */, const QString & /* msg */ )
{
//	kdDebug(14180) << k_funcinfo << endl;
}

void YahooAccount::slotTypingNotify( const QString &who, int what )
{
	emit receivedTypingMsg(who, what);
}

void YahooAccount::slotGameNotify( const QString & /* who */, int /* stat */ )
{
//	kdDebug(14180) << k_funcinfo << endl;
}

void YahooAccount::slotMailNotify( const QString& from, const QString& /* subject */, int cnt )
{
//	kdDebug(14180) << k_funcinfo << "Mail count: " << cnt << endl;

	if ( cnt > m_currentMailCount && from.isEmpty() )
	{
		QObject::connect(KNotification::event( "yahoo_mail", i18n( "You have one unread message in your Yahoo inbox.",
			"You have %n unread messages in your Yahoo inbox.", cnt ), 0 , 0 , i18n( "Open Inbox..." ) ),
		                 SIGNAL(activated(unsigned int ) ) , this, SLOT( slotOpenInbox() ) );
		m_currentMailCount = cnt;
	}
	else if ( cnt > m_currentMailCount )
	{	kdDebug(14180) << k_funcinfo << "attempting to trigger event" << endl;
		QObject::connect(KNotification::event( "yahoo_mail", i18n( "You have a message from %1 in your Yahoo inbox.").arg(from) 
			, 0 , 0 , i18n( "Open Inbox..." ) ), SIGNAL(activated(unsigned int ) ) , this, SLOT( slotOpenInbox() ) );
		m_currentMailCount = cnt;
	}
}

void YahooAccount::slotSystemMessage( const QString & /* msg */ )
{
//	kdDebug(14180) << k_funcinfo << msg << endl;
}

void YahooAccount::slotError( const QString &err, int fatal )
{
	kdDebug(14180) << k_funcinfo << fatal << ": " << err << endl;
	m_lastDisconnectCode = fatal;
	m_keepaliveTimer->stop();
	if(isConnected()) { // If we are already disconnected we don't need this MessageBox (<heiko@rangun.de>)
		KMessageBox::queuedMessageBox( Kopete::UI::Global::mainWidget(), KMessageBox::Error, i18n( "<qt>The connection with the Yahoo server was lost.</qt>" ), 
							i18n( "Connection Lost - Yahoo Plugin" ) );
	
		if ( fatal == 1 || fatal == 2 || fatal == -1 )
			disconnect();
	}
}

void YahooAccount::slotRemoveHandler( int /* fd */ )
{
//	kdDebug(14180) << k_funcinfo << endl;
}

void YahooAccount::slotGotWebcamInvite( const QString& who )
{
	YahooContact* kc = contact( who );
	if ( kc == NULL ) {
		kdDebug(14180) << k_funcinfo << "contact " << who << " doesn't exist." << endl;
		return;
	}
	kc->gotWebcamInvite();
}

void YahooAccount::slotGotWebcamImage( const QString& who, const QPixmap& image )
{
	YahooContact* kc = contact( who );
	if ( kc == NULL ) {
		kdDebug(14180) << k_funcinfo << "contact " << who << " doesn't exist." << endl;
		return;
	}
	kc->receivedWebcamImage( image );
}

void YahooAccount::slotGotBuddyIconChecksum(const QString &who, int checksum)
{
	YahooContact *kc = contact( who );
	if ( kc == NULL ) {
		kdDebug(14180) << k_funcinfo << "contact " << who << " doesn't exist." << endl;
		return;
	}

	if ( checksum == kc->property( YahooProtocol::protocol()->iconCheckSum ).value().toInt() &&
	     QFile::exists( locateLocal( "appdata", "yahoopictures/"+ who.toLower().replace(QRegExp("[./~]"),"-")  +".png" ) ) )
	{
		kdDebug(14180) << k_funcinfo << "Icon already exists. I will not request it again." << endl;
		return;
	} else
		m_session->requestBuddyIcon( who );
}

void YahooAccount::slotGotBuddyIconInfo(const QString &who, KURL url, int checksum)
{
	kdDebug(14180) << k_funcinfo << endl;
	YahooContact *kc = contact( who );
	if ( kc == NULL ) {
		kdDebug(14180) << k_funcinfo << "contact " << who << " doesn't exist." << endl;
		return;
	}

	if ( checksum == kc->property( YahooProtocol::protocol()->iconCheckSum ).value().toInt()  &&
	     QFile::exists( locateLocal( "appdata", "yahoopictures/"+ who.toLower().replace(QRegExp("[./~]"),"-")  +".png" ) ))
	{
		kdDebug(14180) << k_funcinfo << "Icon already exists. I will not download it again." << endl;
		return;
	} else
		m_session->downloadBuddyIcon( who, url, checksum );
}

void YahooAccount::slotGotBuddyIcon( const QString &who, KTempFile *file, int checksum )
{
	kdDebug(14180) << k_funcinfo << endl;
	YahooContact *kc = contact( who );
	if ( kc == NULL ) {
		kdDebug(14180) << k_funcinfo << "contact " << who << " doesn't exist." << endl;
		return;
	}
	kc->setDisplayPicture( file, checksum );	
}
void YahooAccount::slotGotBuddyIconRequest( const QString & who )
{
	kdDebug(14180) << k_funcinfo << endl;
	YahooContact *kc = contact( who );
	if ( kc == NULL ) {
		kdDebug(14180) << k_funcinfo << "contact " << who << " doesn't exist." << endl;
		return;
	}
	kc->sendBuddyIconInfo( myself()->property( YahooProtocol::protocol()->iconRemoteUrl ).value().toString(),
							myself()->property( YahooProtocol::protocol()->iconCheckSum ).value().toInt() );
}

void YahooAccount::setBuddyIcon( KURL url )
{
	
	kdDebug(14180) << k_funcinfo << "Url: " << url.path() << endl;
	QString s = url.path();
	if ( url.path().isEmpty() )
	{
		myself()->removeProperty( Kopete::Global::Properties::self()->photo() );
		myself()->removeProperty( YahooProtocol::protocol()->iconRemoteUrl );
		myself()->removeProperty( YahooProtocol::protocol()->iconExpire );
		setPictureFlag( 0 );
		
		slotBuddyIconChanged( QString::null );
	}
	else
	{
		QImage image( url.path() );
		QString newlocation( locateLocal( "appdata", "yahoopictures/"+ url.fileName().toLower() ) ) ;
		QFile iconFile( newlocation );
		QByteArray data;
		uint expire = myself()->property( YahooProtocol::protocol()->iconExpire ).value().toInt();
		
		if ( image.isNull() ) {
			KMessageBox::sorry( Kopete::UI::Global::mainWidget(), i18n( "<qt>The selected buddy icon could not be opened. <br>Please set a new buddy icon.</qt>" ), i18n( "Yahoo Plugin" ) );
			return;
		}
		image = image.smoothScale( 96, 96, QImage::ScaleMax );
		if(image.width() > image.height()) {
			image = image.copy((image.width()-image.height())/2, 0, image.height(), image.height());
		}
		else if(image.height() > image.width()) {
			image = image.copy(0, (image.height()-image.width())/2, image.width(), image.width());
		}

		if( !image.save( newlocation, "PNG" ) || !iconFile.open(QIODevice::ReadOnly) )
		{
			KMessageBox::sorry( Kopete::UI::Global::mainWidget(), i18n( "An error occurred when trying to change the display picture." ), i18n( "Yahoo Plugin" ) );
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
		
		setPictureFlag( 2 );
		
		if ( checksum != static_cast<uint>(myself()->property( YahooProtocol::protocol()->iconCheckSum ).value().toInt()) ||
		     QDateTime::currentDateTime().toTime_t() > expire )
		{
			myself()->setProperty( YahooProtocol::protocol()->iconCheckSum, checksum );
			myself()->setProperty( YahooProtocol::protocol()->iconExpire , QDateTime::currentDateTime().toTime_t() + 604800 );
			configGroup()->writeEntry( "iconCheckSum", checksum );
			configGroup()->writeEntry( "iconExpire", myself()->property( YahooProtocol::protocol()->iconExpire ).value().toInt() );
			if ( m_session != 0 )
				m_session->uploadBuddyIcon( newlocation, data.size() );
		}
	}
}

void YahooAccount::slotBuddyIconChanged( const QString &url )
{
	kdDebug(14180) << k_funcinfo << endl;
	Q3DictIterator<Kopete::Contact> it( contacts() );
	int checksum = myself()->property( YahooProtocol::protocol()->iconCheckSum ).value().toInt();

	if ( url.isEmpty() )	// remove pictures from buddie's clients
	{
		checksum = 0;	
		setPictureFlag( 0 );
	}
	else
	{
		myself()->setProperty( YahooProtocol::protocol()->iconRemoteUrl, url );
		configGroup()->writeEntry( "iconRemoteUrl", url );
		setPictureFlag( 2 );
	}
	
	for ( ; it.current(); ++it )
	{
		if ( it.current() == myself() || !it.current()->isReachable() )
			continue;
		static_cast< YahooContact* >( it.current() )->sendBuddyIconChecksum( checksum );
		static_cast< YahooContact* >( it.current() )->sendBuddyIconUpdate( pictureFlag() );
	}
}

void YahooAccount::slotWebcamClosed( const QString& who, int reason )
{
	YahooContact* kc = contact( who );
	if ( kc == NULL ) {
		kdDebug(14180) << k_funcinfo << "contact " << who << " doesn't exist." << endl;
		return;
	}
	kc->webcamClosed( reason );
}

void YahooAccount::setOnlineStatus( const Kopete::OnlineStatus& status , const QString &reason)
{
	if ( myself()->onlineStatus().status() == Kopete::OnlineStatus::Offline &&
	     status.status() == Kopete::OnlineStatus::Online )
	{
		connect( status );
	}
	else if ( myself()->onlineStatus().status() != Kopete::OnlineStatus::Offline &&
	          status.status() == Kopete::OnlineStatus::Offline )
	{
		disconnect();
	}
	else if ( myself()->onlineStatus().status() != Kopete::OnlineStatus::Offline &&
	          status.internalStatus() == 99 && reason.isEmpty())
	{
		// Get custom away message from User
		theAwayDialog->show( 99 );
	}
	else if ( myself()->onlineStatus().status() != Kopete::OnlineStatus::Offline )
	{
		slotGoStatus( status.internalStatus(), reason );
	}
}

void YahooAccount::slotOpenInbox()
{
	KRun::runURL( KURL( QString::fromLatin1("http://mail.yahoo.com/") ) , "text/html" );
}

void YahooAccount::slotOpenYAB()
{
	KRun::runURL( KURL( QString::fromLatin1("http://address.yahoo.com/") ) , "text/html" );
}

#include "yahooaccount.moc"

// vim: set noet ts=4 sts=4 sw=4:
//kate: indent-mode csands; tab-width 4;
