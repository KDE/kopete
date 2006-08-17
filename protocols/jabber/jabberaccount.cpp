
/***************************************************************************
                   jabberaccount.cpp  -  core Jabber account class
                             -------------------
    begin                : Sat M??? 8 2003
    copyright            : (C) 2003 by Till Gerken <till@tantalo.net>
							Based on JabberProtocol by Daniel Stone <dstone@kde.org>
							and Till Gerken <till@tantalo.net>.
	copyright            : (C) 2006 by Olivier Goffart <ogoffart at kde.org>

			   Kopete (C) 2001-2003 Kopete developers
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

#include "im.h"
#include "filetransfer.h"
#include "xmpp.h"
#include "xmpp_tasks.h"
#include "qca.h"
#include "bsocket.h"

#include "jabberaccount.h"
#include "jabberbookmarks.h"

#include <time.h>

#include <qstring.h>
#include <qregexp.h>
#include <qtimer.h>

#include <kconfig.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kapplication.h>
#include <kaboutdata.h>
#include <ksocketbase.h>
#include <kpassdlg.h>
#include <kinputdialog.h>

#include "kopetepassword.h"
#include "kopeteawayaction.h"
#include "kopetemetacontact.h"
#include "kopeteuiglobal.h"
#include "kopetegroup.h"
#include "kopetecontactlist.h"
#include "kopeteaccountmanager.h"
#include "contactaddednotifydialog.h"

#include "jabberconnector.h"
#include "jabberclient.h"
#include "jabberprotocol.h"
#include "jabberresourcepool.h"
#include "jabbercontactpool.h"
#include "jabberfiletransfer.h"
#include "jabbercontact.h"
#include "jabbergroupcontact.h"
#include "jabbercapabilitiesmanager.h"
#include "jabbertransport.h"
#include "dlgjabbersendraw.h"
#include "dlgjabberservices.h"
#include "dlgjabberchatjoin.h"

#include <sys/utsname.h>

#ifdef SUPPORT_JINGLE
#include "voicecaller.h"
#include "jinglevoicecaller.h"

// NOTE: Disabled for 0.12, will develop them futher in KDE4
// #include "jinglesessionmanager.h"
// #include "jinglesession.h"
// #include "jinglevoicesession.h"
#include "jinglevoicesessiondialog.h"
#endif

#define KOPETE_CAPS_NODE "http://kopete.kde.org/jabber/caps"



JabberAccount::JabberAccount (JabberProtocol * parent, const QString & accountId, const char *name)
			  :Kopete::PasswordedAccount ( parent, accountId, 0, name )
{
	kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Instantiating new account " << accountId << endl;

	m_protocol = parent;

	m_jabberClient = 0L;

	m_resourcePool = 0L;
	m_contactPool = 0L;
#ifdef SUPPORT_JINGLE
	m_voiceCaller = 0L;
	//m_jingleSessionManager = 0L; // NOTE: Disabled for 0.12
#endif
	m_bookmarks = new JabberBookmarks(this);
	m_removing=false;
	m_notifiedUserCannotBindTransferPort = false;
	// add our own contact to the pool
	JabberContact *myContact = contactPool()->addContact ( XMPP::RosterItem ( accountId ), Kopete::ContactList::self()->myself(), false );
	setMyself( myContact );

	QObject::connect(Kopete::ContactList::self(), SIGNAL( globalIdentityChanged(const QString&, const QVariant& ) ), SLOT( slotGlobalIdentityChanged(const QString&, const QVariant& ) ) );

	m_initialPresence = XMPP::Status ( "", "", 5, true );

}

JabberAccount::~JabberAccount ()
{
	disconnect ( Kopete::Account::Manual );

	// Remove this account from Capabilities manager.
	protocol()->capabilitiesManager()->removeAccount( this );

	cleanup ();
	
	QMap<QString,JabberTransport*> tranposrts_copy=m_transports;
	QMap<QString,JabberTransport*>::Iterator it;
	for ( it = tranposrts_copy.begin(); it != tranposrts_copy.end(); ++it ) 
		delete it.data();
}

void JabberAccount::cleanup ()
{

	delete m_jabberClient;
	
	m_jabberClient = 0L;

	delete m_resourcePool;
	m_resourcePool = 0L;

	delete m_contactPool;
	m_contactPool = 0L;
	
#ifdef SUPPORT_JINGLE
	delete m_voiceCaller;
	m_voiceCaller = 0L;

// 	delete m_jingleSessionManager;
// 	m_jingleSessionManager = 0L;
#endif
}

void JabberAccount::setS5BServerPort ( int port )
{

	if ( !m_jabberClient )
	{
		return;
	}

	if ( !m_jabberClient->setS5BServerPort ( port ) && !m_notifiedUserCannotBindTransferPort)
	{
		KMessageBox::queuedMessageBox ( Kopete::UI::Global::mainWidget (), KMessageBox::Sorry,
							 i18n ( "Could not bind Jabber file transfer manager to local port. Please check if the file transfer port is already in use or choose another port in the account settings." ),
							 i18n ( "Failed to start Jabber File Transfer Manager" ) );
		m_notifiedUserCannotBindTransferPort = true;
	}

}

KActionMenu *JabberAccount::actionMenu ()
{
	KActionMenu *m_actionMenu = Kopete::Account::actionMenu();

	m_actionMenu->popupMenu()->insertSeparator();

	KAction *action;
	
	action = new KAction (i18n ("Join Groupchat..."), "jabber_group", 0, this, SLOT (slotJoinNewChat ()), this, "actionJoinChat");
	m_actionMenu->insert(action);
	action->setEnabled( isConnected() );
	
	action = m_bookmarks->bookmarksAction( m_bookmarks );
	m_actionMenu->insert(action);
	action->setEnabled( isConnected() );


	m_actionMenu->popupMenu()->insertSeparator();
	
	action =  new KAction ( i18n ("Services..."), "jabber_serv_on", 0,
							this, SLOT ( slotGetServices () ), this, "actionJabberServices");
	action->setEnabled( isConnected() );
	m_actionMenu->insert ( action );

	action = new KAction ( i18n ("Send Raw Packet to Server..."), "mail_new", 0,
										 this, SLOT ( slotSendRaw () ), this, "actionJabberSendRaw") ;
	action->setEnabled( isConnected() );
	m_actionMenu->insert ( action );

	action = new KAction ( i18n ("Edit User Info..."), "identity", 0,
										 this, SLOT ( slotEditVCard () ), this, "actionEditVCard") ;
	action->setEnabled( isConnected() );
	m_actionMenu->insert ( action );


	return m_actionMenu;

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
	for(Kopete::Group *group = groupList.first(); group; group = groupList.next())
		groupNames += group->displayName();

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
	kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "called" << endl;

	/* Cancel connection process if no password has been supplied. */
	if ( password.isEmpty () )
	{
		disconnect ( Kopete::Account::Manual );
		return;
	}

	/* Don't do anything if we are already connected. */
	if ( isConnected () )
		return;

	// instantiate new client backend or clean up old one
	if ( !m_jabberClient )
	{
		m_jabberClient = new JabberClient;
	
		QObject::connect ( m_jabberClient, SIGNAL ( csDisconnected () ), this, SLOT ( slotCSDisconnected () ) );
		QObject::connect ( m_jabberClient, SIGNAL ( csError ( int ) ), this, SLOT ( slotCSError ( int ) ) );
		QObject::connect ( m_jabberClient, SIGNAL ( tlsWarning ( int ) ), this, SLOT ( slotHandleTLSWarning ( int ) ) );
		QObject::connect ( m_jabberClient, SIGNAL ( connected () ), this, SLOT ( slotConnected () ) );
		QObject::connect ( m_jabberClient, SIGNAL ( error ( JabberClient::ErrorCode ) ), this, SLOT ( slotClientError ( JabberClient::ErrorCode ) ) );

		QObject::connect ( m_jabberClient, SIGNAL ( subscription ( const XMPP::Jid &, const QString & ) ),
				   this, SLOT ( slotSubscription ( const XMPP::Jid &, const QString & ) ) );
		QObject::connect ( m_jabberClient, SIGNAL ( rosterRequestFinished ( bool ) ),
				   this, SLOT ( slotRosterRequestFinished ( bool ) ) );
		QObject::connect ( m_jabberClient, SIGNAL ( newContact ( const XMPP::RosterItem & ) ),
				   this, SLOT ( slotContactUpdated ( const XMPP::RosterItem & ) ) );
		QObject::connect ( m_jabberClient, SIGNAL ( contactUpdated ( const XMPP::RosterItem & ) ),
				   this, SLOT ( slotContactUpdated ( const XMPP::RosterItem & ) ) );
		QObject::connect ( m_jabberClient, SIGNAL ( contactDeleted ( const XMPP::RosterItem & ) ),
				   this, SLOT ( slotContactDeleted ( const XMPP::RosterItem & ) ) );
		QObject::connect ( m_jabberClient, SIGNAL ( resourceAvailable ( const XMPP::Jid &, const XMPP::Resource & ) ),
				   this, SLOT ( slotResourceAvailable ( const XMPP::Jid &, const XMPP::Resource & ) ) );
		QObject::connect ( m_jabberClient, SIGNAL ( resourceUnavailable ( const XMPP::Jid &, const XMPP::Resource & ) ),
				   this, SLOT ( slotResourceUnavailable ( const XMPP::Jid &, const XMPP::Resource & ) ) );
		QObject::connect ( m_jabberClient, SIGNAL ( messageReceived ( const XMPP::Message & ) ),
				   this, SLOT ( slotReceivedMessage ( const XMPP::Message & ) ) );
		QObject::connect ( m_jabberClient, SIGNAL ( incomingFileTransfer () ),
				   this, SLOT ( slotIncomingFileTransfer () ) );
		QObject::connect ( m_jabberClient, SIGNAL ( groupChatJoined ( const XMPP::Jid & ) ),
				   this, SLOT ( slotGroupChatJoined ( const XMPP::Jid & ) ) );
		QObject::connect ( m_jabberClient, SIGNAL ( groupChatLeft ( const XMPP::Jid & ) ),
				   this, SLOT ( slotGroupChatLeft ( const XMPP::Jid & ) ) );
		QObject::connect ( m_jabberClient, SIGNAL ( groupChatPresence ( const XMPP::Jid &, const XMPP::Status & ) ),
				   this, SLOT ( slotGroupChatPresence ( const XMPP::Jid &, const XMPP::Status & ) ) );
		QObject::connect ( m_jabberClient, SIGNAL ( groupChatError ( const XMPP::Jid &, int, const QString & ) ),
				   this, SLOT ( slotGroupChatError ( const XMPP::Jid &, int, const QString & ) ) );
		QObject::connect ( m_jabberClient, SIGNAL ( debugMessage ( const QString & ) ),
				   this, SLOT ( slotClientDebugMessage ( const QString & ) ) );
	}
	else
	{
		m_jabberClient->disconnect ();
	}

	// we need to use the old protocol for now
	m_jabberClient->setUseXMPP09 ( true );

	// set SSL flag (this should be converted to forceTLS when using the new protocol)
	m_jabberClient->setUseSSL ( configGroup()->readBoolEntry ( "UseSSL", false ) );

	// override server and port (this should be dropped when using the new protocol and no direct SSL)
	m_jabberClient->setOverrideHost ( true, server (), port () );

	// allow plaintext password authentication or not?
	m_jabberClient->setAllowPlainTextPassword ( configGroup()->readBoolEntry ( "AllowPlainTextPassword", false ) );

	// enable file transfer (if empty, IP will be set after connection has been established)
	KGlobal::config()->setGroup ( "Jabber" );
	m_jabberClient->setFileTransfersEnabled ( true, KGlobal::config()->readEntry ( "LocalIP" ) );
	setS5BServerPort ( KGlobal::config()->readNumEntry ( "LocalPort", 8010 ) );

	//
	// Determine system name
	//
	if ( !configGroup()->readBoolEntry ( "HideSystemInfo", false ) )
	{
		struct utsname utsBuf;

		uname (&utsBuf);

		m_jabberClient->setClientName ("Kopete");
		m_jabberClient->setClientVersion (kapp->aboutData ()->version ());
		m_jabberClient->setOSName (QString ("%1 %2").arg (utsBuf.sysname, 1).arg (utsBuf.release, 2));
	}

	// Set caps node information
	m_jabberClient->setCapsNode(KOPETE_CAPS_NODE);
	m_jabberClient->setCapsVersion(kapp->aboutData()->version());
	
	// Set Disco Identity information
	DiscoItem::Identity identity;
	identity.category = "client";
	identity.type = "pc";
	identity.name = "Kopete";
	m_jabberClient->setDiscoIdentity(identity);

	//BEGIN TIMEZONE INFORMATION
	//
	// Set timezone information (code from Psi)
	// Copyright (C) 2001-2003  Justin Karneges
	//
	time_t x;
	time(&x);
	char str[256];
	char fmt[32];
	int timezoneOffset;
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

	kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "Determined timezone " << timezoneString << " with UTC offset " << timezoneOffset << " hours." << endl;

	m_jabberClient->setTimeZone ( timezoneString, timezoneOffset );

	kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "Connecting to Jabber server " << server() << ":" << port() << endl;

	setPresence( XMPP::Status ("connecting", "", 0, true) );

	switch ( m_jabberClient->connect ( XMPP::Jid ( accountId () + QString("/") + resource () ), password ) )
	{
		case JabberClient::NoTLS:
			// no SSL support, at the connecting stage this means the problem is client-side
			KMessageBox::queuedMessageBox(Kopete::UI::Global::mainWidget (), KMessageBox::Error,
								i18n ("SSL support could not be initialized for account %1. This is most likely because the QCA TLS plugin is not installed on your system.").
								arg(myself()->contactId()),
								i18n ("Jabber SSL Error"));
			break;
	
		case JabberClient::Ok:
		default:
			// everything alright!

			break;
	}

}

void JabberAccount::slotClientDebugMessage ( const QString &msg )
{

	kdDebug (JABBER_DEBUG_PROTOCOL) << k_funcinfo << msg << endl;

}

bool JabberAccount::handleTLSWarning ( JabberClient *jabberClient, int warning )
{
	QString validityString, code;

	QString server = jabberClient->jid().domain ();
	QString accountId = jabberClient->jid().bare ();

	switch ( warning )
	{
		case QCA::TLS::NoCert:
			validityString = i18n("No certificate was presented.");
			code = "NoCert";
			break;
		case QCA::TLS::HostMismatch:
			validityString = i18n("The host name does not match the one in the certificate.");
			code = "HostMismatch";
			break;
		case QCA::TLS::Rejected:
			validityString = i18n("The Certificate Authority rejected the certificate.");
			code = "Rejected";
			break;
		case QCA::TLS::Untrusted:
			// FIXME: write better error message here
			validityString = i18n("The certificate is untrusted.");
			code = "Untrusted";
			break;
		case QCA::TLS::SignatureFailed:
			validityString = i18n("The signature is invalid.");
			code = "SignatureFailed";
			break;
		case QCA::TLS::InvalidCA:
			validityString = i18n("The Certificate Authority is invalid.");
			code = "InvalidCA";
			break;
		case QCA::TLS::InvalidPurpose:
			// FIXME: write better error  message here
			validityString = i18n("Invalid certificate purpose.");
			code = "InvalidPurpose";
			break;
		case QCA::TLS::SelfSigned:
			validityString = i18n("The certificate is self-signed.");
			code = "SelfSigned";
			break;
		case QCA::TLS::Revoked:
			validityString = i18n("The certificate has been revoked.");
			code = "Revoked";
			break;
		case QCA::TLS::PathLengthExceeded:
			validityString = i18n("Maximum certificate chain length was exceeded.");
			code = "PathLengthExceeded";
			break;
		case QCA::TLS::Expired:
			validityString = i18n("The certificate has expired.");
			code = "Expired";
			break;
		case QCA::TLS::Unknown:
		default:
			validityString = i18n("An unknown error occurred trying to validate the certificate.");
			code = "Unknown";
			break;
		}

	return ( KMessageBox::warningContinueCancel ( Kopete::UI::Global::mainWidget (),
						  i18n("<qt><p>The certificate of server %1 could not be validated for account %2: %3</p><p>Do you want to continue?</p></qt>").
						  arg(server, accountId, validityString),
						  i18n("Jabber Connection Certificate Problem"),
						  KStdGuiItem::cont(),
						  QString("KopeteTLSWarning") + server + code) == KMessageBox::Continue );

}

void JabberAccount::slotHandleTLSWarning ( int validityResult )
{
	kdDebug ( JABBER_DEBUG_GLOBAL ) << k_funcinfo << "Handling TLS warning..." << endl;

	if ( handleTLSWarning ( m_jabberClient, validityResult ) )
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
	kdDebug ( JABBER_DEBUG_GLOBAL ) << k_funcinfo << "Handling client error..." << endl;

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
	kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "Connected to Jabber server." << endl;
	
#ifdef SUPPORT_JINGLE
	if(!m_voiceCaller)
	{
		kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Creating Jingle Voice caller..." << endl;
		m_voiceCaller = new JingleVoiceCaller( this );
		QObject::connect(m_voiceCaller,SIGNAL(incoming(const Jid&)),this,SLOT(slotIncomingVoiceCall( const Jid& )));
		m_voiceCaller->initialize();
	}
	


#if 0
	if(!m_jingleSessionManager)
	{
		kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Creating Jingle Session Manager..." << endl;
		m_jingleSessionManager = new JingleSessionManager( this );
		QObject::connect(m_jingleSessionManager, SIGNAL(incomingSession(const QString &, JingleSession *)), this, SLOT(slotIncomingJingleSession(const QString &, JingleSession *)));
	}
#endif

	// Set caps extensions
	m_jabberClient->client()->addExtension("voice-v1", Features(QString("http://www.google.com/xmpp/protocol/voice/v1")));
#endif

	kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "Requesting roster..." << endl;
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
	kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "Setting initial presence..." << endl;
	setPresence ( m_initialPresence );

}

void JabberAccount::slotIncomingFileTransfer ()
{

	// delegate the work to a file transfer object
	new JabberFileTransfer ( this, client()->fileTransferManager()->takeIncoming () );

}

void JabberAccount::setOnlineStatus( const Kopete::OnlineStatus& status  , const QString &reason)
{
	XMPP::Status xmppStatus = m_protocol->kosToStatus( status, reason);

	if( status.status() == Kopete::OnlineStatus::Offline )
	{
			xmppStatus.setIsAvailable( false );
			kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "CROSS YOUR FINGERS! THIS IS GONNA BE WILD" << endl;
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

void JabberAccount::disconnect ( Kopete::Account::DisconnectReason reason )
{
	kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "disconnect() called" << endl;

	if (isConnected ())
	{
		kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "Still connected, closing connection..." << endl;
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
	kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "Disconnected." << endl;

	disconnected ( reason );
}

void JabberAccount::disconnect( Kopete::Account::DisconnectReason reason, XMPP::Status &status )
{
    kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "disconnect( reason, status ) called" << endl;
    
	if (isConnected ())
	{
		kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "Still connected, closing connection..." << endl;
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
	kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "Disconnected." << endl;

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
	kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "Disconnected from Jabber server." << endl;

	/*
	 * We should delete the JabberClient instance here,
	 * but timers etc prevent us from doing so. Iris does
	 * not like to be deleted from a slot.
	 */

	/* It seems that we don't get offline notifications when going offline
	 * with the protocol, so clear all resources manually. */
	resourcePool()->clear();

}

void JabberAccount::handleStreamError (int streamError, int streamCondition, int connectorCode, const QString &server, Kopete::Account::DisconnectReason &errorClass)
{
	QString errorText;
	QString errorCondition;

	errorClass = Kopete::Account::InvalidHost;

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
					errorCondition = i18n("Generic stream error (sorry, I do not have a more-detailed reason)");
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

			errorText = i18n("There was an error in the protocol stream: %1").arg(errorCondition);
			break;

		case XMPP::ClientStream::ErrConnection:
			switch(connectorCode)
			{
 				case KNetwork::KSocketBase::LookupFailure:
					errorClass = Kopete::Account::InvalidHost;
					errorCondition = i18n("Host not found.");
					break;
				case KNetwork::KSocketBase::AddressInUse:
					errorCondition = i18n("Address is already in use.");
					break;
				case KNetwork::KSocketBase::AlreadyCreated:
					errorCondition = i18n("Cannot recreate the socket.");
					break;
				case KNetwork::KSocketBase::AlreadyBound:
					errorCondition = i18n("Cannot bind the socket again.");
					break;
				case KNetwork::KSocketBase::AlreadyConnected:
					errorCondition = i18n("Socket is already connected.");
					break;
				case KNetwork::KSocketBase::NotConnected:
					errorCondition = i18n("Socket is not connected.");
					break;
				case KNetwork::KSocketBase::NotBound:
					errorCondition = i18n("Socket is not bound.");
					break;
				case KNetwork::KSocketBase::NotCreated:
					errorCondition = i18n("Socket has not been created.");
					break;
				case KNetwork::KSocketBase::WouldBlock:
					errorCondition = i18n("Socket operation would block. You should not see this error, please use \"Report Bug\" from the Help menu.");
					break;
				case KNetwork::KSocketBase::ConnectionRefused:
					errorCondition = i18n("Connection refused.");
					break;
				case KNetwork::KSocketBase::ConnectionTimedOut:
					errorCondition = i18n("Connection timed out.");
					break;
				case KNetwork::KSocketBase::InProgress:
					errorCondition = i18n("Connection attempt already in progress.");
					break;
				case KNetwork::KSocketBase::NetFailure:
					errorCondition = i18n("Network failure.");
					break;
				case KNetwork::KSocketBase::NotSupported:
					errorCondition = i18n("Operation is not supported.");
					break;
				case KNetwork::KSocketBase::Timeout:
					errorCondition = i18n("Socket timed out.");
					break;
				default:
					errorClass = Kopete::Account::ConnectionReset;
					//errorCondition = i18n("Sorry, something unexpected happened that I do not know more about.");
					break;
			}
			if(!errorCondition.isEmpty())
				errorText = i18n("There was a connection error: %1").arg(errorCondition);
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

			errorText = i18n("There was a negotiation error: %1").arg(errorCondition);
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

			errorText = i18n("There was a Transport Layer Security (TLS) error: %1").arg(errorCondition);
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

			errorText = i18n("There was an error authenticating with the server: %1").arg(errorCondition);
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

			errorText = i18n("There was an error in the security layer: %1").arg(errorCondition);
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

			errorText = i18n("Could not bind a resource: %1").arg(errorCondition);
			break;

		default:
			errorText = i18n("Unknown error.");
			break;
	}

	/*
	 * This mustn't be queued as otherwise the reconnection
	 * API will attempt to reconnect, queueing another
	 * error until memory is exhausted.
	 */
	if(!errorText.isEmpty())
		KMessageBox::error (Kopete::UI::Global::mainWidget (),
						errorText,
						i18n("Connection problem with Jabber server %1").arg(server));


}

void JabberAccount::slotCSError ( int error )
{
	kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Error in stream signalled." << endl;

	if ( ( error == XMPP::ClientStream::ErrAuth )
		&& ( client()->clientStream()->errorCondition () == XMPP::ClientStream::NotAuthorized ) )
	{
		kdDebug ( JABBER_DEBUG_GLOBAL ) << k_funcinfo << "Incorrect password, retrying." << endl;
		disconnect(Kopete::Account::BadPassword);
	}
	else
	{
		Kopete::Account::DisconnectReason errorClass =  Kopete::Account::Unknown;

		kdDebug ( JABBER_DEBUG_GLOBAL ) << k_funcinfo << "Disconnecting." << endl;

		// display message to user
		if(!m_removing) //when removing the account, connection errors are normal.
			handleStreamError (error, client()->clientStream()->errorCondition (), client()->clientConnector()->errorCode (), server (), errorClass);

		disconnect ( errorClass );
		
		/*	slotCSDisconnected  will not be called*/
		resourcePool()->clear();
	}

}

/* Set presence (usually called by dialog widget). */
void JabberAccount::setPresence ( const XMPP::Status &status )
{
	kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Status: " << status.show () << ", Reason: " << status.status () << endl;

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
	newStatus.setPriority ( configGroup()->readNumEntry ( "Priority", 5 ) );

	XMPP::Jid jid ( myself()->contactId () );
	XMPP::Resource newResource ( resource (), newStatus );

	// update our resource in the resource pool
	resourcePool()->addResource ( jid, newResource );

	// make sure that we only consider our own resource locally
	resourcePool()->lockToResource ( jid, newResource );

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
			kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Sending new presence to the server." << endl;

			XMPP::JT_Presence * task = new XMPP::JT_Presence ( client()->rootTask ());

			task->pres ( newStatus );
			task->go ( true );
		}
		else
		{
			kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "We were not connected, presence update aborted." << endl;
		}
	}

}

void JabberAccount::slotSendRaw ()
{
	/* Check if we're connected. */
	if ( !isConnected () )
	{
		errorConnectFirst ();
		return;
	}

	new dlgJabberSendRaw ( client (), Kopete::UI::Global::mainWidget());

}

void JabberAccount::slotSubscription (const XMPP::Jid & jid, const QString & type)
{
	kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << jid.full () << ", " << type << endl;

	if (type == "subscribe")
	{
		/*
		 * A user wants to subscribe to our presence.
		 */
		kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << jid.full () << " is asking for authorization to subscribe." << endl;

		// Is the user already in our contact list?
		JabberBaseContact *contact = contactPool()->findExactMatch( jid );
		Kopete::MetaContact *metaContact=0L;
		if(contact)
			metaContact=contact->metaContact();
		
		int hideFlags=Kopete::UI::ContactAddedNotifyDialog::InfoButton;
		if( metaContact && !metaContact->isTemporary() )
			hideFlags |= Kopete::UI::ContactAddedNotifyDialog::AddCheckBox | Kopete::UI::ContactAddedNotifyDialog::AddGroupBox ;
		
		Kopete::UI::ContactAddedNotifyDialog *dialog=
				new Kopete::UI::ContactAddedNotifyDialog( jid.full() ,QString::null,this, hideFlags );
		QObject::connect(dialog,SIGNAL(applyClicked(const QString&)),
						this,SLOT(slotContactAddedNotifyDialogClosed(const QString& )));
		dialog->show();
	}
	else if (type == "unsubscribed")
	{
		/*
		 * Someone else removed our authorization to see them.
		 */
		kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << jid.full() << " revoked our presence authorization" << endl;

		XMPP::JT_Roster *task;

		switch (KMessageBox::warningYesNo (Kopete::UI::Global::mainWidget(),
								  i18n
								  ("The Jabber user %1 removed %2's subscription to them. "
								   "This account will no longer be able to view their online/offline status. "
								   "Do you want to delete the contact?").
										  arg (jid.full(), 1).arg (accountId(), 2), i18n ("Notification"), KStdGuiItem::del(), i18n("Keep")))
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

void JabberAccount::slotContactAddedNotifyDialogClosed( const QString & contactid )
{	// the dialog that asked the authorisation is closed. (it was shown in slotSubscrition)
	
	XMPP::JT_Presence *task;
	XMPP::Jid jid(contactid);

	const Kopete::UI::ContactAddedNotifyDialog *dialog =
			dynamic_cast<const Kopete::UI::ContactAddedNotifyDialog *>(sender());
	if(!dialog || !isConnected())
		return;

	if ( dialog->authorized() )
	{
		/*
		* Authorize user.
		*/

		task = new XMPP::JT_Presence ( client()->rootTask () );
		task->sub ( jid, "subscribed" );
		task->go ( true );
	}
	else
	{
		/*
		* Reject subscription.
		*/
		task = new XMPP::JT_Presence ( client()->rootTask () );
		task->sub ( jid, "unsubscribed" );
		task->go ( true );
	}


	if(dialog->added())
	{
		Kopete::MetaContact *parentContact=dialog->addContact();
		if(parentContact)
		{
			QStringList groupNames;
			Kopete::GroupList groupList = parentContact->groups();
			for(Kopete::Group *group = groupList.first(); group; group = groupList.next())
				groupNames += group->displayName();

			XMPP::RosterItem item;
//			XMPP::Jid jid ( contactId );

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

	kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "New roster item " << item.jid().full () << " (Subscription: " << item.subscription().toString () << ")" << endl;

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
				metaContact->addToGroup (Kopete::ContactList::self ()->findGroup (*it));
	
			// put it onto contact list
			Kopete::ContactList::self ()->addMetaContact ( metaContact );
		}
		else
		{
			metaContact=c->metaContact();
			//TODO: syncronize groups
		}

		/*
		* Add / update the contact in our pool. In case the contact is already there,
		* it will be updated. In case the contact is not in the meta contact yet, it
		* will be added to it.
		* The "dirty" flag is false here, because we just received the contact from
		* the server's roster. As such, it is now a synchronized entry.
		*/
		JabberContact *contact = contactPool()->addContact ( item, metaContact, false );

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
	else if(c)  //we don't need to add it, and it is in the contactlist
	{
		Kopete::MetaContact *metaContact=c->metaContact();
		if(metaContact->isTemporary())
			return;
		kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << c->contactId() << 
				" is on the contactlist while it shouldn't.  we are removing it.  - " << c << endl;
		delete c;
		if(metaContact->contacts().isEmpty())
			Kopete::ContactList::self()->removeMetaContact( metaContact );
	}

}

void JabberAccount::slotContactDeleted (const XMPP::RosterItem & item)
{
	kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "Deleting contact " << item.jid().full () << endl;

	// since the contact instance will get deleted here, the GUI should be updated
	contactPool()->removeContact ( item.jid () );

}

void JabberAccount::slotReceivedMessage (const XMPP::Message & message)
{
	kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "New message from " << message.from().full () << endl;

	JabberBaseContact *contactFrom;

	if ( message.type() == "groupchat" )
	{
		// this is a group chat message, forward it to the group contact
		// (the one without resource name)
		XMPP::Jid jid ( message.from().userHost () );

		// try to locate an exact match in our pool first
		contactFrom = contactPool()->findExactMatch ( jid );

		/**
		 * If there was no exact match, something is really messed up.
		 * We can't receive group chat messages from rooms that we are
		 * not a member of and if the room contact vanished somehow,
		 * we're in deep trouble.
		 */
		if ( !contactFrom )
		{
			kdDebug ( JABBER_DEBUG_GLOBAL ) << k_funcinfo << "WARNING: Received a groupchat message but couldn't find room contact. Ignoring message." << endl;
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
			XMPP::Jid jid ( message.from().userHost () );

			// the contact is not in our pool, add it as a temporary contact
			kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << jid.full () << " is unknown to us, creating temporary contact." << endl;

			Kopete::MetaContact *metaContact = new Kopete::MetaContact ();

			metaContact->setTemporary (true);

			contactFrom = contactPool()->addContact ( XMPP::RosterItem ( jid ), metaContact, false );

			Kopete::ContactList::self ()->addMetaContact (metaContact);
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
	kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "Joined group chat " << jid.full () << endl;

	// Create new meta contact that holds the group chat contact.
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
	resourcePool()->addResource ( XMPP::Jid ( jid.userHost () ), XMPP::Resource ( jid.resource () ) );

	// lock the room to our own status
	resourcePool()->lockToResource ( XMPP::Jid ( jid.userHost () ), jid.resource () );
	
	m_bookmarks->insertGroupChat(jid);	
}

void JabberAccount::slotGroupChatLeft (const XMPP::Jid & jid)
{
	kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo "Left groupchat " << jid.full () << endl;
	
	// remove group contact from list
	Kopete::Contact *contact = 
			Kopete::ContactList::self()->findContact( protocol()->pluginId() , accountId() , jid.userHost() );

	if ( contact )
	{
		Kopete::MetaContact *metaContact= contact->metaContact();
		if( metaContact && metaContact->isTemporary() )
			Kopete::ContactList::self()->removeMetaContact ( metaContact );
		else
			contact->deleteLater();
	}

	// now remove it from our pool, which should clean up all subcontacts as well
	contactPool()->removeContact ( XMPP::Jid ( jid.userHost () ) );
	
}

void JabberAccount::slotGroupChatPresence (const XMPP::Jid & jid, const XMPP::Status & status)
{
	kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "Received groupchat presence for room " << jid.full () << endl;

	// fetch room contact (the one without resource)
	JabberGroupContact *groupContact = dynamic_cast<JabberGroupContact *>( contactPool()->findExactMatch ( XMPP::Jid ( jid.userHost () ) ) );

	if ( !groupContact )
	{
		kdDebug ( JABBER_DEBUG_GLOBAL ) << k_funcinfo << "WARNING: Groupchat presence signalled, but we don't have a room contact?" << endl;
		return;
	}

	if ( !status.isAvailable () )
	{
		kdDebug ( JABBER_DEBUG_GLOBAL ) << k_funcinfo << jid.full () << " has become unavailable, removing from room" << endl;

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
	kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "Group chat error - room " << jid.full () << " had error " << error << " (" << reason << ")" << endl;

	switch (error)
	{
	case JabberClient::InvalidPasswordForMUC:
		{
			QCString password;
 			int result = KPasswordDialog::getPassword(password, i18n("A password is required to join the room %1.").arg(jid.node()));
			if (result == KPasswordDialog::Accepted)
				m_jabberClient->joinGroupChat(jid.domain(), jid.node(), jid.resource(), password);
		}
		break;

	case JabberClient::NicknameConflict:
		{
			bool ok;
			QString nickname = KInputDialog::getText(i18n("Error trying to join %1 : nickname %2 is already in use").arg(jid.node(), jid.resource()),
									i18n("Give your nickname"),
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
									i18n ("You can't join the room %1 because you were banned").arg(jid.node()),
									i18n ("Jabber Group Chat") );
		break;

	case JabberClient::MaxUsersReachedForThisMuc:
		KMessageBox::queuedMessageBox ( Kopete::UI::Global::mainWidget (),
									KMessageBox::Error,
									i18n ("You can't join the room %1 because the maximum users has been reached").arg(jid.node()),
									i18n ("Jabber Group Chat") );
		break;

	default:
		{
		QString detailedReason = reason.isEmpty () ? i18n ( "No reason given by the server" ) : reason;

		KMessageBox::queuedMessageBox ( Kopete::UI::Global::mainWidget (),
									KMessageBox::Error,
									i18n ("There was an error processing your request for group chat %1. (Reason: %2, Code %3)").arg ( jid.full (), detailedReason, QString::number ( error ) ),
									i18n ("Jabber Group Chat") );
		}
	}
}

void JabberAccount::slotResourceAvailable (const XMPP::Jid & jid, const XMPP::Resource & resource)
{

	kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "New resource available for " << jid.full() << endl;

	resourcePool()->addResource ( jid, resource );

}

void JabberAccount::slotResourceUnavailable (const XMPP::Jid & jid, const XMPP::Resource & resource)
{

	kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "Resource now unavailable for " << jid.full () << endl;

	resourcePool()->removeResource ( jid, resource );

}

void JabberAccount::slotEditVCard ()
{
	static_cast<JabberContact *>( myself() )->slotUserInfo ();
}

void JabberAccount::slotGlobalIdentityChanged (const QString &key, const QVariant &value)
{
	// Check if this account is excluded from Global Identity.
	if( !configGroup()->readBoolEntry("ExcludeGlobalIdentity", false) )
	{
		JabberContact *jabberMyself = static_cast<JabberContact *>( myself() );
		if( key == Kopete::Global::Properties::self()->nickName().key() )
		{
			QString oldNick = jabberMyself->property( protocol()->propNickName ).value().toString();
			QString newNick = value.toString();
		
			if( newNick != oldNick && isConnected() )
			{
				jabberMyself->setProperty( protocol()->propNickName, newNick );
				jabberMyself->slotSendVCard();
			}
		}
		if( key == Kopete::Global::Properties::self()->photo().key() )
		{
			if( isConnected() )
			{
				jabberMyself->setPhoto( value.toString() );
				jabberMyself->slotSendVCard();
			}
		}
	}
}

const QString JabberAccount::resource () const
{

	return configGroup()->readEntry ( "Resource", "Kopete" );

}

const QString JabberAccount::server () const
{

	return configGroup()->readEntry ( "Server" );

}

const int JabberAccount::port () const
{

	return configGroup()->readNumEntry ( "Port", 5222 );

}

void JabberAccount::slotGetServices ()
{
	dlgJabberServices *dialog = new dlgJabberServices (this);

	dialog->show ();
	dialog->raise ();
}

void JabberAccount::slotIncomingVoiceCall( const Jid &jid )
{
	kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << endl;
#ifdef SUPPORT_JINGLE
	if(voiceCaller())
	{
		kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Showing voice dialog." << endl;
		JingleVoiceSessionDialog *voiceDialog = new JingleVoiceSessionDialog( jid, voiceCaller() );
		voiceDialog->show();
	}
#else
	Q_UNUSED(jid);
#endif
}

// void JabberAccount::slotIncomingJingleSession( const QString &sessionType, JingleSession *session )
// {
// #ifdef SUPPORT_JINGLE
// 	if(sessionType == "http://www.google.com/session/phone")
// 	{
// 		QString from = ((XMPP::Jid)session->peers().first()).full();
// 		//KMessageBox::queuedMessageBox( Kopete::UI::Global::mainWidget(), KMessageBox::Information, QString("Received a voice session invitation from %1.").arg(from) );
// 		JingleVoiceSessionDialog *voiceDialog = new JingleVoiceSessionDialog( static_cast<JingleVoiceSession*>(session) );
// 		voiceDialog->show();
// 	}
// #else
// 	Q_UNUSED( sessionType );
// 	Q_UNUSED( session );
// #endif
// }


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
				   			    "If you unregister, all your contact list may be removed on the server,"
							    "And you will never be able to connect to this account with any client").arg( accountLabel() ),
					i18n("Unregister"),
					KGuiItem(i18n( "Remove and Unregister" ), "editdelete"),
					KGuiItem(i18n( "Remove from kopete only"), "edittrash"),
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
			QObject::connect ( task, SIGNAL ( finished () ), this, SLOT ( slotUnregisterFinished ) );
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
			i18n ("An error occured when trying to remove the account:\n%1").arg(task->statusString()),
			i18n ("Jabber Account Unregistration"));
		m_removing=false;
		return;
	}
	if(m_removing)  //it may be because this is now the timer.
		Kopete::AccountManager::self()->removeAccount( this ); //this will delete this
}





#include "jabberaccount.moc"

// vim: set noet ts=4 sts=4 sw=4:
