
/***************************************************************************
                   jabberaccount.cpp  -  core Jabber account class
                             -------------------
    begin                : Sat M??? 8 2003
    copyright            : (C) 2003 by Till Gerken <till@tantalo.net>
							Based on JabberProtocol by Daniel Stone <dstone@kde.org>
							and Till Gerken <till@tantalo.net>.
	copyright            : (C) 2006 by Olivier Goffart <ogoffart at kde.org>
	Copyright 2006 by Tommi Rantala <tommi.rantala@cs.helsinki.fi>

			   Kopete (C) 2001-2006 Kopete developers
			   <kopete-devel@kde.org>.
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "jabberaccount.h"
#include "im.h"
#include "filetransfer.h"
#include "xmpp.h"
#include "xmpp_tasks.h"
#include "qca.h"
#include "bsocket.h"

#include "jabberbookmarks.h"

#include <time.h>

#include <qstring.h>
#include <qregexp.h>
#include <qtimer.h>
#include <QAbstractSocket>
#include <QPointer>

#include <kcomponentdata.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <knotification.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <kpassworddialog.h>
#include <kinputdialog.h>
#include <kicon.h>
#include <kactionmenu.h>
#include <kglobal.h>
#include <KComponentData>

#include "kopetepassword.h"
#include "kopetemetacontact.h"
#include "kopeteuiglobal.h"
#include "kopetegroup.h"
#include "kopetecontactlist.h"
#include "kopeteaccountmanager.h"
#include "kopeteaddedinfoevent.h"
#include "kopetestatusmanager.h"

#include "jabberclient.h"
#include "jabberprotocol.h"
#include "jabberresourcepool.h"
#include "jabbercontactpool.h"
#include "jabberfiletransfer.h"
#include "jabbercontact.h"
#include "jabbergroupcontact.h"
#include "jabbercapabilitiesmanager.h"
#include "jabbertransport.h"
#include "jabberresource.h"
#include "dlgxmppconsole.h"
#include "dlgjabberservices.h"
#include "dlgjabberchatjoin.h"
#include "jt_pubsub.h"

#include <sys/utsname.h>

#ifdef LIBJINGLE_SUPPORT
#include "libjingle.h"
#endif

#ifdef JINGLE_SUPPORT
#include "jinglecallsmanager.h"
#endif

#define KOPETE_CAPS_NODE "http://kopete.kde.org/jabber/caps"



JabberAccount::JabberAccount (JabberProtocol * parent, const QString & accountId)
			  :Kopete::PasswordedAccount ( parent, accountId, false )
{
	kDebug(JABBER_DEBUG_GLOBAL) << "Instantiating new account " << accountId;

	m_protocol = parent;

	m_jabberClient = new JabberClient;
	
	m_resourcePool = 0L;
	m_contactPool = 0L;

#ifdef JINGLE_SUPPORT
	m_jcm = 0L;
#endif
	m_bookmarks = new JabberBookmarks(this);
	m_removing=false;
	m_notifiedUserCannotBindTransferPort = false;
	// add our own contact to the pool
	JabberContact *myContact = contactPool()->addContact ( XMPP::RosterItem ( accountId ), Kopete::ContactList::self()->myself(), false );
	setMyself( myContact );

	m_initialPresence = XMPP::Status ( "", "", 5, true );

	// instantiate new client backend
	QObject::connect ( m_jabberClient, SIGNAL (csDisconnected()), this, SLOT (slotCSDisconnected()) );
	QObject::connect ( m_jabberClient, SIGNAL (csError(int)), this, SLOT (slotCSError(int)) );
	QObject::connect ( m_jabberClient, SIGNAL (tlsWarning(QCA::TLS::IdentityResult,QCA::Validity)), this, SLOT (slotHandleTLSWarning(QCA::TLS::IdentityResult,QCA::Validity)) );
	QObject::connect ( m_jabberClient, SIGNAL (connected()), this, SLOT (slotConnected()) );
	QObject::connect ( m_jabberClient, SIGNAL (error(JabberClient::ErrorCode)), this, SLOT (slotClientError(JabberClient::ErrorCode)) );
	
	QObject::connect ( m_jabberClient, SIGNAL (subscription(XMPP::Jid,QString)),
	                   this, SLOT (slotSubscription(XMPP::Jid,QString)) );
	QObject::connect ( m_jabberClient, SIGNAL (rosterRequestFinished(bool)),
	                   this, SLOT (slotRosterRequestFinished(bool)) );
	QObject::connect ( m_jabberClient, SIGNAL (newContact(XMPP::RosterItem)),
	                   this, SLOT (slotContactUpdated(XMPP::RosterItem)) );
	QObject::connect ( m_jabberClient, SIGNAL (contactUpdated(XMPP::RosterItem)),
	                   this, SLOT (slotContactUpdated(XMPP::RosterItem)) );
	QObject::connect ( m_jabberClient, SIGNAL (contactDeleted(XMPP::RosterItem)),
	                   this, SLOT (slotContactDeleted(XMPP::RosterItem)) );
	QObject::connect ( m_jabberClient, SIGNAL (resourceAvailable(XMPP::Jid,XMPP::Resource)),
	                   this, SLOT (slotResourceAvailable(XMPP::Jid,XMPP::Resource)) );
	QObject::connect ( m_jabberClient, SIGNAL (resourceUnavailable(XMPP::Jid,XMPP::Resource)),
	                   this, SLOT (slotResourceUnavailable(XMPP::Jid,XMPP::Resource)) );
	QObject::connect ( m_jabberClient, SIGNAL (messageReceived(XMPP::Message)),
	                   this, SLOT (slotReceivedMessage(XMPP::Message)) );
	QObject::connect ( m_jabberClient, SIGNAL (incomingFileTransfer()),
	                   this, SLOT (slotIncomingFileTransfer()) );
	QObject::connect ( m_jabberClient, SIGNAL (groupChatJoined(XMPP::Jid)),
	                   this, SLOT (slotGroupChatJoined(XMPP::Jid)) );
	QObject::connect ( m_jabberClient, SIGNAL (groupChatLeft(XMPP::Jid)),
	                   this, SLOT (slotGroupChatLeft(XMPP::Jid)) );
	QObject::connect ( m_jabberClient, SIGNAL (groupChatPresence(XMPP::Jid,XMPP::Status)),
	                   this, SLOT (slotGroupChatPresence(XMPP::Jid,XMPP::Status)) );
	QObject::connect ( m_jabberClient, SIGNAL (groupChatError(XMPP::Jid,int,QString)),
	                   this, SLOT (slotGroupChatError(XMPP::Jid,int,QString)) );
	QObject::connect ( m_jabberClient, SIGNAL (debugMessage(QString)),
	                   this, SLOT (slotClientDebugMessage(QString)) );

#ifdef LIBJINGLE_SUPPORT
	m_libjingle = new Libjingle;
#endif

}

JabberAccount::~JabberAccount ()
{
	disconnect ( Kopete::Account::Manual );

	// Remove this account from Capabilities manager.
	if ( protocol() && protocol()->capabilitiesManager() )
		protocol()->capabilitiesManager()->removeAccount( this );

	cleanup ();
	
	QMap<QString,JabberTransport*> tranposrts_copy=m_transports;
	QMap<QString,JabberTransport*>::Iterator it;
	for ( it = tranposrts_copy.begin(); it != tranposrts_copy.end(); ++it ) 
		delete it.value();
}

void JabberAccount::cleanup ()
{

	delete m_jabberClient;
	
	m_jabberClient = 0L;

	delete m_resourcePool;
	m_resourcePool = 0L;

	delete m_contactPool;
	m_contactPool = 0L;

#ifdef LIBJINGLE_SUPPORT
	delete m_libjingle;
	m_libjingle = 0L;
#endif

#ifdef JINGLE_SUPPORT
	delete m_jcm;
	m_jcm = 0L;
#endif
}

void JabberAccount::setS5BServerPort ( int port )
{
	if ( !m_jabberClient->setS5BServerPort ( port ) && !m_notifiedUserCannotBindTransferPort)
	{
		KMessageBox::queuedMessageBox ( Kopete::UI::Global::mainWidget (), KMessageBox::Sorry,
							 i18n ( "Could not bind the Jabber file transfer manager to a local port. Please check if the file transfer port is already in use, or choose another port in the account settings." ),
							 i18n ( "Failed to start Jabber File Transfer Manager" ) );
		m_notifiedUserCannotBindTransferPort = true;
	}

}

void JabberAccount::fillActionMenu( KActionMenu *actionMenu )
{
	Kopete::Account::fillActionMenu( actionMenu );

	actionMenu->addSeparator();

	KAction *action;
	
	action = new KAction( this );
	action->setIcon( KIcon("jabber_group") );
	action->setText( i18n("Join Groupchat...") );
	QObject::connect( action, SIGNAL(triggered(bool)), this, SLOT(slotJoinNewChat()) );
	actionMenu->addAction(action);
	action->setEnabled( isConnected() );
	
	action = m_bookmarks->bookmarksAction( m_bookmarks );
	actionMenu->addAction(action);
	action->setEnabled( isConnected() );


	actionMenu->addSeparator();
	
	action = new KAction( this );
	action->setIcon( KIcon("jabber_serv_on") );
	action->setText( i18n ("Services...") );
	QObject::connect( action, SIGNAL(triggered(bool)), this, SLOT(slotGetServices()) );
	action->setEnabled( isConnected() );
	actionMenu->addAction( action );

	action = new KAction( this );
	action->setIcon( ( KIcon("mail-message-new") ) );
	action->setText( i18n ("XML Console") );
	QObject::connect( action, SIGNAL(triggered(bool)), this, SLOT(slotXMPPConsole()) );
	actionMenu->addAction( action );

	action = new KAction( this );
	action->setIcon( ( KIcon("document-properties") ) );
	action->setText( i18n ("Edit User Info...") );
	QObject::connect( action, SIGNAL(triggered(bool)), this, SLOT(slotEditVCard()) );
	action->setEnabled( isConnected() );
	actionMenu->addAction( action );

	KActionMenu *mMoodMenu = new KActionMenu(i18n("Set Mood"), actionMenu);
	for(int i = 0; i <= Mood::Worried; i++)
	{
		action = new KAction(mMoodMenu);
		action->setText(MoodManager::self()->getMoodName((Mood::Type)i));
		action->setData(QVariant(i));
		QObject::connect( action, SIGNAL(triggered(bool)), this, SLOT(slotSetMood()) );
		mMoodMenu->addAction( action );
	}
	actionMenu->addAction( mMoodMenu );
}

JabberResourcePool *JabberAccount::resourcePool ()
{

	if ( !m_resourcePool )
		m_resourcePool = new JabberResourcePool ( this );

	return m_resourcePool;

}

JabberContactPool *JabberAccount::contactPool ()
{

	if ( !m_contactPool )
		m_contactPool = new JabberContactPool ( this );

	return m_contactPool;

}

bool JabberAccount::createContact (const QString & contactId,  Kopete::MetaContact * metaContact)
{

	// collect all group names
	QStringList groupNames;
	Kopete::GroupList groupList = metaContact->groups();
	foreach( Kopete::Group *group, groupList )
	{
		if (group->type() == Kopete::Group::Normal)
			groupNames += group->displayName();
		else if (group->type() == Kopete::Group::TopLevel)
			groupNames += QString();
	}

	if(groupNames.size() == 1 && groupNames.at(0).isEmpty())
		groupNames.clear();

	XMPP::Jid jid ( contactId );
	XMPP::RosterItem item ( jid );
	item.setName ( metaContact->displayName () );
	item.setGroups ( groupNames );

	// this contact will be created with the "dirty" flag set
	// (it will get reset if the contact appears in the roster during connect)
	JabberContact *contact = contactPool()->addContact ( item, metaContact, true );

	return ( contact != 0 );

}

void JabberAccount::errorConnectFirst ()
{

	KMessageBox::queuedMessageBox ( Kopete::UI::Global::mainWidget (),
									KMessageBox::Error,
									i18n ("Please connect first."), i18n ("Jabber Error") );

}

void JabberAccount::errorConnectionLost ()
{
	disconnected( Kopete::Account::ConnectionReset );
}

bool JabberAccount::isConnecting ()
{

	XMPP::Jid jid ( myself()->contactId () );

	// see if we are currently trying to connect
	return resourcePool()->bestResource ( jid ).status().show () == QString("connecting");

}

void JabberAccount::connectWithPassword ( const QString &password )
{
	kDebug (JABBER_DEBUG_GLOBAL) << "called";

	/* Cancel connection process if no password has been supplied. */
	if ( password.isEmpty () )
	{
		disconnect ( Kopete::Account::Manual );
		return;
	}

	/* Don't do anything if we are already connected. */
	if ( isConnected () )
		return;

	// clean up old client backend
	m_jabberClient->disconnect ();

	bool customServer = configGroup()->readEntry("CustomServer", false);

	if ( customServer )
	{
		m_jabberClient->setUseXMPP09 ( true );
		// override server and port (this should be dropped when using the new protocol and no direct SSL)
		m_jabberClient->setOverrideHost ( true, server (), port () );
	}
	else
	{
		m_jabberClient->setUseXMPP09 ( false );
		m_jabberClient->setOverrideHost ( false );
	}

	// set SSL flag (this should be converted to forceTLS when using the new protocol)
	m_jabberClient->setUseSSL ( configGroup()->readEntry ( "UseSSL", false ) );

	// allow plaintext password authentication or not?
	m_jabberClient->setAllowPlainTextPassword ( configGroup()->readEntry ( "AllowPlainTextPassword", false ) );

	// enable file transfer (if empty, IP will be set after connection has been established)
	KConfigGroup config = KGlobal::config()->group ( "Jabber" );
	m_jabberClient->setFileTransfersEnabled ( true, config.readEntry ( "LocalIP" ) );
	setS5BServerPort ( config.readEntry ( "LocalPort", 8010 ) );

	//
	// Determine system name
	//
	if ( !configGroup()->readEntry ( "HideSystemInfo", false ) )
	{
		struct utsname utsBuf;

		uname (&utsBuf);

		m_jabberClient->setClientName ("Kopete");
		m_jabberClient->setClientVersion (KGlobal::mainComponent().aboutData()->version ());
		m_jabberClient->setOSName (QString ("%1 %2").arg (utsBuf.sysname, 1).arg (utsBuf.release, 2));
	}

	// Set caps node information
	m_jabberClient->setCapsNode(KOPETE_CAPS_NODE);
	m_jabberClient->setCapsVersion(KGlobal::mainComponent().aboutData()->version());
	
	// Set Disco Identity information
	DiscoItem::Identity identity;
	identity.category = "client";
	identity.type = "pc";
	identity.name = "Kopete";
	m_jabberClient->setDiscoIdentity(identity);

	//BEGIN TIMEZONE INFORMATION
	//
	// Set timezone information (code from Psi)
	// Copyright (C) 2001-2003  Justin Karneges <justin@affinix.com>
	//
	time_t x;
	time(&x);
	char str[256];
	char fmt[32];
	int timezoneOffset(0);
	QString timezoneString;
	
	strcpy ( fmt, "%z" );
	strftime ( str, 256, fmt, localtime ( &x ) );
	
	if ( strcmp ( fmt, str ) )
	{
		QString s = str;
		if ( s.at ( 0 ) == '+' )
			s.remove ( 0, 1 );
		s.truncate ( s.length () - 2 );
		timezoneOffset = s.toInt();
	}

	strcpy ( fmt, "%Z" );
	strftime ( str, 256, fmt, localtime ( &x ) );

	if ( strcmp ( fmt, str ) )
		timezoneString = str;
	//END of timezone code

	kDebug (JABBER_DEBUG_GLOBAL) << "Determined timezone " << timezoneString << " with UTC offset " << timezoneOffset << " hours.";

	m_jabberClient->setTimeZone ( timezoneString, timezoneOffset );

	kDebug (JABBER_DEBUG_GLOBAL) << "Connecting to Jabber server " << server() << ":" << port();

	setPresence( XMPP::Status ("connecting", "", 0, true) );

	switch ( m_jabberClient->connect ( XMPP::Jid ( accountId () + QString("/") + resource () ), password ) )
	{
		case JabberClient::NoTLS:
			// no SSL support, at the connecting stage this means the problem is client-side
			KMessageBox::queuedMessageBox(Kopete::UI::Global::mainWidget (), KMessageBox::Error,
								i18n ("SSL support could not be initialized for account %1. This is most likely because TLS support for QCA is not available.",
								myself()->contactId()),
								i18n ("Jabber SSL Error"));
			break;
	
		case JabberClient::Ok:
		default:
			// everything alright!

			break;
	}

#ifdef LIBJINGLE_SUPPORT
	m_libjingle->setUser(myself()->contactId (), password);
#endif

}

void JabberAccount::slotClientDebugMessage ( const QString &msg )
{

	kDebug (JABBER_DEBUG_PROTOCOL) << msg;

}

bool JabberAccount::handleTLSWarning (
		JabberClient *jabberClient,
		QCA::TLS::IdentityResult identityResult,
		QCA::Validity validityResult )
{
	QString validityString, code, idString, idCode;

	QString server    = jabberClient->jid().domain ();
	QString accountId = jabberClient->jid().bare ();

	switch ( identityResult )
	{
		case QCA::TLS::Valid:
			break;
		case QCA::TLS::HostMismatch:
			idString = i18n("The host name does not match the one in the certificate.");
			idCode   = "HostMismatch";
			break;
		case QCA::TLS::InvalidCertificate:
			idString = i18n("The certificate is invalid.");
			idCode   = "InvalidCert";
			break;
		case QCA::TLS::NoCertificate:
			idString = i18n("No certificate was presented.");
			idCode   = "NoCert";
			break;
	}

	switch ( validityResult )
	{
		case QCA::ValidityGood:
			break;
		case QCA::ErrorRejected:
			validityString = i18n("The Certificate Authority rejected the certificate.");
			code = "Rejected";
			break;
		case QCA::ErrorUntrusted:
			validityString = i18n("The certificate is not trusted.");
			code = "Untrusted";
			break;
		case QCA::ErrorSignatureFailed:
			validityString = i18n("The signature is invalid.");
			code = "SignatureFailed";
			break;
		case QCA::ErrorInvalidCA:
			validityString = i18n("The Certificate Authority is invalid.");
			code = "InvalidCA";
			break;
		case QCA::ErrorInvalidPurpose:
			validityString = i18n("Invalid certificate purpose.");
			code = "InvalidPurpose";
			break;
		case QCA::ErrorSelfSigned:
			validityString = i18n("The certificate is self-signed.");
			code = "SelfSigned";
			break;
		case QCA::ErrorRevoked:
			validityString = i18n("The certificate has been revoked.");
			code = "Revoked";
			break;
		case QCA::ErrorPathLengthExceeded:
			validityString = i18n("Maximum certificate chain length was exceeded.");
			code = "PathLengthExceeded";
			break;
		case QCA::ErrorExpired:
			validityString = i18n("The certificate has expired.");
			code = "Expired";
			break;
		case QCA::ErrorExpiredCA:
			validityString = i18n("The Certificate Authority has expired.");
			code = "ExpiredCA";
			break;
		case QCA::ErrorValidityUnknown:
			validityString = i18n("Validity is unknown.");
			code = "ValidityUnknown";
			break;
	}

	QString message;
   
	if (!idString.isEmpty())
	{
		if (!validityString.isEmpty())
		{
			message = i18n("<qt><p>The identity and the certificate of server %1 could not be "
					"validated for account %2:</p><p>%3</p><p>%4</p><p>Do you want to continue?</p></qt>",
					server, accountId, idString, validityString);
		}
		else
		{
			message = i18n("<qt><p>The certificate of server %1 could not be validated for "
					"account %2: %3</p><p>Do you want to continue?</p></qt>",
					server, accountId, idString);
		}
	} else {
		message = i18n("<qt><p>The certificate of server %1 could not be validated for "
			"account %2: %3</p><p>Do you want to continue?</p></qt>",
			server, accountId, validityString);
	}

	return ( KMessageBox::warningContinueCancel ( Kopete::UI::Global::mainWidget (),
					  message,
					  i18n("Jabber Connection Certificate Problem"),
					  KStandardGuiItem::cont(),KStandardGuiItem::cancel(),
					  QString("KopeteTLSWarning") + server + idCode + code) == KMessageBox::Continue );

}

void JabberAccount::slotHandleTLSWarning (
		QCA::TLS::IdentityResult identityResult,
		QCA::Validity validityResult )
{
	kDebug ( JABBER_DEBUG_GLOBAL ) << "Handling TLS warning...";

	if ( handleTLSWarning ( m_jabberClient, identityResult, validityResult ) )
	{
		// resume stream
		m_jabberClient->continueAfterTLSWarning ();
	}
	else
	{
		// disconnect stream
		disconnect ( Kopete::Account::Manual );
	}

}

void JabberAccount::slotClientError ( JabberClient::ErrorCode errorCode )
{
	kDebug ( JABBER_DEBUG_GLOBAL ) << "Handling client error...";

	switch ( errorCode )
	{
		case JabberClient::NoTLS:
		default:
			KMessageBox::queuedMessageBox ( Kopete::UI::Global::mainWidget (), KMessageBox::Error,
					     i18n ("An encrypted connection with the Jabber server could not be established."),
					     i18n ("Jabber Connection Error"));
			disconnect ( Kopete::Account::Manual );
			break;
	}

}

void JabberAccount::slotConnected ()
{
	kDebug (JABBER_DEBUG_GLOBAL) << "Connected to Jabber server.";

#ifdef LIBJINGLE_SUPPORT
	loginLibjingle();
#endif

#ifdef JINGLE_SUPPORT
	qDebug() << "Create JingleCallsManager";
	m_jcm = new JingleCallsManager(this);
#endif

	kDebug (JABBER_DEBUG_GLOBAL) << "Requesting roster...";
	m_jabberClient->requestRoster ();
}

void JabberAccount::slotRosterRequestFinished ( bool success )
{

	if ( success )
	{
		// the roster was imported successfully, clear
		// all "dirty" items from the contact list
		contactPool()->cleanUp ();
	}

	/* Since we are online now, set initial presence. Don't do this
	* before the roster request or we will receive presence
	* information before we have updated our roster with actual
	* contacts from the server! (Iris won't forward presence
	* information in that case either). */
	kDebug (JABBER_DEBUG_GLOBAL) << "Setting initial presence...";
	setPresence ( m_initialPresence );

}

void JabberAccount::slotIncomingFileTransfer ()
{

	// delegate the work to a file transfer object
	new JabberFileTransfer ( this, client()->fileTransferManager()->takeIncoming () );

}

void JabberAccount::setOnlineStatus( const Kopete::OnlineStatus& status, const Kopete::StatusMessage &reason, const OnlineStatusOptions& /* options */ )
{
	XMPP::Status xmppStatus = m_protocol->kosToStatus( status, reason.message() );

	if( status.status() == Kopete::OnlineStatus::Offline )
	{

		#ifdef LIBJINGLE_SUPPORT
			m_libjingle->logout();
		#endif
			xmppStatus.setIsAvailable( false );
			kDebug (JABBER_DEBUG_GLOBAL) << "CROSS YOUR FINGERS! THIS IS GONNA BE WILD";
			disconnect (Manual, xmppStatus);	
			return;
    	}

	if( isConnecting () )
	{
		return;
	}
	

	if ( !isConnected () )
	{
		// we are not connected yet, so connect now
		m_initialPresence = xmppStatus;
		connect ( status );
	}
	else
	{
		setPresence ( xmppStatus );
	}
}

void JabberAccount::setStatusMessage( const Kopete::StatusMessage &statusMessage )
{
	setOnlineStatus( myself()->onlineStatus(), statusMessage, Kopete::Account::KeepSpecialFlags );
}

void JabberAccount::disconnect ( Kopete::Account::DisconnectReason reason )
{
	kDebug (JABBER_DEBUG_GLOBAL) << "disconnect() called";

	if (isConnected ())
	{
		kDebug (JABBER_DEBUG_GLOBAL) << "Still connected, closing connection...";
		/* Tell backend class to disconnect. */
		m_jabberClient->disconnect ();
	}

	// make sure that the connection animation gets stopped if we're still
	// in the process of connecting
	setPresence ( XMPP::Status ("", "", 0, false) );
	m_initialPresence = XMPP::Status ("", "", 5, true);

	/* FIXME:
	 * We should delete the JabberClient instance here,
	 * but active timers in Iris prevent us from doing so.
	 * (in a failed connection attempt, these timers will
	 * try to access an already deleted object).
	 * Instead, the instance will lurk until the next
	 * connection attempt.
	 */
	kDebug (JABBER_DEBUG_GLOBAL) << "Disconnected.";

	disconnected ( reason );
}

void JabberAccount::disconnect( Kopete::Account::DisconnectReason reason, XMPP::Status &status )
{
    kDebug (JABBER_DEBUG_GLOBAL) << "disconnect( reason, status ) called";
    
	if (isConnected ())
	{
		kDebug (JABBER_DEBUG_GLOBAL) << "Still connected, closing connection...";
		/* Tell backend class to disconnect. */
		m_jabberClient->disconnect (status);
	}

	// make sure that the connection animation gets stopped if we're still
	// in the process of connecting
	setPresence ( status );

	/* FIXME:
	 * We should delete the JabberClient instance here,
	 * but active timers in Iris prevent us from doing so.
	 * (in a failed connection attempt, these timers will
	 * try to access an already deleted object).
	 * Instead, the instance will lurk until the next
	 * connection attempt.
	 */
	kDebug (JABBER_DEBUG_GLOBAL) << "Disconnected.";

	Kopete::Account::disconnected ( reason );
}

void JabberAccount::disconnect ()
{
	disconnect ( Manual );
}

void JabberAccount::slotConnect ()
{
	connect ();
}

void JabberAccount::slotDisconnect ()
{
	disconnect ( Kopete::Account::Manual );
}

void JabberAccount::slotCSDisconnected ()
{
	kDebug (JABBER_DEBUG_GLOBAL) << "Disconnected from Jabber server.";

	/*
	 * We should delete the JabberClient instance here,
	 * but timers etc prevent us from doing so. Iris does
	 * not like to be deleted from a slot.
	 */
	if (isConnected() || isConnecting())
		disconnect ( Kopete::Account::Unknown );

	/* It seems that we don't get offline notifications when going offline
	 * with the protocol, so clear all resources manually. */
	resourcePool()->clear();

#ifdef JINGLE_SUPPORT
	delete m_jcm;
	m_jcm = 0L;
#endif

}

void JabberAccount::handleStreamError (int streamError, int streamCondition, int connectorCode, const QString &server, Kopete::Account::DisconnectReason &errorClass, QString additionalErrMsg)
{
	if ( Kopete::StatusManager::self()->globalStatusCategory() == Kopete::OnlineStatusManager::Busy )
		return;

	QString errorText;
	QString errorCondition;

	errorClass = Kopete::Account::Unknown;

	/*
	 * Display error to user.
	 * FIXME: for unknown errors, maybe add error codes?
	 */
	switch(streamError)
	{
		case XMPP::Stream::ErrParse:
			errorClass = Kopete::Account::Unknown;
			errorText = i18n("Malformed packet received.");
			break;

		case XMPP::Stream::ErrProtocol:
			errorClass = Kopete::Account::Unknown;
			errorText = i18n("There was an unrecoverable error in the protocol.");
			break;

		case XMPP::Stream::ErrStream:
			switch(streamCondition)
			{
				case XMPP::Stream::GenericStreamError:
					errorCondition = i18n("Generic stream error.");
					break;
				case XMPP::Stream::Conflict:
					// FIXME: need a better error message here
					errorCondition = i18n("There was a conflict in the information received.");
					break;
				case XMPP::Stream::ConnectionTimeout:
					errorCondition = i18n("The stream timed out.");
					break;
				case XMPP::Stream::InternalServerError:
					errorCondition = i18n("Internal server error.");
					break;
				case XMPP::Stream::InvalidFrom:
					errorCondition = i18n("Stream packet received from an invalid address.");
					break;
				case XMPP::Stream::InvalidXml:
					errorCondition = i18n("Malformed stream packet received.");
					break;
				case XMPP::Stream::PolicyViolation:
					// FIXME: need a better error message here
					errorCondition = i18n("Policy violation in the protocol stream.");
					break;
				case XMPP::Stream::ResourceConstraint:
					// FIXME: need a better error message here
					errorCondition = i18n("Resource constraint.");
					break;
				case XMPP::Stream::SystemShutdown:
					// FIXME: need a better error message here
					errorCondition = i18n("System shutdown.");
					break;
				default:
					errorCondition = i18n("Unknown reason.");
					break;
			}

			errorText = i18n("There was an error in the protocol stream: %1", errorCondition);
			break;

		case XMPP::ClientStream::ErrConnection:
			switch(connectorCode)
			{
 				case QAbstractSocket::HostNotFoundError:
					errorClass = Kopete::Account::InvalidHost;
					errorCondition = i18n("Host not found.");
					break;
				case QAbstractSocket::AddressInUseError:
					errorCondition = i18n("Address is already in use.");
					break;
				case QAbstractSocket::ConnectionRefusedError:
					errorCondition = i18n("Connection refused.");
					break;
				case QAbstractSocket::UnfinishedSocketOperationError:
					errorCondition = i18n("Connection attempt already in progress.");
					break;
				case QAbstractSocket::NetworkError:
					errorCondition = i18n("Network failure.");
					break;
				case QAbstractSocket::SocketTimeoutError:
					errorCondition = i18n("Socket timed out.");
					break;
				case QAbstractSocket::RemoteHostClosedError:
					errorCondition= i18n("Remote closed connection.");
					break;
				default:
					errorClass = Kopete::Account::ConnectionReset;
					errorCondition = i18n("Unexpected error condition (%1).", connectorCode);
					break;
			}
			if(!errorCondition.isEmpty())
				errorText = i18n("There was a connection error: %1", errorCondition);
			break;

		case XMPP::ClientStream::ErrNeg:
			switch(streamCondition)
			{
				case XMPP::ClientStream::HostUnknown:
					// FIXME: need a better error message here
					errorCondition = i18n("Unknown host.");
					break;
				case XMPP::ClientStream::RemoteConnectionFailed:
					// FIXME: need a better error message here
					errorCondition = i18n("Could not connect to a required remote resource.");
					break;
				case XMPP::ClientStream::SeeOtherHost:
					errorCondition = i18n("It appears we have been redirected to another server; I do not know how to handle this.");
					break;
				case XMPP::ClientStream::UnsupportedVersion:
					errorCondition = i18n("Unsupported protocol version.");
					break;
				default:
					errorCondition = i18n("Unknown error.");
					break;
			}

			errorText = i18n("There was a negotiation error: %1", errorCondition);
			break;

		case XMPP::ClientStream::ErrTLS:
			switch(streamCondition)
			{
				case XMPP::ClientStream::TLSStart:
					errorCondition = i18n("Server rejected our request to start the TLS handshake.");
					break;
				case XMPP::ClientStream::TLSFail:
					errorCondition = i18n("Failed to establish a secure connection.");
					break;
				default:
					errorCondition = i18n("Unknown error.");
					break;
			}

			errorText = i18n("There was a Transport Layer Security (TLS) error: %1", errorCondition);
			break;

		case XMPP::ClientStream::ErrAuth:
			switch(streamCondition)
			{
				case XMPP::ClientStream::GenericAuthError:
					errorCondition = i18n("Login failed with unknown reason.");
					break;
				case XMPP::ClientStream::NoMech:
					errorCondition = i18n("No appropriate authentication mechanism available.");
					break;
				case XMPP::ClientStream::BadProto:
					errorCondition = i18n("Bad SASL authentication protocol.");
					break;
				case XMPP::ClientStream::BadServ:
					errorCondition = i18n("Server failed mutual authentication.");
					break;
				case XMPP::ClientStream::EncryptionRequired:
					errorCondition = i18n("Encryption is required but not present.");
					break;
				case XMPP::ClientStream::InvalidAuthzid:
					errorCondition = i18n("Invalid user ID.");
					break;
				case XMPP::ClientStream::InvalidMech:
					errorCondition = i18n("Invalid mechanism.");
					break;
				case XMPP::ClientStream::InvalidRealm:
					errorCondition = i18n("Invalid realm.");
					break;
				case XMPP::ClientStream::MechTooWeak:
					errorCondition = i18n("Mechanism too weak.");
					break;
				case XMPP::ClientStream::NotAuthorized:
					errorCondition = i18n("Wrong credentials supplied. (check your user ID and password)");
					break;
				case XMPP::ClientStream::TemporaryAuthFailure:
					errorCondition = i18n("Temporary failure, please try again later.");
					break;
				default:
					errorCondition = i18n("Unknown error.");
					break;
			}

			errorText = i18n("There was an error authenticating with the server: %1", errorCondition);
			break;

		case XMPP::ClientStream::ErrSecurityLayer:
			switch(streamCondition)
			{
				case XMPP::ClientStream::LayerTLS:
					errorCondition = i18n("Transport Layer Security (TLS) problem.");
					break;
				case XMPP::ClientStream::LayerSASL:
					errorCondition = i18n("Simple Authentication and Security Layer (SASL) problem.");
					break;
				default:
					errorCondition = i18n("Unknown error.");
					break;
			}

			errorText = i18n("There was an error in the security layer: %1", errorCondition);
			break;

		case XMPP::ClientStream::ErrBind:
			switch(streamCondition)
			{
				case XMPP::ClientStream::BindNotAllowed:
					errorCondition = i18n("No permission to bind the resource.");
					break;
				case XMPP::ClientStream::BindConflict:
					errorCondition = i18n("The resource is already in use.");
					break;
				default:
					errorCondition = i18n("Unknown error.");
					break;
			}

			errorText = i18n("Could not bind a resource: %1", errorCondition);
			break;

		default:
			errorText = i18n("Unknown error.");
			break;
	}

	if(!errorText.isEmpty()) {
		if (!additionalErrMsg.isEmpty()) {
			errorText += "\n" + additionalErrMsg;
		}

		KNotification::event( QLatin1String("connection_error"), i18n("Kopete: Connection problem with Jabber server %1", server),
		                      errorText,
		                      KIconLoader::global()->iconPath( "jabber_protocol", KIconLoader::Dialog ),
		                      Kopete::UI::Global::mainWidget() );
	}
}

void JabberAccount::slotCSError ( int error )
{
	kDebug(JABBER_DEBUG_GLOBAL) << "Error in stream signalled.";

	if ( ( error == XMPP::ClientStream::ErrAuth )
		&& ( client()->clientStream()->errorCondition () == XMPP::ClientStream::NotAuthorized ) )
	{
		kDebug ( JABBER_DEBUG_GLOBAL ) << "Incorrect password, retrying.";
		disconnect(Kopete::Account::BadPassword);
	}
	else
	{
		Kopete::Account::DisconnectReason errorClass =  Kopete::Account::Unknown;

		kDebug ( JABBER_DEBUG_GLOBAL ) << "Disconnecting.";

		// display message to user
		// when removing or disconnecting, connection errors are normal
		if(!m_removing && (isConnected() || isConnecting()))
			handleStreamError (error, client()->clientStream()->errorCondition (), client()->clientConnector()->errorCode (), server (), errorClass, client()->clientStream()->errorText());

		if (isConnected() || isConnecting())
			disconnect ( errorClass );
		
		/*	slotCSDisconnected  will not be called*/
		resourcePool()->clear();
	}

}

/* Set presence (usually called by dialog widget). */
void JabberAccount::setPresence ( const XMPP::Status &status )
{
	kDebug(JABBER_DEBUG_GLOBAL) << "Status: " << status.show () << ", Reason: " << status.status ();

	// fetch input status
	XMPP::Status newStatus = status;
	
	// TODO: Check if Caps is enabled
	// Send entity capabilities
	if( client() )
	{
		newStatus.setCapsNode( client()->capsNode() );
		newStatus.setCapsVersion( client()->capsVersion() );
		newStatus.setCapsExt( client()->capsExt() );
	}

	// make sure the status gets the correct priority
	int newPriority = configGroup()->readEntry ( "Priority", 5 );
	if ( newStatus.isAway() && configGroup()->hasKey( "AwayPriority" ))
	{
		newPriority = configGroup()->readEntry( "AwayPriority", 0 );
	}
	newStatus.setPriority ( newPriority );
	kDebug(JABBER_DEBUG_GLOBAL) << "New priority: " << newPriority;

	XMPP::Jid jid;

	if ( client() )
		jid = client()->jid ();

	if ( jid.isEmpty() && myself() )
		jid = myself()->contactId ();

	if ( jid.isEmpty() )
		return;

	XMPP::Resource oldResource ( m_lastResource );

	kDebug(JABBER_DEBUG_GLOBAL) << "Old resource:" << m_lastResource;

	// update resource from jabber client
	m_lastResource = jid.resource();
	if ( m_lastResource.isEmpty() )
		m_lastResource = resource();

	XMPP::Resource newResource ( m_lastResource, newStatus );

	// for resource pool we need capabilities manager
	// JabberAccount::setPresence can be called when deleting JabberAccount
	if (protocol() && protocol()->capabilitiesManager())
	{
		// update our resource in the resource pool
		resourcePool()->addResource ( jid, newResource );

		// make sure that we only consider our own resource locally
		resourcePool()->lockToResource ( jid, newResource );

		// remove old resource if was changed and was not empty
		if ( ! oldResource.name().isEmpty() && oldResource.name() != newResource.name() )
			resourcePool()->removeResource ( jid, oldResource );
	}

	kDebug(JABBER_DEBUG_GLOBAL) << "New resource:" << m_lastResource;

	/*
	 * Unless we are in the connecting status, send a presence packet to the server
	 */
	if(status.show () != QString("connecting") )
	{
		/*
		 * Make sure we are actually connected before sending out a packet.
		 */
		if (isConnected())
		{
			kDebug(JABBER_DEBUG_GLOBAL) << "Sending new presence to the server.";

			XMPP::JT_Presence * task = new XMPP::JT_Presence ( client()->rootTask ());

			task->pres ( newStatus );
			task->go ( true );

			// update our capabilities (for this we need capabilities manager)
			if (protocol() && protocol()->capabilitiesManager())
			{
				m_lastStatus = newStatus;
				m_lastXMPPResource = newResource;
				protocol()->capabilitiesManager()->updateCapabilities( this, jid, m_lastStatus );
				QTimer::singleShot ( client()->getPenaltyTime () * 1000 + 2000, this, SLOT (slotUpdateOurCapabilities()) );
			}
		}
		else
		{
			kDebug(JABBER_DEBUG_GLOBAL) << "We were not connected, presence update aborted.";
		}
	}

}

void JabberAccount::slotUpdateOurCapabilities ()
{
	if ( ! myself() )
		return;
	XMPP::Jid jid ( myself()->contactId () );
	JabberResource * resource = resourcePool()->getJabberResource( jid, m_lastResource );
	if ( resource )
		resource->setResource ( m_lastXMPPResource );
	protocol()->capabilitiesManager()->updateCapabilities( this, jid, m_lastStatus );
	dynamic_cast<JabberContact *>(myself())->updateResourceList ();
}

void JabberAccount::slotXMPPConsole ()
{
	dlgXMPPConsole *w = new dlgXMPPConsole( client (), Kopete::UI::Global::mainWidget());
	QObject::connect( m_jabberClient, SIGNAL (incomingXML(QString)),
	                  w, SLOT (slotIncomingXML(QString)) );
	QObject::connect( m_jabberClient, SIGNAL (outgoingXML(QString)),
	                  w, SLOT (slotOutgoingXML(QString)) );
	w->show();
}

void JabberAccount::slotSetMood()
{
	KAction *action = (KAction *)sender();
	Mood::Type type = (Mood::Type)action->data().toInt();

    PubSubItem psi("current", Mood(type).toXml(*client()->client()->rootTask()->doc()));
    JT_PubSubPublish *task = new JT_PubSubPublish(client()->client()->rootTask(), QString("http://jabber.org/protocol/mood"), psi);
    task->go(true);

}

void JabberAccount::slotSubscription (const XMPP::Jid & jid, const QString & type)
{
	kDebug (JABBER_DEBUG_GLOBAL) << jid.full () << ", " << type;

	if (type == "subscribe")
	{
		/*
		 * A user wants to subscribe to our presence.
		 */
		kDebug (JABBER_DEBUG_GLOBAL) << jid.full () << " is asking for authorization to subscribe.";

		// Is the user already in our contact list?
		JabberBaseContact *contact = contactPool()->findExactMatch( jid );
		Kopete::MetaContact *metaContact=0L;
		if(contact)
			metaContact=contact->metaContact();

		Kopete::AddedInfoEvent::ShowActionOptions actions = Kopete::AddedInfoEvent::AuthorizeAction;
		actions |= Kopete::AddedInfoEvent::BlockAction;

		if( !metaContact || metaContact->isTemporary() )
			actions |= Kopete::AddedInfoEvent::AddAction;

		Kopete::AddedInfoEvent* event = new Kopete::AddedInfoEvent( jid.full(), this );
		QObject::connect( event, SIGNAL(actionActivated(uint)),
		                  this, SLOT(slotAddedInfoEventActionActivated(uint)) );

		event->showActions( actions );
		event->sendEvent();
	}
	else if (type == "unsubscribed")
	{
		/*
		 * Someone else removed our authorization to see them.
		 */
		kDebug (JABBER_DEBUG_GLOBAL) << jid.full() << " revoked our presence authorization";

		XMPP::JT_Roster *task;

		switch (KMessageBox::warningYesNo (Kopete::UI::Global::mainWidget(),
								  i18n
								  ("The Jabber user %1 removed %2's subscription to him/her. "
								   "This account will no longer be able to view his/her online/offline status. "
								   "Do you want to delete the contact?",
								    jid.full(), accountId()), i18n ("Notification"), KStandardGuiItem::del(), KGuiItem( i18n("Keep") )))
		{

			case KMessageBox::Yes:
				/*
				 * Delete this contact from our roster.
				 */
				task = new XMPP::JT_Roster ( client()->rootTask ());

				task->remove (jid);
				task->go (true);

				break;

			default:
				/*
				 * We want to leave the contact in our contact list.
				 * In this case, we need to delete all the resources
				 * we have for it, as the Jabber server won't signal us
				 * that the contact is offline now.
				 */
				resourcePool()->removeAllResources ( jid );
				break;

		}
	}
}

void JabberAccount::slotAddedInfoEventActionActivated ( uint actionId )
{
	const Kopete::AddedInfoEvent *event =
		dynamic_cast<const Kopete::AddedInfoEvent *>(sender());

	if ( !event || !isConnected() )
		return;

	XMPP::Jid jid(event->contactId());
	if ( actionId == Kopete::AddedInfoEvent::AuthorizeAction )
	{
		/*
		* Authorize user.
		*/
		XMPP::JT_Presence *task = new XMPP::JT_Presence ( client()->rootTask () );
		task->sub ( jid, "subscribed" );
		task->go ( true );
	}
	else if ( actionId == Kopete::AddedInfoEvent::BlockAction )
	{
		/*
		* Reject subscription.
		*/
		XMPP::JT_Presence *task = new XMPP::JT_Presence ( client()->rootTask () );
		task->sub ( jid, "unsubscribed" );
		task->go ( true );
	}
	else if( actionId == Kopete::AddedInfoEvent::AddContactAction )
	{
		Kopete::MetaContact *parentContact=event->addContact();
		if(parentContact)
		{
			QStringList groupNames;
			Kopete::GroupList groupList = parentContact->groups();
			foreach(Kopete::Group *group,groupList)
			{
				if (group->type() == Kopete::Group::Normal)
					groupNames += group->displayName();
				else if (group->type() == Kopete::Group::TopLevel)
					groupNames += QString();
			}

			if(groupNames.size() == 1 && groupNames.at(0).isEmpty())
				groupNames.clear();

			XMPP::RosterItem item;

			item.setJid ( jid );
			item.setName ( parentContact->displayName() );
			item.setGroups ( groupNames );

			// add the new contact to our roster.
			XMPP::JT_Roster * rosterTask = new XMPP::JT_Roster ( client()->rootTask () );

			rosterTask->set ( item.jid(), item.name(), item.groups() );
			rosterTask->go ( true );

			// send a subscription request.
			XMPP::JT_Presence *presenceTask = new XMPP::JT_Presence ( client()->rootTask () );
	
			presenceTask->sub ( jid, "subscribe" );
			presenceTask->go ( true );
		}
	}
}



void JabberAccount::slotContactUpdated (const XMPP::RosterItem & item)
{

	/**
	 * Subscription types are: Both, From, To, Remove, None.
	 * Both:   Both sides have authed each other, each side
	 *         can see each other's presence
	 * From:   The other side can see us.
	 * To:     We can see the other side. (implies we are
	 *         authed)
	 * Remove: Other side revoked our subscription request.
	 *         Not to be handled here.
	 * None:   No subscription.
	 *
	 * Regardless of the subscription type, we have to add
	 * a roster item here.
	 */

	kDebug (JABBER_DEBUG_GLOBAL) << "New roster item " << item.jid().full () << " (Subscription: " << item.subscription().toString () << ")";

	/*
	 * See if the contact need to be added, according to the criterias of 
	 *  JEP-0162: Best Practices for Roster and Subscription Management
	 * http://www.jabber.org/jeps/jep-0162.html#contacts
	 */
	bool need_to_add=false;
	if(item.subscription().type() == XMPP::Subscription::Both || item.subscription().type() == XMPP::Subscription::To)
		need_to_add = true;
	else if( !item.ask().isEmpty() )
		need_to_add = true;
	else if( !item.name().isEmpty() || !item.groups().isEmpty() )
		need_to_add = true;
	
	/*
	 * See if the contact is already on our contact list
	 */
	Kopete::Contact *c= contactPool()->findExactMatch( item.jid() );
	
	if( c && c == c->Kopete::Contact::account()->myself() )  //don't use JabberBaseContact::account() which return alwaus the JabberAccount, and not the transport
	{
		// don't let remove the gateway contact, eh!
		need_to_add = true;  
	}

	if(need_to_add)
	{
		Kopete::MetaContact *metaContact=0L;
		if (!c)
		{
			/*
			* No metacontact has been found which contains a contact with this ID,
			* so add a new metacontact to the list.
			*/
			metaContact = new Kopete::MetaContact ();
			QStringList groups = item.groups ();
	
			// add this metacontact to all groups the contact is a member of
			for (QStringList::Iterator it = groups.begin (); it != groups.end (); ++it)
			{
				if ( it->isEmpty() )
					metaContact->addToGroup (Kopete::Group::topLevel ());
				else
					metaContact->addToGroup (Kopete::ContactList::self ()->findGroup (*it));
			}
	
			// put it onto contact list
			Kopete::ContactList::self ()->addMetaContact ( metaContact );
		}
		else
		{
			metaContact=c->metaContact();
			//TODO: synchronize groups
		}

		/*
		* Add / update the contact in our pool. In case the contact is already there,
		* it will be updated. In case the contact is not in the meta contact yet, it
		* will be added to it.
		* The "dirty" flag is false here, because we just received the contact from
		* the server's roster. As such, it is now a synchronized entry.
		*/
		JabberContact *contact = contactPool()->addContact ( item, metaContact, false );
		if (!contact) // Don't crash if we have JabberContact with the same jid as JabberGroupContact, bug 204243
			return;

		/*
		* Set authorization property
		*/
		if ( !item.ask().isEmpty () )
		{
			contact->setProperty ( protocol()->propAuthorizationStatus, i18n ( "Waiting for authorization" ) );
		}
		else
		{
			contact->removeProperty ( protocol()->propAuthorizationStatus );
		}
	}
	else if(c)  //we don't need to add it, and it is in the contact list
	{
		Kopete::MetaContact *metaContact=c->metaContact();
		if(metaContact->isTemporary())
			return;
		kDebug (JABBER_DEBUG_GLOBAL) << c->contactId() << 
				" is on the contact list while it should not.  we are removing it.  - " << c << endl;
		delete c;
		if(metaContact->contacts().isEmpty())
			Kopete::ContactList::self()->removeMetaContact( metaContact );
	}

}

void JabberAccount::slotContactDeleted (const XMPP::RosterItem & item)
{
	kDebug (JABBER_DEBUG_GLOBAL) << "Deleting contact " << item.jid().full ();

	// since the contact instance will get deleted here, the GUI should be updated
	contactPool()->removeContact ( item.jid () );

}

void JabberAccount::slotReceivedMessage (const XMPP::Message & message)
{
	kDebug (JABBER_DEBUG_GLOBAL) << "New message from " << message.from().full ();

	JabberBaseContact *contactFrom;

	if ( message.type() == "groupchat" )
	{
		// this is a groupchat message, forward it to the group contact
		// (the one without resource name)
		XMPP::Jid jid ( message.from().bare() );

		// try to locate an exact match in our pool first
		contactFrom = contactPool()->findExactMatch ( jid );

		/**
		 * If there was no exact match, something is really messed up.
		 * We can't receive groupchat messages from rooms that we are
		 * not a member of and if the room contact vanished somehow,
		 * we're in deep trouble.
		 */
		if ( !contactFrom )
		{
			kDebug ( JABBER_DEBUG_GLOBAL ) << "WARNING: Received a groupchat message but could not find room contact. Ignoring message.";
			return;
		}
	}
	else
	{
		// try to locate an exact match in our pool first
		contactFrom = contactPool()->findExactMatch ( message.from () );

		if ( !contactFrom )
		{
			// we have no exact match, try a broader search
			contactFrom = contactPool()->findRelevantRecipient ( message.from () );
		}

		// see if we found the contact in our pool
		if ( !contactFrom )
		{
			// eliminate the resource from this contact,
			// otherwise we will add the contact with the
			// resource to our list
			// NOTE: This is a stupid way to do it, but
			// message.from().setResource("") had no
			// effect. Iris bug?
			XMPP::Jid jid ( message.from().bare() );

			// the contact is not in our pool, add it as a temporary contact
			kDebug (JABBER_DEBUG_GLOBAL) << jid.full () << " is unknown to us, creating temporary contact.";

			Kopete::MetaContact *metaContact = new Kopete::MetaContact ();

			metaContact->setTemporary (true);

			contactFrom = contactPool()->addContact ( XMPP::RosterItem ( jid ), metaContact, false );

			Kopete::ContactList::self ()->addMetaContact (metaContact);
		}
		else if ( contactFrom->inherits( (const char*)("JabberGroupMemberContact") ) )
		{	// Add JabberGroupMemberContact to contact list because we have received private group chat message
			Kopete::ContactList::self()->addMetaContact( contactFrom->metaContact() );
		}
	}

	// pass the message on to the contact
	contactFrom->handleIncomingMessage (message);

}

void JabberAccount::slotJoinNewChat ()
{

	if (!isConnected ())
	{
		errorConnectFirst ();
		return;
	}

	dlgJabberChatJoin *joinDialog = new dlgJabberChatJoin ( this, Kopete::UI::Global::mainWidget () );
	joinDialog->show ();

}

void JabberAccount::slotGroupChatJoined (const XMPP::Jid & jid)
{
	kDebug (JABBER_DEBUG_GLOBAL) << "Joined groupchat " << jid.full ();

	// Create new meta contact that holds the groupchat contact.
	Kopete::MetaContact *metaContact = new Kopete::MetaContact ();

	metaContact->setTemporary ( true );

	// Create a groupchat contact for this room
	JabberGroupContact *groupContact = dynamic_cast<JabberGroupContact *>( contactPool()->addGroupContact ( XMPP::RosterItem ( jid ), true, metaContact, false ) );

	if(groupContact)
	{
		// Add the groupchat contact to the meta contact.
		//metaContact->addContact ( groupContact );
	
		Kopete::ContactList::self ()->addMetaContact ( metaContact );
	}
	else
		delete metaContact;

	/**
	 * Add an initial resource for this contact to the pool. We need
	 * to do this to be able to lock the group status to our own presence.
	 * Our own presence will be updated right after this method returned
	 * by slotGroupChatPresence(), since the server will signal our own
	 * presence back to us.
	 */
	resourcePool()->addResource ( XMPP::Jid ( jid.bare() ), XMPP::Resource ( jid.resource () ) );

	// lock the room to our own status
	resourcePool()->lockToResource ( XMPP::Jid ( jid.bare() ), jid.resource () );
	
	m_bookmarks->insertGroupChat(jid);	
}

void JabberAccount::slotGroupChatLeft (const XMPP::Jid & jid)
{
	kDebug (JABBER_DEBUG_GLOBAL) << "Left groupchat " << jid.full ();
	
	// remove group contact from list
	Kopete::Contact *contact = 
			Kopete::ContactList::self()->findContact( protocol()->pluginId() , accountId() , jid.bare() );

	if ( contact )
	{
		Kopete::MetaContact *metaContact= contact->metaContact();
		if( metaContact && metaContact->isTemporary() )
			Kopete::ContactList::self()->removeMetaContact ( metaContact );
		else
			contact->deleteLater();
	}

	// now remove it from our pool, which should clean up all subcontacts as well
	contactPool()->removeContact ( XMPP::Jid ( jid.bare() ) );
	
}

void JabberAccount::slotGroupChatPresence (const XMPP::Jid & jid, const XMPP::Status & status)
{
	kDebug (JABBER_DEBUG_GLOBAL) << "Received groupchat presence for room " << jid.full ();

	// fetch room contact (the one without resource)
	JabberGroupContact *groupContact = dynamic_cast<JabberGroupContact *>( contactPool()->findExactMatch ( XMPP::Jid ( jid.bare() ) ) );

	if ( !groupContact )
	{
		kDebug ( JABBER_DEBUG_GLOBAL ) << "WARNING: Groupchat presence signalled, but we do not have a room contact?";
		return;
	}

	if ( !status.isAvailable () )
	{
		kDebug ( JABBER_DEBUG_GLOBAL ) << jid.full () << " has become unavailable, removing from room";

		// remove the resource from the pool
		resourcePool()->removeResource ( jid, XMPP::Resource ( jid.resource (), status ) );

		// the person has become unavailable, remove it
		groupContact->removeSubContact ( XMPP::RosterItem ( jid ) );
	}
	else
	{
		// add a resource for this contact to the pool (existing resources will be updated)
		resourcePool()->addResource ( jid, XMPP::Resource ( jid.resource (), status ) );

		// make sure the contact exists in the room (if it exists already, it won't be added twice)
		groupContact->addSubContact ( XMPP::RosterItem ( jid ) );
	}

}

void JabberAccount::slotGroupChatError (const XMPP::Jid &jid, int error, const QString &reason)
{
	kDebug (JABBER_DEBUG_GLOBAL) << "Group chat error - room " << jid.full () << " had error " << error << " (" << reason << ")";

	switch (error)
	{
	case JabberClient::InvalidPasswordForMUC:
		{
			QPointer <KPasswordDialog> dlg = new KPasswordDialog(Kopete::UI::Global::mainWidget());
			dlg->setPrompt(i18n("A password is required to join the room %1.", jid.node()));
			if (dlg->exec() == KPasswordDialog::Accepted && dlg)
				m_jabberClient->joinGroupChat(jid.domain(), jid.node(), jid.resource(), dlg->password());
			delete dlg;
		}
		break;

	case JabberClient::NicknameConflict:
		{
			bool ok;
			QString nickname = KInputDialog::getText(i18n("Error trying to join %1 : nickname %2 is already in use", jid.node(), jid.resource()),
									i18n("Provide your nickname"),
									QString(),
									&ok);
			if (ok)
			{
				m_jabberClient->joinGroupChat(jid.domain(), jid.node(), nickname);
			}
		}
		break;

	case JabberClient::BannedFromThisMUC:
		KMessageBox::queuedMessageBox ( Kopete::UI::Global::mainWidget (),
									KMessageBox::Error,
									i18n ("You cannot join the room %1 because you have been banned", jid.node()),
									i18n ("Jabber Group Chat") );
		break;

	case JabberClient::MaxUsersReachedForThisMuc:
		KMessageBox::queuedMessageBox ( Kopete::UI::Global::mainWidget (),
									KMessageBox::Error,
									i18n ("You cannot join the room %1 because the maximum number of users has been reached", jid.node()),
									i18n ("Jabber Group Chat") );
		break;

	default:
		{
		QString detailedReason = reason.isEmpty () ? i18n ( "No reason given by the server" ) : reason;

		KMessageBox::queuedMessageBox ( Kopete::UI::Global::mainWidget (),
									KMessageBox::Error,
									i18n ("There was an error processing your request for groupchat %1. (Reason: %2, Code %3)", jid.full (), detailedReason, error ),
									i18n ("Jabber Group Chat") );
		}
	}
}

void JabberAccount::slotResourceAvailable (const XMPP::Jid & jid, const XMPP::Resource & resource)
{

	kDebug (JABBER_DEBUG_GLOBAL) << "New resource available for " << jid.full();

	resourcePool()->addResource ( jid, resource );

}

void JabberAccount::slotResourceUnavailable (const XMPP::Jid & jid, const XMPP::Resource & resource)
{

	kDebug (JABBER_DEBUG_GLOBAL) << "Resource now unavailable for " << jid.full ();

	resourcePool()->removeResource ( jid, resource );

}

void JabberAccount::slotEditVCard ()
{
	static_cast<JabberContact *>( myself() )->slotUserInfo ();
}

QString JabberAccount::resource () const
{

	return configGroup()->readEntry ( "Resource", "Kopete" );

}

QString JabberAccount::server () const
{

	return configGroup()->readEntry ( "Server" );

}

int JabberAccount::port () const
{

	return configGroup()->readEntry ( "Port", 5222 );

}

void JabberAccount::slotGetServices ()
{
	dlgJabberServices *dialog = new dlgJabberServices (this);

	dialog->show ();
	dialog->raise ();
}

void JabberAccount::addTransport( JabberTransport * tr, const QString &jid )
{
	m_transports.insert(jid,tr);
}

void JabberAccount::removeTransport( const QString &jid )
{
	m_transports.remove(jid);
}

bool JabberAccount::removeAccount( )
{
	if(!m_removing)
	{
		int result=KMessageBox::warningYesNoCancel( Kopete::UI::Global::mainWidget () ,
				   i18n( "Do you want to also unregister \"%1\" from the Jabber server ?\n"
				   			    "If you unregister, your whole contact list may be removed from the server,"
							    " and you will never be able to connect to this account with any client", accountLabel() ),
					i18n("Unregister"),
					KGuiItem(i18n( "Remove and Unregister" ), "edit-delete"),
					KGuiItem(i18n( "Remove only from Kopete"), "user-trash"),KStandardGuiItem::cancel(),
					QString(), KMessageBox::Notify | KMessageBox::Dangerous );
		if(result == KMessageBox::Cancel)
		{
			return false;
		}
		else if(result == KMessageBox::Yes)
		{
			if (!isConnected())
			{
				errorConnectFirst ();
				return false;
			}
			
			XMPP::JT_Register *task = new XMPP::JT_Register ( client()->rootTask () );
			QObject::connect ( task, SIGNAL (finished()), this, SLOT (slotUnregisterFinished) );
			task->unreg ();
			task->go ( true );
			m_removing=true;
			// from my experiment, not all server reply us with a response.   it simply dosconnect
			// so after one seconde, we will force to remove the account
			QTimer::singleShot(1111, this, SLOT(slotUnregisterFinished())); 
			
			return false; //the account will be removed when the task will be finished
		}
	}
	
	//remove transports from config file.
	QMap<QString,JabberTransport*> tranposrts_copy=m_transports;
	QMap<QString,JabberTransport*>::Iterator it;
	for ( it = tranposrts_copy.begin(); it != tranposrts_copy.end(); ++it )
	{
		(*it)->jabberAccountRemoved();
	}
	return true;
}

void JabberAccount::slotUnregisterFinished( )
{
	const XMPP::JT_Register * task = dynamic_cast<const XMPP::JT_Register *>(sender ());

	if ( task && ! task->success ())
	{
		KMessageBox::queuedMessageBox ( 0L, KMessageBox::Error,
			i18n ("An error occurred while trying to remove the account:\n%1", task->statusString()),
			i18n ("Jabber Account Unregistration"));
		m_removing=false;
		return;
	}
	if(m_removing)  //it may be because this is now the timer.
		Kopete::AccountManager::self()->removeAccount( this ); //this will delete this
}

#ifdef LIBJINGLE_SUPPORT

void JabberAccount::loginLibjingle()
{
	if ( ! enabledLibjingle() || m_libjingle->isConnected() )
		return;

	bool customServer = configGroup()->readEntry("CustomServer", false);

	if ( customServer ) {
		m_libjingle->setServer(server(), port());
		m_libjingle->login();
		return;
	}

	XMPP::Jid jid(myself()->contactId());

	/* We need to specify server only if jabber account is not google's one */
	if ( jid.domain() == "gmail.com" ) {
		m_libjingle->login();
		return;
	}

	XMPP::ServiceResolver * resolver = new XMPP::ServiceResolver;
	resolver->setProtocol(XMPP::ServiceResolver::IPv4); // libjingle does not support ipv6
	QObject::connect(resolver, SIGNAL(resultReady(QHostAddress,quint16)), this, SLOT(loginLibjingleResolver(QHostAddress,quint16)));
	QObject::connect(resolver, SIGNAL(error(XMPP::ServiceResolver::Error)), resolver, SLOT(deleteLater()));
	resolver->start("xmpp-client", "tcp", jid.domain(), 5222);
}

void JabberAccount::loginLibjingleResolver(const QHostAddress &address, quint16 port)
{
	XMPP::ServiceResolver * resolver = qobject_cast<XMPP::ServiceResolver *>(sender());
	if ( resolver ) {
		QObject::disconnect(resolver, 0, 0, 0);
		resolver->deleteLater();
	}

	kDebug (JABBER_DEBUG_GLOBAL) << "address:" << address.toString() << "port:" << port;

	m_libjingle->setServer(address.toString(), port);
	m_libjingle->login();
}

bool JabberAccount::enabledLibjingle()
{
	return configGroup()->readEntry("Libjingle", true);
}

void JabberAccount::enableLibjingle(bool b)
{
	configGroup()->writeEntry("Libjingle", b);
	if ( ! b )
		m_libjingle->logout();
	else if ( isConnected() )
		loginLibjingle();
}

bool JabberAccount::supportLibjingle(const QString &user)
{
	return ( enabledLibjingle() && m_libjingle->isOnline(user) );
}

void JabberAccount::makeLibjingleCall(const QString &user)
{
	if ( enabledLibjingle() )
		m_libjingle->makeCall(user);
}

#endif

void JabberAccount::setMergeMessages(bool b)
{
	configGroup()->writeEntry("MergeMessages", b);
}

bool JabberAccount::mergeMessages()
{
	return configGroup()->readEntry("MergeMessages", true);
}

void JabberAccount::setOldEncrypted(bool b)
{
	configGroup()->writeEntry("OldEncrypted", b);
}

bool JabberAccount::oldEncrypted()
{
	return configGroup()->readEntry("OldEncrypted", false);
}

/*
JabberMoodAction::JabberMoodAction(const Mood::Type type, QObject *parent):
KAction(parent)
{
	mType = type;
	setText(MoodManager::self()->getMoodName(mType));
	//setIcon( KIcon( QString( "icq_xstatus%1" ).arg( mStatus.status() ) ) );
	//setToolTip();
	QObject::connect(this, SIGNAL(triggered(bool)), this, SLOT(triggered()));
}

void JabberMoodAction::triggered()
{
	emit triggered(mType);
}*/

#include "jabberaccount.moc"

// vim: set noet ts=4 sts=4 sw=4:
