
/***************************************************************************
                   jabberaccount.cpp  -  core Jabber account class
                             -------------------
    begin                : Sat Mär 8 2003
    copyright            : (C) 2003 by Till Gerken <till@tantalo.net>
							Based on JabberProtocol by Daniel Stone <dstone@kde.org>
							and Till Gerken <till@tantalo.net>.

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

#include "jabberaccount.h"

#include <qapplication.h>
#include <qcursor.h>
#include <qmap.h>
#include <qpixmap.h>
#include <qregexp.h>
#include <qstring.h>
#include <qtimer.h>

#include <kdebug.h>
#include <kgenericfactory.h>
#include <kmessagebox.h>
#include <kiconloader.h>
#include <kpopupmenu.h>
#include <kstandarddirs.h>
#include <klineeditdlg.h>
#include <kapplication.h>
#include <kaboutdata.h>

#include "kopetecontact.h"
#include "kopetecontactlist.h"
#include "kopetemetacontact.h"
#include "kopetemessagemanager.h"
#include "kopeteaway.h"
#include "kopeteprotocol.h"
#include "kopeteaccount.h"
#include "kopeteplugin.h"
#include "kopeteuiglobal.h"
#include "kopetegroup.h"
#include "addcontactpage.h"
#include "dlgjabbersendraw.h"
#include "dlgjabberservices.h"
#include "dlgjabberchatjoin.h"
#include "jabberaddcontactpage.h"
#include "jabbergroupchat.h"

#include "im.h"
#include "xmpp.h"
#include "qca.h"

#include <sys/utsname.h>


JabberAccount::JabberAccount (JabberProtocol * parent, const QString & accountId, const char *name):KopeteAccount (parent, accountId, name)
{
	kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Instantiating new account " << accountId << endl;

	mProtocol = parent;

	setMyself( new JabberContact (accountId, accountId.section('@', 0, 0), QStringList (), this, 0L) );

	jabberClient = 0L;
	jabberClientStream = 0L;
	jabberClientConnector = 0L;
	jabberTLS = 0L;
	jabberTLSHandler = 0L;

	awayDialog = new JabberAwayDialog(this);

	initialPresence = protocol()->JabberKOSOnline;

}

JabberAccount::~JabberAccount ()
{
	disconnect ();

	cleanup ();

	delete awayDialog;
}

void JabberAccount::cleanup ()
{

	delete jabberClient;
	delete jabberClientStream;
	delete jabberClientConnector;
	delete jabberTLSHandler;
	delete jabberTLS;

	jabberClient = 0L;
	jabberClientStream = 0L;
	jabberClientConnector = 0L;
	jabberTLS = 0L;
	jabberTLSHandler = 0L;

}

KActionMenu *JabberAccount::actionMenu ()
{
	KActionMenu *m_actionMenu = new KActionMenu( accountId(), myself()->onlineStatus().iconFor(this),  this );

	m_actionMenu->popupMenu()->insertTitle(
		myself()->onlineStatus().iconFor(myself()),
		i18n("%2 <%1>")
#if QT_VERSION < 0x030200
			.arg(accountId()).arg(myself()->displayName()));
#else
			.arg(accountId(), myself()->displayName()));
#endif

	m_actionMenu->insert(new KAction (mProtocol->JabberKOSOnline.caption(),
		mProtocol->JabberKOSOnline.iconFor(this), 0, this, SLOT (slotGoOnline ()), this,
		"actionJabberConnect"));

	m_actionMenu->insert(new KAction (mProtocol->JabberKOSChatty.caption(),
		mProtocol->JabberKOSChatty.iconFor(this), 0, this, SLOT (slotGoChatty ()), this,
		"actionJabberChatty"));

	m_actionMenu->insert(new KAction (mProtocol->JabberKOSAway.caption(),
		mProtocol->JabberKOSAway.iconFor(this), 0, this, SLOT (slotGoAway ()), this,
		"actionJabberAway"));

	m_actionMenu->insert(new KAction (mProtocol->JabberKOSXA.caption(),
		mProtocol->JabberKOSXA.iconFor(this), 0, this, SLOT (slotGoXA ()), this,
		"actionJabberXA"));

	m_actionMenu->insert(new KAction (mProtocol->JabberKOSDND.caption(),
		mProtocol->JabberKOSDND.iconFor(this), 0, this, SLOT (slotGoDND ()), this,
		"actionJabberDND"));

	m_actionMenu->insert(new KAction (mProtocol->JabberKOSInvisible.caption(),
		mProtocol->JabberKOSInvisible.iconFor(this), 0, this, SLOT (slotGoInvisible ()), this,
		"actionJabberInvisible"));

	m_actionMenu->insert(new KAction (mProtocol->JabberKOSOffline.caption(),
		mProtocol->JabberKOSOffline.iconFor(this), 0, this,SLOT (slotGoOffline ()), this,
		"actionJabberDisconnect"));

	m_actionMenu->popupMenu()->insertSeparator();
	m_actionMenu->insert(new KAction (i18n ("Join Groupchat..."), "jabber_group", 0,
		this, SLOT (slotJoinNewChat ()), this, "actionJoinChat"));
	m_actionMenu->popupMenu()->insertSeparator();
	m_actionMenu->insert(new KAction (i18n ("Services..."), "jabber_serv_on", 0,
		this, SLOT (slotGetServices ()), this, "actionJabberServices"));
	m_actionMenu->insert(new KAction (i18n ("Send Raw Packet to Server..."), "mail_new", 0,
		this, SLOT (slotSendRaw ()), this, "actionJabberSendRaw"));
	m_actionMenu->insert(new KAction (i18n ("Edit User Info..."), "identity", 0,
		this, SLOT (slotEditVCard ()), this, "actionEditVCard"));

	return m_actionMenu;
}

/*
 *  Add a contact to Meta Contact
 */
bool JabberAccount::addContactToMetaContact (const QString & contactId, const QString & displayName, KopeteMetaContact * metaContact)
{

	/* collect all group names */
	QStringList groupNames;
	QPtrList<KopeteGroup> groupList = metaContact->groups();
	for(KopeteGroup *group = groupList.first(); group; group = groupList.next())
		groupNames += group->displayName();


	JabberContact *jc = createContact(contactId, displayName, groupNames, metaContact);

	return (jc != 0);

}

void JabberAccount::errorConnectFirst ()
{
	KMessageBox::queuedMessageBox(Kopete::UI::Global::mainWidget (), KMessageBox::Error,  i18n ("Please connect first."), i18n ("Error"));
}


void JabberAccount::connect ()
{
	kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "called" << endl;

	/* Don't do anything if we are already connected. */
	if (isConnected ())
		return;

	/* This is dirty but has to be done: if a previous connection attempt
	 * failed, psi doesn't handle recovering too well. we are not allowed to
	 * call close in the slotConnected() slot since it causes a crash, so we
	 * have to delete the psi backend altogether here for safety if it still
	 * exists.
	 * FIXME: verify that this problem still exists with Iris.
	 */
	if (jabberClient)
	{
		jabberClient->close ();

		cleanup ();
	}

	/*
	 * Setup authentication layer
	 */
	bool trySSL = false;
	if (pluginData (protocol (), "UseSSL") == "true")
	{
		bool sslPossible = QCA::isSupported(QCA::CAP_TLS);

		if (!sslPossible)
		{
			KMessageBox::queuedMessageBox(Kopete::UI::Global::mainWidget (), KMessageBox::Error,
								i18n ("SSL is not supported. This is most likely because the QCA TLS plugin is not installed on your system."), i18n ("SSL Error"));
			return;
		}
		else
		{
			trySSL = true;

			jabberTLS = new QCA::TLS;
			jabberTLSHandler = new XMPP::QCATLSHandler(jabberTLS);

			{
				using namespace XMPP;
				QObject::connect(jabberTLSHandler, SIGNAL(tlsHandshaken()), this, SLOT(slotTLSHandshaken()));
			}
		}
	}

	/*
	 * Setup proxy layer
	 */
	QString proxyTypeStr = pluginData (protocol (), "ProxyType");
	int proxyType = XMPP::AdvancedConnector::Proxy::None;

	if (proxyTypeStr == QString ("HTTPS"))
	{
		proxyType = XMPP::AdvancedConnector::Proxy::HttpConnect;
	}
	else
	{
		if (proxyTypeStr == QString ("SOCKS"))
		{
			proxyType = XMPP::AdvancedConnector::Proxy::Socks;
		}
		else
		{
			if (proxyTypeStr == QString ("HTTPPoll"))
			{
				proxyType = XMPP::AdvancedConnector::Proxy::HttpPoll;
			}
		}
	}

	XMPP::AdvancedConnector::Proxy proxy;

	switch(proxyType)
	{
		case XMPP::AdvancedConnector::Proxy::None:
			// no proxy
			break;

		case XMPP::AdvancedConnector::Proxy::HttpConnect:
			// use HTTP
			proxy.setHttpConnect( pluginData (protocol (), "ProxyName"), pluginData (protocol (), "ProxyPort").toInt () );
			break;

		case XMPP::AdvancedConnector::Proxy::HttpPoll:
			// use HTTP polling
			proxy.setHttpPoll ( pluginData (protocol (), "ProxyName"), pluginData (protocol (), "ProxyPort").toInt (), pluginData (protocol (), "ProxyUrl") );
			proxy.setPollInterval (2);
			break;

		case XMPP::AdvancedConnector::Proxy::Socks:
			// use socks
			proxy.setSocks( pluginData (protocol (), "ProxyName"), pluginData (protocol (), "ProxyPort").toInt () );
			break;
	}

	if (pluginData (protocol (), "ProxyAuth") == QString::fromLatin1 ("true"))
		proxy.setUserPass (pluginData (protocol (), "ProxyUser"), pluginData (protocol (), "ProxyPass"));

	/*
	 * Instantiate connector, responsible for dealing with the socket.
	 * This class makes use of the proxy layer created above.
	 */
	jabberClientConnector = new XMPP::AdvancedConnector;
	jabberClientConnector->setOptHostPort (server (), port ());
	jabberClientConnector->setProxy(proxy);
	jabberClientConnector->setOptSSL(trySSL);

	/*
	 * Instantiate client stream which handles the network communication by referring
	 * to a connector (proxying etc.) and a TLS handler (security layer)
	 */
	jabberClientStream = new XMPP::ClientStream(jabberClientConnector, jabberTLSHandler);

	{
		using namespace XMPP;
		QObject::connect (jabberClientStream, SIGNAL (needAuthParams(bool, bool, bool)),
				  this, SLOT (slotCSNeedAuthParams (bool, bool, bool)));
		QObject::connect (jabberClientStream, SIGNAL (authenticated()),
				  this, SLOT (slotCSAuthenticated ()));
		QObject::connect (jabberClientStream, SIGNAL (connectionClosed ()),
				  this, SLOT (slotCSDisconnected ()));
		QObject::connect (jabberClientStream, SIGNAL (delayedCloseFinished ()),
				  this, SLOT (slotCSDisconnected ()));
		QObject::connect (jabberClientStream, SIGNAL (warning (int)),
				  this, SLOT (slotCSWarning (int)));
		QObject::connect (jabberClientStream, SIGNAL (error (int)),
				  this, SLOT (slotCSError (int)));
	}

	/*
	 * FIXME: This is required until we fully support XMPP 1.0
	 *        Upon switching to XMPP 1.0, add full TLS capabilities
	 *        with fallback (setOptProbe()) and remove the call below.
	 */
	jabberClientStream->setOldOnly(true);

	/*
	 * Initiate anti-idle timer (will be triggered every 55 seconds).
	 */
	jabberClientStream->setNoopTime(55000);

	/*
	 * Allow plaintext password authentication or not?
	 */
	jabberClientStream->setAllowPlain(pluginData (protocol (), "AllowPlainTextPassword") == QString::fromLatin1("true"));

	/*
	 * Setup client layer.
	 */
	jabberClient = new XMPP::Client (this);

	/* This should only be done here to connect the signals, otherwise it is a
	 * bad idea.
	 */
	{
		using namespace XMPP;
		QObject::connect (jabberClient, SIGNAL (subscription (const Jid &, const QString &)), this,
						SLOT (slotSubscription (const Jid &, const QString &)));
		QObject::connect (jabberClient, SIGNAL (rosterItemAdded (const RosterItem &)), this, SLOT (slotNewContact (const RosterItem &)));
		QObject::connect (jabberClient, SIGNAL (rosterItemUpdated (const RosterItem &)), this, SLOT (slotContactUpdated (const RosterItem &)));
		QObject::connect (jabberClient, SIGNAL (rosterItemRemoved (const RosterItem &)), this, SLOT (slotContactDeleted (const RosterItem &)));
		QObject::connect (jabberClient, SIGNAL (resourceAvailable (const Jid &, const Resource &)), this,
						SLOT (slotResourceAvailable (const Jid &, const Resource &)));
		QObject::connect (jabberClient, SIGNAL (resourceUnavailable (const Jid &, const Resource &)), this,
						SLOT (slotResourceUnavailable (const Jid &, const Resource &)));
		QObject::connect (jabberClient, SIGNAL (messageReceived (const Message &)), this, SLOT (slotReceivedMessage (const Message &)));
		QObject::connect (jabberClient, SIGNAL (groupChatJoined (const Jid &)), this, SLOT (slotGroupChatJoined (const Jid &)));
		QObject::connect (jabberClient, SIGNAL (groupChatLeft (const Jid &)), this, SLOT (slotGroupChatLeft (const Jid &)));
		QObject::connect (jabberClient, SIGNAL (groupChatPresence (const Jid &, const Status &)), this,
						SLOT (slotGroupChatPresence (const Jid &, const Status &)));
		QObject::connect (jabberClient, SIGNAL (groupChatError (const Jid &, int, const QString &)), this,
						SLOT (slotGroupChatError (const Jid &, int, const QString &)));
		QObject::connect (jabberClient, SIGNAL (debugText (const QString &)), this, SLOT (slotPsiDebug (const QString &)));
	}

	struct utsname utsBuf;

	uname (&utsBuf);

	jabberClient->setClientName ("Kopete");
	jabberClient->setClientVersion (kapp->aboutData ()->version ());
	jabberClient->setOSName (QString ("%1 %2").arg (utsBuf.sysname, 1).arg (utsBuf.release, 2));

	kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "Connecting to Jabber server " << server() << ":" << port() << endl;

	setPresence(protocol()->JabberKOSConnecting, "");

	jabberClient->connectToServer (jabberClientStream,
				       XMPP::Jid(accountId() + QString("/") + pluginData( protocol (), "Resource")),
				       true);

}

void JabberAccount::slotPsiDebug (const QString & _msg)
{
	QString msg = _msg;

	msg = msg.replace( QRegExp( "<password>[^<]*</password>\n" ), "<password>[Filtered]</password>\n" );
	msg = msg.replace( QRegExp( "<digest>[^<]*</digest>\n" ), "<digest>[Filtered]</digest>\n" );

	kdDebug (JABBER_DEBUG_PROTOCOL) << k_funcinfo << "Psi: " << msg << endl;

}

int JabberAccount::handleTLSWarning (int warning, QString server)
{
	QString validityString, code;

	switch(warning)
	{
		case QCA::TLS::NoCert:
			validityString = i18n("No certificate was presented.");
			code = "NoCert";
			break;
		case QCA::TLS::HostMismatch:
			validityString = i18n("The hostname does not match the one in the certificate.");
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
			validityString = i18n("An unknown error occured trying to validate the certificate.");
			code = "Unknown";
			break;
		}

	return KMessageBox::warningContinueCancel(Kopete::UI::Global::mainWidget (),
						  i18n("The server's certificate could not be validated: %1").
						  arg(validityString),
						  i18n("Connection Certificate Problem"),
						  KStdGuiItem::cont(),
						  QString("KopeteTLSWarning") + server + code);

}

void JabberAccount::slotTLSHandshaken ()
{

	kdDebug() << k_funcinfo << "TLS handshake done, testing certificate validity..." << endl;

	int validityResult = jabberTLS->certificateValidityResult ();

	if(validityResult == QCA::TLS::Valid)
	{
		kdDebug() << k_funcinfo << "Certificate is valid, continuing." << endl;

		// valid certificate, continue
		jabberTLSHandler->continueAfterHandshake ();
	}
	else
	{
		kdDebug() << k_funcinfo << "Certificate is not valid, asking user what to do next." << endl;

		// certificate is not valid, query the user
		if(handleTLSWarning (validityResult, server ()) == KMessageBox::Continue)
		{
			jabberTLSHandler->continueAfterHandshake ();
		}
		else
		{
			disconnect ();
		}
	}

}

void JabberAccount::slotCSNeedAuthParams (bool user, bool pass, bool realm)
{

	kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "Sending auth credentials..." << endl;

	XMPP::Jid jid(accountId());

	if(user)
	{
		jabberClientStream->setUsername(jid.node());
	}

	if(pass)
	{
		jabberClientStream->setPassword(password());
	}

	if(realm)
	{
		jabberClientStream->setRealm(jid.domain());
	}

	jabberClientStream->continueAfterParams();

}

void JabberAccount::slotCSAuthenticated ()
{
	kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "Connected to Jabber server." << endl;

	/* slow down the polling interval for HTTP Poll proxies */
	jabberClientConnector->changePollInterval (10);

	/* start the client operation */
	XMPP::Jid jid(accountId());
	jabberClient->start ( jid.domain(), jid.node(), password(), pluginData( protocol (), "Resource") );

	kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "Requesting roster..." << endl;

	/* Request roster. */
	jabberClient->rosterRequest ();

	/* Since we are online now, set initial presence. Don't do this
	 * before the roster request or we will receive presence
	 * information before we have updated our roster with actual
	 * contacts from the server! (libpsi won't forward presence
	 * information in that case either). */
	kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "Setting initial presence..." << endl;
	setPresence (initialPresence, static_cast<JabberContact *>( myself() )->reason ());

}

void JabberAccount::disconnect ()
{
	kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "disconnect() called" << endl;

	if (isConnected ())
	{
		kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "Still connected, closing connection..." << endl;
		/* Tell backend class to disconnect. */
		jabberClient->close ();
	}

	// make sure that the connection animation gets stopped if we're still
	// in the process of connecting
	setPresence (protocol()->JabberKOSOffline, "disconnected");

	/* FIXME:
	 * We should delete the XMPP::Client instance here,
	 * but active timers in psi prevent us from doing so.
	 * (in a failed connection attempt, these timers will
	 * try to access an already deleted object).
	 * Instead, the instance will lurk until the next
	 * connection attempt.
	 */
	kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "Disconnected." << endl;

}

void JabberAccount::slotConnect ()
{
	connect ();
}

void JabberAccount::slotDisconnect ()
{
	disconnect ();
}

void JabberAccount::slotCSDisconnected ()
{
	kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "Disconnected from Jabber server." << endl;

	/* FIXME:
	 * We should delete the XMPP::Client instance here,
	 * but timers etc prevent us from doing so. (Psi does
	 * not like to be deleted from a slot).
	 */

	/* It seems that we don't get offline notifications when going offline
	 * with the protocol, so update all contacts manually. */
	for (QDictIterator < KopeteContact > it (contacts ()); it.current (); ++it)
		static_cast < JabberContact * >(*it)->slotUpdatePresence (protocol()->JabberKOSOffline, "disconnected");

}

void JabberAccount::slotCSWarning (int warning)
{

	/*
	 * FIXME: these warnings don't mean anything to the user just yet,
	 *        so we simply ignore them (pre XMPP 1.0 warning, no TLS warning).
	 *
	switch(warning)
	{
		case XMPP::ClientStream::WarnOldVersion:
		case XMPP::ClientStream::WarnNoTLS:
	}
	*/

	jabberClientStream->continueAfterWarning ();

}

void JabberAccount::handleStreamError (int streamError, int streamCondition, int connectorCode, QString server)
{
	QString errorText;
	QString errorCondition;

	/*
	 * Display error to user.
	 * FIXME: for unknown errors, maybe add error codes?
	 */
	switch(streamError)
	{
		case XMPP::Stream::ErrParse:
			errorText = i18n("Malformed packet received.");
			break;

		case XMPP::Stream::ErrProtocol:
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
					errorCondition = i18n("The connection timed out.");
					break;
				case XMPP::Stream::InternalServerError:
					errorCondition = i18n("Internal server error.");
					break;
				case XMPP::Stream::InvalidFrom:
					errorCondition = i18n("Packet received from an invalid address.");
					break;
				case XMPP::Stream::InvalidXml:
					errorCondition = i18n("Malformed packet received.");
					break;
				case XMPP::Stream::PolicyViolation:
					// FIXME: need a better error message here
					errorCondition = i18n("Policy violation.");
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
				case XMPP::AdvancedConnector::ErrConnectionRefused:
					errorCondition = i18n("Connection refused.");
					break;
				case XMPP::AdvancedConnector::ErrHostNotFound:
					errorCondition = i18n("Host not found.");
					break;
				case XMPP::AdvancedConnector::ErrProxyConnect:
					errorCondition = i18n("Could not connect to proxy server.");
					break;
				case XMPP::AdvancedConnector::ErrProxyNeg:
					errorCondition = i18n("Could not negotiate with the proxy server.");
					break;
				case XMPP::AdvancedConnector::ErrProxyAuth:
					errorCondition = i18n("Could not authenticate with the proxy server.");
					break;
				default:
					errorCondition = i18n("Sorry, something unexpected happened that I do not know more about.");
					break;
			}

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

	KMessageBox::queuedMessageBox (Kopete::UI::Global::mainWidget (),
								   KMessageBox::Error,
								   errorText,
								   i18n("Connection problem with Jabber server %1").arg(server));


}

void JabberAccount::slotCSError (int error)
{
	kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Error in stream signalled, disconnecting." << endl;

	// display message to user
	handleStreamError (error, jabberClientStream->errorCondition (), jabberClientConnector->errorCode (), server ());

	disconnect ();

	// manually force the slot to be called since in case of an error,
	// libpsi will most likely be confused and not emit signals anymore
	slotCSDisconnected();

}

/* Set presence (usually called by dialog widget). */
void JabberAccount::setPresence (const KopeteOnlineStatus & status, const QString & reason, int priority)
{

	/*
	 * If we are in the process of connecting, only update our local presence
	 * and don't send anything across the wire.
	 */
	if(status == protocol()->JabberKOSConnecting)
	{
		kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Setting new presence locally (from or to connecting)." << endl;

		static_cast<JabberContact *>( myself() )->slotUpdatePresence (status, reason);
	}
	else
	{
		/*
		 * If we are already connected and changing our presence or if we are connecting
		 * and set our initial presence, send new presence packet to the server.
		 * Sorry for the ugly if() below but the requirement for certain timings to send out presence
		 * packets and the silly implementation of KopeteAccount::isOnline() leave no other choice.
		 */
		if (isConnected())
		{
			kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Sending new presence to the server." << endl;

			XMPP::Status presence;

			presence.setPriority (priority);
			presence.setStatus (reason);
			presence.setIsAvailable (true);

			if (status == protocol()->JabberKOSOnline)
				presence.setShow ("");
			else if (status == protocol()->JabberKOSChatty)
				presence.setShow ("chat");
			else if (status == protocol()->JabberKOSAway)
				presence.setShow ("away");
			else if (status == protocol()->JabberKOSXA)
				presence.setShow ("xa");
			else if (status == protocol()->JabberKOSDND)
				presence.setShow ("dnd");
			else if (status == protocol()->JabberKOSInvisible)
				presence.setIsInvisible (true);
			else if (status != protocol()->JabberKOSOffline)
			{
				kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "Unknown presence status, " << "ignoring (status == " << status.description () << ")" << endl;
				return;
			}

			kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "Updating presence to show(" << presence.show () << "), status(" << presence.status () << "), with reason \"" << reason << "\"" << endl;

			static_cast<JabberContact *>( myself() )->slotUpdatePresence (status, reason);

			XMPP::JT_Presence * task = new XMPP::JT_Presence (jabberClient->rootTask ());

			task->pres (presence);
			task->go (true);
		}
		else
		{
			kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "We were not connected, presence update aborted." << endl;
		}
	}

}

void JabberAccount::setAway (bool away, const QString & reason)
{
	kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "Setting away mode: " << away << endl;

	if(away)
		setPresence (protocol()->JabberKOSAway, reason);
	else
		setPresence (protocol()->JabberKOSOnline, reason);

}

void JabberAccount::setAvailable (void)
{
	kdDebug (JABBER_DEBUG_GLOBAL) << "[JabberAccount] Coming back from away mode." << endl;
	slotGoOnline ();
}

void JabberAccount::slotGoOnline ()
{
	kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "called." << endl;

	if (!isConnected ())
	{
		/* We are not connected yet, so connect now. */
		initialPresence = protocol()->JabberKOSOnline;
		connect ();
	}
	else
	{
		setPresence (protocol()->JabberKOSOnline, "");
	}

}

void JabberAccount::slotGoOffline ()
{
	kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "called." << endl;

	disconnect ();
}

void JabberAccount::slotGoChatty ()
{
	kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "called." << endl;

	if (!isConnected ())
	{
		/* We are not connected yet, so connect now. */
		initialPresence = protocol()->JabberKOSChatty;
		connect ();
	}
	else
	{
		setPresence (protocol()->JabberKOSChatty, "");
	}

}

void JabberAccount::slotGoAway ()
{
	kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "called." << endl;

	if (!isConnected ())
	{
		/* We are not connected yet, so connect now. */
		initialPresence = protocol()->JabberKOSAway;
		connect ();
	}
	else
	{
		awayDialog->show(JabberProtocol::JabberAway);
	}

}

void JabberAccount::slotGoXA ()
{
	kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "called." << endl;

	if (!isConnected ())
	{
		/* We are not connected yet, so connect now. */
		initialPresence = protocol()->JabberKOSXA;
		connect ();
	}
	else
	{
		awayDialog->show(JabberProtocol::JabberXA);
	}

}

void JabberAccount::slotGoDND ()
{
	kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "called." << endl;

	if (!isConnected ())
	{
		/* We are not connected yet, so connect now. */
		initialPresence = protocol()->JabberKOSDND;
		connect ();
	}
	else
	{
		awayDialog->show(JabberProtocol::JabberDND);
	}

}

void JabberAccount::slotGoInvisible ()
{
	kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "called." << endl;

	if (!isConnected ())
	{
		/* We are not connected yet, so connect now. */
		initialPresence = protocol()->JabberKOSInvisible;
		connect ();
	}
	else
	{
		setPresence (protocol()->JabberKOSInvisible, "");
	}

}

void JabberAccount::slotSendRaw ()
{
	/* Check if we're connected. */
	if (!isConnected ())
	{
		errorConnectFirst ();
		return;
	}

	new dlgJabberSendRaw (jabberClient, Kopete::UI::Global::mainWidget());

}

void JabberAccount::subscribe (const XMPP::Jid & jid)
{

	if (!isConnected ())
	{
		errorConnectFirst ();
		return;
	}

	XMPP::JT_Presence * task = new XMPP::JT_Presence (jabberClient->rootTask ());

	task->sub (jid, "subscribe");
	task->go (true);
}

void JabberAccount::subscribed (const XMPP::Jid & jid)
{
	if (!isConnected ())
	{
		errorConnectFirst ();
		return;
	}

	XMPP::JT_Presence * task = new XMPP::JT_Presence (jabberClient->rootTask ());

	task->sub (jid, "subscribed");
	task->go (true);
}

void JabberAccount::unsubscribed (const XMPP::Jid & jid)
{
	if (!isConnected ())
	{
		errorConnectFirst ();
		return;
	}

	XMPP::JT_Presence * task = new XMPP::JT_Presence (jabberClient->rootTask ());

	task->sub (jid, "unsubscribed");
	task->go (true);
}

void JabberAccount::sendPresenceToNode (const KopeteOnlineStatus & pres, const QString & userID)
{

	if (!isConnected ())
	{
		errorConnectFirst ();
		return;
	}

	XMPP::JT_Presence * task = new XMPP::JT_Presence (jabberClient->rootTask ());

	XMPP::Jid jid (userID);
	XMPP::Status status;

	if (pres == protocol()->JabberKOSOnline)
		status.setShow ("");
	else if (pres == protocol()->JabberKOSChatty)
		status.setShow ("chat");
	else if (pres == protocol()->JabberKOSAway)
		status.setShow ("away");
	else if (pres == protocol()->JabberKOSXA)
		status.setShow ("xa");
	else if (pres == protocol()->JabberKOSDND)
		status.setShow ("dnd");
	else if (pres == protocol()->JabberKOSInvisible)
	{
		status.setShow ("away");
		status.setIsInvisible (true);
	}
	else
		status.setShow ("away");

	task->pres (jid, status);
	task->go (true);
}

JabberContact *JabberAccount::createContact (const QString & jid, const QString & alias, const QStringList & groups, KopeteMetaContact * metaContact)
{

	JabberContact *jc = new JabberContact (jid, alias, groups, this, metaContact);

	return jc;

}


void JabberAccount::createAddContact (KopeteMetaContact * mc, const XMPP::RosterItem & item)
{

	if (!mc)
	{
		/*
		 * If no metacontact has been given, try to locate an existing one
		 * that contains a contact with the same ID that we are to create.
		 */
		mc = KopeteContactList::contactList ()->findContact (protocol ()->pluginId (), accountId (), item.jid ().userHost ().lower());

		if (mc)
		{
			/*
			 * A metacontact exists that does contain a contact with the same ID
			 */
			JabberContact *jc = (JabberContact *) mc->findContact (protocol ()->pluginId (),
																   accountId (),
																   item.jid ().userHost ().lower());

			if (jc)
			{
				/*
				 * Since the subcontact exists already, we don't recreate it but
				 * merely update its data according to our parameters.
				 */
				kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "Contact " << item.jid ().userHost () << " already exists, updating" << endl;
				jc->slotUpdateContact (item);
				return;
			}
			else
			{
				/*
				 * If this code is reached, something is severely broken:
				 * A subcontact of a metacontact exists but we are unable to
				 * retrieve it's pointer.
				 */
				kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "****Warning**** : " << item.jid ().userHost () << " already exists, and can be found" << endl;
			}
		}
	}

	/*
	 * If we got here and mc is still NULL, the contact is not
	 * in the contact list yet and we need to create a new metacontact
	 * for it.
	 */
	bool isContactInList = true;
	if (!mc)
	{
		kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "Adding contact " << item.jid ().userHost () << " ..." << endl;

		isContactInList = false;

		mc = new KopeteMetaContact ();
		QStringList groups = item.groups ();

		for (QStringList::Iterator it = groups.begin (); it != groups.end (); ++it)
			mc->addToGroup (KopeteContactList::contactList ()->getGroup (*it));
	}

	/*
	 * At this point, we either found the metacontact or created a new
	 * one. The only thing left to do is to create a new Jabber contact
	 * inside it.
	 */
	QString contactName;

	if (item.name ().isNull () || item.name ().isEmpty ())
		contactName = item.jid ().userHost ();
	else
		contactName = item.name ();

	createContact (item.jid().userHost(), contactName, item.groups(), mc);

	if (!isContactInList)
		KopeteContactList::contactList ()->addMetaContact (mc);

}

void JabberAccount::slotSubscription (const XMPP::Jid & jid, const QString & type)
{
	kdDebug (JABBER_DEBUG_GLOBAL) << "[JabberAccount] slotSubscription(" << jid.userHost () << ", " << type << ");" << endl;

	if (type == "subscribe")
	{
		/* A new user wants to subscribe. */
		kdDebug (JABBER_DEBUG_GLOBAL) << "[JabberAccount] slotSubscription(): " << jid.userHost () << " is asking for authorization to subscribe." << endl;

		switch (KMessageBox::questionYesNoCancel (Kopete::UI::Global::mainWidget (),
												  i18n
												  ("The Jabber user %1 wants to add you to their "
												   "contact list; do you want to authorize them? "
												   "Selecting Cancel will ignore the request.").
												  arg (jid.userHost (), 1), i18n ("Authorize Jabber User?"), i18n ("Authorize"), i18n ("Deny")))
		{
			KopeteMetaContact *mc;

		case KMessageBox::Yes:
			/* Authorize user. */
			subscribed (jid);

			/* Is the user already in our contact list? */
			mc = KopeteContactList::contactList ()->findContact (protocol ()->pluginId (), accountId (), jid.userHost ());

			/* If it is not, ask the user if he wants to subscribe in return. */
			if ((!mc || mc->isTemporary()) && (KMessageBox::questionYesNo (Kopete::UI::Global::mainWidget (),
													i18n
													("Do you want to add %1 to your contact "
													 "list in return?").arg (jid.userHost (), 1), i18n ("Add Jabber User?")) == KMessageBox::Yes))
				/* Subscribe to user's presence. */
				subscribe (jid);
			break;

		case KMessageBox::No:
			/* Reject subscription. */
				unsubscribed (jid);

			break;

		case KMessageBox::Cancel:
			/* Leave the user in the dark. */
			break;
		}

	}
	else if (type == "unsubscribed")
	{
		/* Someone else removed us from their roster. */
		kdDebug (JABBER_DEBUG_GLOBAL) << "[JabberAccount] " << jid.userHost () << " deleted auth!" << endl;

		XMPP::JT_Roster * task = new XMPP::JT_Roster (jabberClient->rootTask ());
		switch (KMessageBox::warningYesNo (Kopete::UI::Global::mainWidget(),
								  i18n
								  ("The Jabber user %1 removed %2's subscription to them."
								   "This account will no longer be able to view their online/offline status."
								   "\nDo you want to delete the contact?").
								  arg (jid.userHost (), 1).arg (accountId(), 2), i18n ("Notification")))
		{

		case KMessageBox::Yes:
			task->remove (jid);
			task->go (true);
			break;

		default:
			/* We want to leave the contact in our contact list, so do nothing. */
			break;

		}
	}
}

void JabberAccount::slotNewContact (const XMPP::RosterItem & item)
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
	 * a roster item here. FIXME: To be done is uniform
	 * check of the ask() value to see if we are waiting
	 * for authorization ("subscribe")
	 */

	QString debugStr = "[JabberAccount] New Contact " + item.jid ().userHost () + " (Subscription::";

	switch (item.subscription ().type ())
	{
	case XMPP::Subscription::Both:	// both sides can see the contact
		debugStr += "Both | <->";
		break;

	case XMPP::Subscription::From:	// he can see us
		debugStr += "From | <--";
		break;

	case XMPP::Subscription::To:	// we can see him
		debugStr += "To | -->";
		break;

	case XMPP::Subscription::None:	// waiting for authorization
		debugStr += "None | ---";
		break;
	}

	debugStr += ") " + item.ask ();

	kdDebug (JABBER_DEBUG_GLOBAL) << debugStr << endl;

	/* Add the contact to the GUI. */
	createAddContact (0L, item);
}

void JabberAccount::slotContactDeleted (const XMPP::RosterItem & item)
{
	kdDebug (JABBER_DEBUG_GLOBAL) << "[JabberAccount] Deleting contact " << item.jid ().userHost () << endl;

	if (!contacts ()[item.jid().userHost().lower()])
	{
		kdDebug (JABBER_DEBUG_GLOBAL) << "[JabberProtocl] WARNING: slotContactDeleted() " << "was asked to delete a non-existing contact." << endl;
		return;
	}

	JabberContact *jc = static_cast < JabberContact * >(contacts ()[item.jid().userHost().lower()]);

	/* This will also cause the contact to disappear from the metacontact. */
	delete jc;
}

void JabberAccount::slotContactUpdated (const XMPP::RosterItem & item)
{
	kdDebug (JABBER_DEBUG_GLOBAL) << "[JabberAccount] Status update for " << item.jid ().userHost () << endl;

	/* Sanity check. */
	if (!contacts ()[item.jid().userHost().lower()])
	{
		kdDebug (JABBER_DEBUG_GLOBAL) << "[JabberAccount] WARNING: slotContactUpdated() " << "was requested to update a non-existing contact." << endl;
		return;
	}
	// update the contact data
	static_cast < JabberContact * >(contacts ()[item.jid().userHost().lower()])->slotUpdateContact (item);
}

void JabberAccount::slotReceivedMessage (const XMPP::Message & message)
{
	QString userHost;
	JabberContact *contactFrom;

	userHost = message.from ().userHost ();
	contactFrom = static_cast < JabberContact * >(contacts ()[userHost.lower()]);

	if (userHost.isEmpty ())
	{
		/* If the sender field is empty, it is a server message.
		 *
		 * When I wrote it, this made sense, but should it be displayed
		 * in a KopeteEmailWindow now? -DS */
		kdDebug (JABBER_DEBUG_GLOBAL) << "[JabberAccount] New server message for us!" << endl;

		KMessageBox::information (Kopete::UI::Global::mainWidget (), message.body (), i18n ("Jabber: Server Message"));
	}
	else
	{
		kdDebug (JABBER_DEBUG_GLOBAL) << "[JabberAccount] New message from '" << userHost << "'" << endl;

		/* See if the contact is actually in our roster. */
		if (!contactFrom)
		{
			/* So, either it's a group chat, or we're not subscribed
			 * to them. Either way. */
			kdDebug (JABBER_DEBUG_GLOBAL) << "[JabberAccount] Message received from an " << "unknown contact, creating temporary contact." << endl;

			KopeteMetaContact *metaContact = new KopeteMetaContact ();

			metaContact->setTemporary (true);

			contactFrom = createContact (userHost, userHost, QStringList (), metaContact);

			KopeteContactList::contactList ()->addMetaContact (metaContact);
		}

		/* Pass the message on to the contact. */
		contactFrom->slotReceivedMessage (message);
	}

}

void JabberAccount::slotJoinNewChat ()
{
	if (!isConnected ())
	{
		errorConnectFirst ();
		return;
	}

	dlgJabberChatJoin *dlg = new dlgJabberChatJoin (this, Kopete::UI::Global::mainWidget ());

	dlg->show ();
	dlg->raise ();
}

void JabberAccount::slotGroupChatJoined (const XMPP::Jid & jid)
{
	kdDebug (JABBER_DEBUG_GLOBAL) << "[JabberAccount] Joined group chat " << jid.full () << endl;

	/* Create new meta contact that holds the group chat contact. */
	KopeteMetaContact *mc = new KopeteMetaContact ();

	mc->setTemporary (true);

	/* The group chat object basically works like a JabberContact. */
	JabberGroupChat *groupChat = new JabberGroupChat (jid, QStringList (), this, mc);

	/* Add the group chat class to the meta contact. */
	mc->addContact (groupChat);

	KopeteContactList::contactList ()->addMetaContact (mc);
}

void JabberAccount::slotGroupChatLeft (const XMPP::Jid & jid)
{
	kdDebug (JABBER_DEBUG_GLOBAL) << "[JabberAccount] Left groupchat " << jid.full () << endl;
	delete static_cast < JabberGroupChat * >(contacts ()[jid.userHost().lower()]);
}

void JabberAccount::slotGroupChatPresence (const XMPP::Jid & jid, const XMPP::Status & status)
{
	kdDebug (JABBER_DEBUG_GLOBAL) << "[JabberAccount] Received groupchat presence for room " << jid.full () << endl;
	static_cast < JabberGroupChat * >(contacts ()[jid.userHost().lower()])->updatePresence (jid, status);
}

void JabberAccount::slotGroupChatError (const XMPP::Jid & jid, int error, const QString & reason)
{
	/* FIXME: Present this to the user, damnit! */
	kdDebug (JABBER_DEBUG_GLOBAL) << "[JabberAccount] Group chat error - room " << jid.userHost () << " had error " << error << " (" << reason << ")!" << endl;
}

void JabberAccount::slotResourceAvailable (const XMPP::Jid & jid, const XMPP::Resource & resource)
{

	kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "New resource available for " << jid.userHost () << endl;

	if (!contacts ()[jid.userHost().lower()])
	{
		kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "Trying to add a resource, but " << "couldn't find an entry for " << jid.userHost () << endl;
		return;
	}

	if(static_cast<JabberContact *>(contacts ()[jid.userHost().lower()]) == myself())
	{
		kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Ignoring resource by other client for ourselves." << endl;
		return;
	}

	static_cast < JabberContact * >(contacts ()[jid.userHost().lower()])->slotResourceAvailable (jid, resource);
}

void JabberAccount::slotResourceUnavailable (const XMPP::Jid & jid, const XMPP::Resource & resource)
{

	kdDebug (JABBER_DEBUG_GLOBAL) << "[JabberAccount] Resource now unavailable for " << jid.userHost () << endl;

	if (!contacts ()[jid.userHost().lower()])
	{
		kdDebug (JABBER_DEBUG_GLOBAL) << "[JabberAccount] Trying to remove a resource, " << "but couldn't find an entry for " << jid.userHost () << endl;
		return;
	}

	if(static_cast<JabberContact *>(contacts ()[jid.userHost().lower()]) == myself())
	{
		kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Ignoring resource by other client for ourselves." << endl;
		return;
	}

	static_cast < JabberContact * >(contacts ()[jid.userHost().lower()])->slotResourceUnavailable (jid, resource);

}

void JabberAccount::slotEditVCard ()
{
	static_cast<JabberContact *>( myself() )->slotEditVCard ();
}

bool JabberAccount::addContact( const QString &contactId, const QString &displayName,
							   KopeteMetaContact *parentContact, const KopeteAccount::AddMode mode, const QString &groupName,
							   bool isTemporary)
{
	XMPP::RosterItem item;

	item.setJid(XMPP::Jid(contactId));
	item.setName(contactId);
	item.setGroups(groupName);

	//createAddContact(parentContact, item);

	// add the new contact to our roster.
	XMPP::JT_Roster * rosterTask = new XMPP::JT_Roster(jabberClient->rootTask());

	rosterTask->set(item.jid(), item.name(), item.groups());
	rosterTask->go(true);

	// send a subscription request.
	subscribe(item.jid());

	return KopeteAccount::addContact(contactId, displayName, parentContact, mode, groupName, isTemporary);

}

void JabberAccount::removeContact (const XMPP::RosterItem & item)
{
	if (!isConnected ())
	{
		errorConnectFirst ();
		return;
	}

	XMPP::JT_Roster * rosterTask = new XMPP::JT_Roster (jabberClient->rootTask ());

	rosterTask->remove (item.jid ());
	rosterTask->go (true);
}

const QString JabberAccount::resource () const
{

	return pluginData (protocol (), "Resource");

}

const QString JabberAccount::server () const
{

	return pluginData (protocol (), QString::fromLatin1 ("Server"));

}

const int JabberAccount::port () const
{

	return pluginData (protocol (), "Port").toInt ();

}

XMPP::Client *JabberAccount::client()
{

	return jabberClient;

}

void JabberAccount::slotGetServices ()
{
	dlgJabberServices *dialog = new dlgJabberServices (this);

	dialog->show ();
	dialog->raise ();
}

#include "jabberaccount.moc"

// vim: set noet ts=4 sts=4 sw=4:

