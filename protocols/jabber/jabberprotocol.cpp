/*
    jabberprotocol.cpp  -  Base class for the Kopete Jabber protocol

    Copyright (c) 2002 by Daniel Stone <dstone@kde.org>
    Copyright (c) 2002 by Till Gerken <till@tantalo.net>

    Kopete    (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include <kdebug.h>
#include <kgenericfactory.h>
#include <kconfig.h>
#include <kmessagebox.h>
#include <kiconloader.h>
#include <kpopupmenu.h>
#include <kstandarddirs.h>
#include <klineeditdlg.h>

#include <qapplication.h>
#include <qcursor.h>
#include <qmap.h>
#include <qtimer.h>
#include <qpixmap.h>
#include <qstringlist.h>

#include "client.h"
#include "stream.h"
#include "tasks.h"
#include "types.h"
#include "vcard.h"

#include <sys/utsname.h>

#include "../kopete/kopete.h"
#include "kopetecontact.h"
#include "kopetecontactlist.h"
#include "kopetemetacontact.h"
#include "kopetemessagemanager.h"
#include "kopeteaway.h"
#include "kopeteprotocol.h"
#include "kopeteplugin.h"
#include "addcontactpage.h"
#include "jabbercontact.h"
#include "jabberprefs.h"
#include "dlgjabberstatus.h"
#include "dlgjabbersendraw.h"
#include "dlgjabberservices.h"
#include "dlgjabberchatjoin.h"
#include "jabberaddcontactpage.h"
#include "jabbermap.h"
#include "jabbergroupchat.h"
#include "jabberprotocol.h"

JabberProtocol *JabberProtocol::protocolInstance = 0;

K_EXPORT_COMPONENT_FACTORY(kopete_jabber, KGenericFactory<JabberProtocol>);

JabberProtocol::JabberProtocol(QObject *parent, QString name, QStringList)
	: KopeteProtocol(parent, name),
	JabberOnline(KopeteOnlineStatus::Online, 25, this, 0, "jabber_online", i18n("Go O&nline" ), i18n("Online")),
	JabberChatty(KopeteOnlineStatus::Online, 20, this, 1, "jabber_chatty", i18n("Set F&ree to Chat" ), i18n("Free to Chat")),
	JabberAway(KopeteOnlineStatus::Away, 25, this, 2, "jabber_away", i18n("Set A&way"), i18n("Away")),
	JabberXA(KopeteOnlineStatus::Away, 20, this, 3, "jabber_away", i18n("Set E&xtended Away"), i18n("Extended Away")),
	JabberDND(KopeteOnlineStatus::Away, 15, this, 4, "jabber_na", i18n("Set &Do not Disturb"), i18n("Do not Disturb")),
	JabberOffline(KopeteOnlineStatus::Offline, 20, this, 5, "jabber_offline", i18n("Go O&ffline"), i18n("Offline")),
	JabberInvisible(KopeteOnlineStatus::Online, 5, this, 6, "jabber_offline", i18n("Set I&nvisible"), i18n("Invisible"))
{
	kdDebug(JABBER_DEBUG_GLOBAL) << "[JabberProtocol] Loading ..." << endl;

	/* This is meant to be a singleton, so we will check if we have
	 * been loaded before. */
	if (protocolInstance) {
		kdDebug(JABBER_DEBUG_GLOBAL) << "[JabberProtocol] Warning: Protocol already "
				             << "loaded, not initializing again." << endl;
		return;
	}

	protocolInstance = this;

	jabberClient = 0L;
	registerFlag = 0;

	/* This is deleted in the destructor. */
	reasonDialog = 0L;
	/* This is not yet implemented. */
	sendRawDialog = 0L;

	myContact = 0L;

	initialPresence = JabberOnline;

	preferences = new JabberPreferences("jabber_protocol", this);
	QObject::connect(preferences, SIGNAL(saved()), this,
			 SLOT(slotSettingsChanged()));

	// read the Jabber ID from Kopete's configuration
	KGlobal::config()->setGroup("Jabber");

	// setup actions
	initActions();

	// read remaining settings from configuration file
	slotSettingsChanged();
	addAddressBookField( "messaging/xmpp", KopetePlugin::MakeIndexField );
}

JabberProtocol::~JabberProtocol() {
	disconnect();

	if(jabberClient) {
		delete jabberClient;
		jabberClient = 0L;
	}

	/* Kick the SSL library. */
	Jabber::Stream::unloadSSL();

	/* make sure that the next attempt to load Jabber
	 * re-initializes the protocol class. */
	protocolInstance = 0L;

	/* Delete the send raw dialog. */
	delete sendRawDialog;
}

void JabberProtocol::errorConnectFirst() {
	KMessageBox::error(qApp->mainWidget(), i18n("Please connect first"),
			   i18n("Error"));
}

KActionMenu *JabberProtocol::protocolActions() {
	KActionMenu *protocolMenu = new KActionMenu();
	for (JabberAccount *tmpAccount = accounts.first(); tmpAccount != accounts.last(); tmpAccount = accounts.next())
		protocolMenu->insert(tmpAccount->actionMenu());
	return protocolMenu;
}

void JabberProtocol::initActions() {
	// initialize icon that sits in Kopete's status bar
	setStatusIcon("jabber_offline");

	KGlobal::config()->setGroup("Jabber");

	// make sure we load the SSL library if required
	if(KGlobal::config()->readBoolEntry("UseSSL", "0"))
	{
		// try to load QSSL library needed by libpsi
		// FIXME: this is ugly because it uses fixed dirs!
		// should check ldconfig or something
		QStringList dirs;

		dirs += "/usr/lib";
		dirs += "/usr/local/lib";
		dirs += QStringList::split(":", QString(getenv("LD_LIBRARY_PATH")));

		Jabber::Stream::loadSSL(dirs);
	}

	// if we need to connect on startup, do it now
	if (KGlobal::config()->readBoolEntry( "AutoConnect", false ) )
		QTimer::singleShot( 0, this, SLOT( connect() ) );

	// create a contact instance for self
	QString userId = KGlobal::config()->readEntry("UserID", "");
	QString server = KGlobal::config()->readEntry("Server", "jabber.org");
	myContact = new JabberContact(QString("%1@%2").arg(userId, 1).arg(server, 2),
				      userId, QStringList(i18n("Unknown")),
				      this, 0L, QString::null);

}

void JabberProtocol::connect()
{
	kdDebug(JABBER_DEBUG_GLOBAL) << "[JabberProtocol] connect()" << endl;

	/* Don't do anything if we are already connected. */
	if (isConnected())
		return;

	/* this is dirty but has to be done:
	 * if a previous connection attempt failed, psi
	 * doesn't handle recovering too well. we are not
	 * allowed to call close in the slotConnected() slot
	 * since it causes a crash, so we have to delete the
	 * psi backend altogether here for safety if it still
	 * exists.
	 *
	 * yo, right, but use c-style comments for multi-line, eh? -ds */
	if(jabberClient) {
		jabberClient->close();
		delete jabberClient;
		jabberClient = 0L;
	}

	/* instantiate new Psi backend class for handling the protocol
	 * and setup slots and signals */
	if(!jabberClient)
	{
		jabberClient = new Jabber::Client(this);

		// this should only be done here to connect the signals,
		// otherwise it is a bad idea
		using namespace Jabber;

		QObject::connect(jabberClient, SIGNAL(handshaken()),
						this, SLOT(slotHandshaken()));
		QObject::connect(jabberClient,
						SIGNAL(authFinished(bool, int, const QString &)),
						this, SLOT(slotConnected(bool, int, const QString &)));

		QObject::connect(jabberClient, SIGNAL(closeFinished()),
						this, SLOT(slotDisconnected()));

		QObject::connect(jabberClient,
						SIGNAL(subscription(const Jid &, const QString &)),
						this, SLOT(slotSubscription(const Jid &, const QString &)));

		QObject::connect(jabberClient,
						SIGNAL(rosterItemAdded(const RosterItem &)),
						this, SLOT(slotNewContact(const RosterItem &)));

		QObject::connect(jabberClient,
						SIGNAL(rosterItemUpdated(const RosterItem &)),
						this, SLOT(slotContactUpdated(const RosterItem &)));

		QObject::connect(jabberClient,
						SIGNAL(rosterItemRemoved(const RosterItem &)),
						this, SLOT(slotContactDeleted(const RosterItem &)));

		QObject::connect(jabberClient,
						SIGNAL(resourceAvailable(const Jid &, const Resource &)),
						this, SLOT(slotResourceAvailable(const Jid &, const Resource &)));

		QObject::connect(jabberClient,
						SIGNAL(resourceUnavailable(const Jid &, const Resource &)),
						this, SLOT(slotResourceUnavailable(const Jid &, const Resource &)));

		QObject::connect(jabberClient,
						SIGNAL(messageReceived(const Message &)),
						this, SLOT(slotReceivedMessage(const Message &)));

		QObject::connect(jabberClient, SIGNAL(groupChatJoined(const Jid &)),
						this, SLOT(slotGroupChatJoined(const Jid &)));

		QObject::connect(jabberClient, SIGNAL(groupChatLeft(const Jid &)),
						this, SLOT(slotGroupChatLeft(const Jid &)));

		QObject::connect(jabberClient,
						SIGNAL(groupChatPresence(const Jid &, const Status &)),
						this, SLOT(slotGroupChatPresence(const Jid &, const Status &)));

		QObject::connect(jabberClient,
						SIGNAL(groupChatError(const Jid &, int, const QString &)),
						this, SLOT(slotGroupChatError(const Jid &, int, const QString &)));

		QObject::connect(jabberClient, SIGNAL(error(const StreamError &)),
						this, SLOT(slotError(const StreamError &)));

		QObject::connect(jabberClient, SIGNAL(debugText(const QString &)),
						this, SLOT(slotPsiDebug(const QString &)));

		utsname utsBuf;
		uname(&utsBuf);

		jabberClient->setClientName("Kopete Jabber Plugin");
		jabberClient->setClientVersion(KOPETE_VERSION);
		jabberClient->setOSName(QString("%1 %2").arg(utsBuf.sysname,
								1).arg(utsBuf.release,2));

	}

	// read all configuration data from the configuration file
	KGlobal::config()->setGroup("Jabber");
	QString userId = KGlobal::config()->readEntry("UserID", "");
	QString server = KGlobal::config()->readEntry("Server", "jabber.org");
	int port = KGlobal::config()->readNumEntry("Port", 5222);

	if(userId.isEmpty())
	{
		int r=KMessageBox::warningContinueCancel(qApp->mainWidget(),
			i18n("<qt>You have not yet specified your Jabber username. "
							"You can specify your Jabber settings in the Kopete "
							"configuration dialog<br>"
							"Do you want to configure Jabber now?</qt>" ),
			i18n( "Jabber Plugin Not Configured Yet" ),
			KGuiItem( i18n( "C&onfigure..." ), "configure" ) );
		if(r!=KMessageBox::Cancel)
		{
			preferences->activate();
		}
		return;
	}


	// check if we are capable of using SSL if requested
	if(KGlobal::config()->readBoolEntry("UseSSL", "0"))
	{
		bool sslPossible = jabberClient->setSSLEnabled(true);

		if(!sslPossible)
		{
			KMessageBox::error(qApp->mainWidget(),
				i18n("SSL is not supported. This is most likely "
				"because the QSSL library could not be found."),
			i18n("SSL Error"));
			return;
		}
	}

	// parse proxy settings
	QString proxyTypeStr = KGlobal::config()->readEntry("ProxyType", "None");
	int proxyType = Jabber::StreamProxy::None;

	if(proxyTypeStr == QString("HTTPS"))
		proxyType = Jabber::StreamProxy::HTTPS;
	else
		if(proxyTypeStr == QString("SOCKS4"))
				proxyType = Jabber::StreamProxy::SOCKS4;
		else
				if(proxyTypeStr == QString("SOCKS5"))
						proxyType = Jabber::StreamProxy::SOCKS5;

	Jabber::StreamProxy proxy(proxyType,
					KGlobal::config()->readEntry("ProxyName", ""),
					KGlobal::config()->readNumEntry("ProxyPort", 8080));

	proxy.setUseAuth(KGlobal::config()->readBoolEntry("ProxyAuth", false));
	proxy.setUser(KGlobal::config()->readEntry("ProxyUser", ""));
	proxy.setPass(KGlobal::config()->readEntry("ProxyPass", ""));

	jabberClient->setProxy(proxy);

	delete myContact;

	// create a contact instance for self
	myContact = new JabberContact(
					QString("%1@%2").arg(userId, 1).arg(server, 2),
					userId, QStringList(i18n("Unknown")),
					this, 0L, QString::null);

	// set the title according to the new changes
	actionStatusMenu->popupMenu()->changeTitle(
					menuTitleId, userId + "@" + server );

	kdDebug(JABBER_DEBUG_GLOBAL) << "[JabberProtocol] Connecting to Jabber server "
								<< server << ":" << port << endl;
	kdDebug(JABBER_DEBUG_GLOBAL) << "                 with UserID " << userId << endl;

	// now connect
	jabberClient->connectToHost(server, port);

	// play movie to indicate connection attempt
	setStatusIcon("jabber_connecting");

}

void JabberProtocol::slotPsiDebug(const QString &msg)
{
	kdDebug(JABBER_DEBUG_PROTOCOL) << "[JabberProtocol] Psi: " << msg << endl;
}

void JabberProtocol::slotHandshaken()
{

	kdDebug(JABBER_DEBUG_GLOBAL) << "[JabberProtocol] Performing login..." << endl;

	if(registerFlag)
	{
		Jabber::JT_Register *task =
				new Jabber::JT_Register(jabberClient->rootTask());

		QObject::connect(task, SIGNAL(finished()),
						this, SLOT(slotRegisterUserDone()));

		task->reg(KGlobal::config()->readEntry("UserID", ""),
					KGlobal::config()->readEntry("Password", ""));

		task->go(true);
	}
	else
	{
		KGlobal::config()->setGroup("Jabber");

		if(KGlobal::config()->readEntry("AuthType", "digest")
						== QString("digest"))
		{
			jabberClient->authDigest(
							KGlobal::config()->readEntry("UserID", ""),
							KGlobal::config()->readEntry("Password", ""),
							KGlobal::config()->readEntry("Resource", "Kopete"));
		}
		else
		{
			jabberClient->authPlain(
							KGlobal::config()->readEntry("UserID", ""),
							KGlobal::config()->readEntry("Password", ""),
							KGlobal::config()->readEntry("Resource", "Kopete"));
		}
	}

}

void JabberProtocol::slotConnected(bool success, int statusCode,
				const QString &statusString)
{

	if(success)
	{
		kdDebug(JABBER_DEBUG_GLOBAL) << "[JabberProtocol] Connected to Jabber server."
									<< endl;

		setStatusIcon("jabber_online");

		// request roster
		jabberClient->rosterRequest();

		// since we are online now, set initial presence
		// don't do this before the roster request or we will receive
		// presence information before we have updated our roster
		// with actual contacts from the server! (libpsi won't forward
		// presence information in that case either)
		setPresence( initialPresence, myContact->reason() );

		// initiate anti-idle timer (will be triggered every 10 seconds)
		jabberClient->setNoopTime(10000);
	}
	else
	{
		kdDebug(JABBER_DEBUG_GLOBAL) << "[JabberProtocol] Connection failed! Status: "
				<< statusCode << ", " << statusString << endl;

		setStatusIcon("jabber_offline");

		KMessageBox::error(qApp->mainWidget(),
				i18n("Connection failed with reason \"%1\"").arg(
				statusString, 1),
				i18n("Connection Failed"));
	}

}


JabberProtocol *JabberProtocol::protocol()
{
	// return current instance
	return protocolInstance;
}

void JabberProtocol::disconnect()
{
	kdDebug(JABBER_DEBUG_GLOBAL) << "[JabberProtocol] disconnect() called" << endl;

	if (isConnected())
	{
		kdDebug(JABBER_DEBUG_GLOBAL) << "[JabberProtocol] Still connected, closing connection..." << endl;

		// tell backend class to disconnect
		jabberClient->close();
	}

	/* FIXME:
	 * We should delete the Jabber::Client instance here,
	 * but active timers in psi prevent us from doing so.
	 * (in a failed connection attempt, these timers will
	 * try to access an already deleted object)
	 * Instead, the instance will lurk until the next
	 * connection attempt.
	 */

	kdDebug(JABBER_DEBUG_GLOBAL) << "[JabberProtocol] Disconnected." << endl;

	setStatusIcon("jabber_offline");

	// it seems that we don't get offline notifications
	// when going offline with the protocol, so update
	// all contacts manually
	QDictIterator<KopeteContact> it( contacts() );
	for ( ; it.current() ; ++it )
	{
		static_cast<JabberContact *>( *it )->slotUpdatePresence( JabberOffline, "" );
	}

}

void JabberProtocol::slotConnect()
{
	connect();
}

void JabberProtocol::slotDisconnect()
{
	disconnect();
}

void JabberProtocol::slotDisconnected()
{
	kdDebug(JABBER_DEBUG_GLOBAL) << "[JabberProtocol] Disconnected from Jabber server."
			<< endl;

	/* FIXME:
	 * We should delete the Jabber::Client instance here,
	 * but timers etc prevent us from doing so. (psi does
	 * not like to be deleted from a slot)
	 */

	setStatusIcon("jabber_offline");
}

void JabberProtocol::slotError(const Jabber::StreamError &error)
{
	// determine type of error
	switch(error.type())
	{
	case Jabber::StreamError::DNS:
			KMessageBox::error(qApp->mainWidget(),
							i18n("DNS error (%1)").arg(error.details(), 1),
							i18n("Error Connecting to Jabber Server"));
			break;

	case Jabber::StreamError::Refused:
			KMessageBox::error(qApp->mainWidget(),
							i18n("Connection refused (%1)").arg(error.details(), 1),
							i18n("Error Connecting to Jabber Server"));
			break;

	case Jabber::StreamError::Timeout:
			KMessageBox::error(qApp->mainWidget(),
							i18n("Timeout (%1)").arg(error.details(), 1),
							i18n("Error Connecting to Jabber Server"));
			break;

	case Jabber::StreamError::Socket:
			KMessageBox::error(qApp->mainWidget(),
							i18n("Socket error (%1)").arg(error.details(), 1),
							i18n("Error Connecting to Jabber Server"));
			break;

	case Jabber::StreamError::Disconnected:
			KMessageBox::error(qApp->mainWidget(),
							i18n("Remote server closed connection (%1)").arg(
											error.details(), 1),
							i18n("Error Connecting to Jabber Server"));
			break;

	case Jabber::StreamError::Handshake:
			KMessageBox::error(qApp->mainWidget(),
							i18n("Handshake failed (%1)").arg(error.details(), 1),
							i18n("Error Connecting to Jabber Server"));
			break;

	case Jabber::StreamError::SSL:
			KMessageBox::error(qApp->mainWidget(),
							i18n("SSL error (%1)").arg(error.details(), 1),
							i18n("Error Connecting to Jabber Server"));
			break;

	case Jabber::StreamError::Proxy:
			KMessageBox::error(qApp->mainWidget(),
							i18n("Proxy error (%1)").arg(error.details(), 1),
							i18n("Error Connecting to Jabber Server"));
			break;

	case Jabber::StreamError::Unknown:
	default:
			KMessageBox::error(qApp->mainWidget(),
							i18n("An unknown error was encountered (%1)").arg(
											error.details(), 1),
							i18n("Error Connecting to Jabber Server"));
			break;
	}

	disconnect();
}

/*
 * Set presence (usually called by dialog widget)
 */
void JabberProtocol::setPresence( const KopeteOnlineStatus &status, const QString &reason, int priority )
{
	if (isConnected())
	{
		Jabber::Status presence;

		presence.setPriority(priority);
		presence.setStatus(reason);
		presence.setIsAvailable(true);

		if( status == JabberOnline )
			presence.setShow("");
		else if( status == JabberChatty )
			presence.setShow("chat");
		else if( status == JabberAway )
			presence.setShow("away");
		else if( status == JabberXA )
			presence.setShow("xa");
		else if( status == JabberDND )
			presence.setShow("dnd");
		else if( status == JabberInvisible )
			presence.setIsInvisible(true);
		else
		{
			kdDebug( JABBER_DEBUG_GLOBAL ) << k_funcinfo << "Unknown presence status, "
											<< "ignoring (status == " << status.description() << ")"
											<< endl;
		}

		kdDebug( JABBER_DEBUG_GLOBAL ) << k_funcinfo << "Updating to \"" << presence.status()
									<< "\" with reason \"" << reason << endl;

		myContact->slotUpdatePresence(status, reason);

		Jabber::JT_Presence *task =
				new Jabber::JT_Presence(jabberClient->rootTask());

		task->pres(presence);
		task->go(true);
	}
}

void JabberProtocol::setAway(void)
{
	kdDebug(JABBER_DEBUG_GLOBAL) << "[JabberProtocol] Setting globally away." << endl;
	setPresence( JabberAway, KopeteAway::getInstance()->message() );
}

void JabberProtocol::setAvailable(void)
{
	slotGoOnline();
}

KopeteContact* JabberProtocol::myself() const
{
	return myContact;
}

void JabberProtocol::deserializeContact( KopeteMetaContact *metaContact,
				const QMap<QString, QString> &serializedData,
				const QMap<QString, QString> & /* addressBookData */ )
{
	kdDebug( JABBER_DEBUG_GLOBAL ) << k_funcinfo << "Deserializing data for metacontact "
										 << metaContact->displayName() << endl;
	new JabberContact( serializedData[ "contactId" ],
			serializedData[ "displayName" ],
			QStringList::split( ',', serializedData[ "groups" ] ),
			this, metaContact, serializedData[ "identityId" ] );

}

AddContactPage *JabberProtocol::createAddContactWidget(QWidget *parent)
{
	return new JabberAddContactPage(this, parent);
}

void JabberProtocol::slotGoOnline()
{
	kdDebug(JABBER_DEBUG_GLOBAL) << "[JabberProtocol] Going online!" << endl;

	if (!isConnected())
	{
		// we are not connected yet, so connect now
		initialPresence = JabberOnline;
		connect();
	}

	setPresence( JabberOnline, "" );

}

void JabberProtocol::slotGoOffline()
{
	kdDebug(JABBER_DEBUG_GLOBAL) << "[JabberProtocol] Going offline." << endl;

	disconnect();
}

void JabberProtocol::slotGoChatty()
{
	kdDebug(JABBER_DEBUG_GLOBAL) << "[JabberProtocol] Setting 'chatty' mode." << endl;

	if (!isConnected())
	{
		// we are not connected yet, so connect now
		initialPresence = JabberChatty;
		connect();
	}

	setPresence( JabberChatty, "" );

}

void JabberProtocol::slotGoAway()
{
	kdDebug(JABBER_DEBUG_GLOBAL) << "[JabberProtocol] Setting away mode." << endl;

	if (!isConnected())
	{
		// we are not connected yet, so connect now
		initialPresence = JabberAway;
		connect();
	}

	// kill old reason dialog if it's still in memory
	if (reasonDialog != 0L)
		delete reasonDialog;

	// TODO Fix this to work with KopeteAwayDialog
	reasonDialog = new dlgJabberStatus( this, JabberAway, qApp->mainWidget() );
}

void JabberProtocol::slotGoXA()
{
	kdDebug(JABBER_DEBUG_GLOBAL) << "[JabberProtocol] Setting extended away mode."
								<< endl;

	if (!isConnected())
	{
		// we are not connected yet, so connect now
		initialPresence = JabberXA;
		connect();
	}

	// kill old reason dialog if it's still in memory
	if (reasonDialog != 0L)
			delete reasonDialog;

	// TODO Fix this to work with KopeteAwayDialog
	reasonDialog = new dlgJabberStatus( this, JabberXA, qApp->mainWidget() );

}

void JabberProtocol::slotGoDND()
{
	kdDebug(JABBER_DEBUG_GLOBAL) << "[JabberProtocol] Setting do not disturb mode."
								<< endl;

	if (!isConnected())
	{
		// we are not connected yet, so connect now
		initialPresence = JabberDND;
		connect();
	}

	// kill old reason dialog if it's still in memory
	if (reasonDialog != 0L)
		delete reasonDialog;

	// TODO Fix this to work with KopeteAwayDialog
	reasonDialog = new dlgJabberStatus(this, JabberDND, qApp->mainWidget() );

}

void JabberProtocol::slotGoInvisible()
{
	kdDebug(JABBER_DEBUG_GLOBAL) << "[JabberProtocol] Setting invisible mode." << endl;

	if (!isConnected())
	{
		// we are not connected yet, so connect now
		initialPresence = JabberInvisible;
		connect();
	}

	setPresence( JabberInvisible );

}

void JabberProtocol::slotSendRaw()
{
	// Check if we're connected
	if(!isConnected())
	{
		errorConnectFirst();
		return;
	}

	// Check if we have one yet
	if(sendRawDialog == 0L){
		sendRawDialog = new dlgJabberSendRaw(jabberClient, qApp->mainWidget());
	}
	// tell it to show
	sendRawDialog->show();

}

void JabberProtocol::subscribe(const Jabber::Jid &jid)
{

	if(!isConnected())
	{
		errorConnectFirst();
		return;
	}

	Jabber::JT_Presence *task =
		new Jabber::JT_Presence(jabberClient->rootTask());

	task->sub(jid, "subscribe");
	task->go(true);

}

void JabberProtocol::subscribed(const Jabber::Jid &jid)
{
	if(!isConnected())
	{
		errorConnectFirst();
		return;
	}

	Jabber::JT_Presence *task =
		new Jabber::JT_Presence(jabberClient->rootTask());

	task->sub(jid, "subscribed");
	task->go(true);

}

void JabberProtocol::sendPresenceToNode( const KopeteOnlineStatus &pres, const QString &userId )
{

	if(!isConnected())
	{
		errorConnectFirst();
		return;
	}

	Jabber::JT_Presence *task =
			new Jabber::JT_Presence(jabberClient->rootTask());

	Jabber::Jid jid(userId);
	Jabber::Status status;

	if( pres == JabberOnline )
		status.setShow("");
	else if( pres == JabberChatty )
		status.setShow("chat");
	else if( pres == JabberAway )
		status.setShow("away");
	else if( pres == JabberXA )
		status.setShow("xa");
	else if( pres == JabberDND )
		status.setShow("dnd");
	else if( pres == JabberInvisible )
	{
		status.setShow("away");
		status.setIsInvisible(true);
	}
	else
		status.setShow("away");

	task->pres(jid, status);
	task->go(true);

}

JabberContact *JabberProtocol::createContact(const QString &jid,
				const QString &alias, const QStringList &groups,
				KopeteMetaContact *metaContact, const QString &identity)
{

	JabberContact *jc =
			new JabberContact(jid, alias, groups, this, metaContact, identity);

	metaContact->addContact(jc );

	return jc;
}


void JabberProtocol::createAddContact(KopeteMetaContact *mc,
				const Jabber::RosterItem &item)
{
	if(!mc)
	{
		mc = KopeteContactList::contactList()->findContact(pluginId(),
				myContact->userId(), item.jid().userHost());

		if(mc)
		{
			JabberContact *jc = (JabberContact *)mc->findContact(pluginId(),
							myContact->userId(), item.jid().userHost());

			if(jc)
			{
				// existing contact, update data
				kdDebug(JABBER_DEBUG_GLOBAL) << "[JabberProtocol] Contact "
											<< item.jid().userHost()
											<< " already exists, updating"
											<< endl;

				jc->slotUpdateContact(item);

				return;
			}
			else
			{
					kdDebug(JABBER_DEBUG_GLOBAL) << "[JabberProtocol]****Warning**** : "
												<< item.jid().userHost()
												<< " already exists, and can be found"
												<< endl;
			}
		}
	}

	kdDebug(JABBER_DEBUG_GLOBAL) << "[JabberProtocol] Adding contact "
								<< item.jid().userHost()
								<< " ..." << endl;

	bool isContactInList=true;;

	if(!mc)
	{
			isContactInList = false;
			mc = new KopeteMetaContact();
			QStringList groups=item.groups();
			for( QStringList::Iterator it = groups.begin();
					it != groups.end(); ++it )
			{
					mc->addToGroup( KopeteContactList::contactList()->getGroup(*it) );
			}
	}

	QString contactName;

	if(item.name().isNull() || item.name().isEmpty())
			contactName = item.jid().userHost();
	else
			contactName = item.name();

	createContact(item.jid().userHost(), contactName,
					item.groups(), mc, myContact->userId());

	if(!isContactInList)
			KopeteContactList::contactList()->addMetaContact(mc);

}

void JabberProtocol::slotSubscription(const Jabber::Jid &jid,
				const QString &type)
{
	kdDebug(JABBER_DEBUG_GLOBAL) << "[JabberProtocol] slotSubscription("
								 << jid.userHost() << ", " << type << ");" << endl;

	if(type == "subscribe")
	{
		// a new user wants to subscribe
		kdDebug(JABBER_DEBUG_GLOBAL) << "[JabberProtocol] slotSubscription(): "
									 << jid.userHost()
									 << " asks for authorization to subscribe."
									 << endl;

		switch(KMessageBox::questionYesNoCancel(qApp->mainWidget(),
									 i18n("The Jabber user %1 wants to add you to their "
													 "contact list. Do you want to authorize them?"
													 " Selecting cancel will ignore the request."
											 ).arg(jid.userHost(), 1),
									 i18n("Authorize Jabber User?"),
									 i18n("Authorize"), i18n("Deny")))
		{
			Jabber::JT_Presence *task;
			KopeteMetaContact *mc;

			case KMessageBox::Yes:
				// authorize user
				subscribed(jid);

				// is the user already in our contact list?
				mc = KopeteContactList::contactList()->findContact(pluginId(),
								myContact->userId(), jid.userHost());

				// if it is not, ask the user if he wants to subscribe in return
				if(!mc && (KMessageBox::questionYesNo(
													 qApp->mainWidget(),
													 i18n("Do you want to add %1 to your contact "
														 "list in return?").arg(jid.userHost(), 1),
													 i18n("Add Jabber User?"))
											 == KMessageBox::Yes))
				{
					// subscribe to user
					subscribe(jid);
				}

				break;

			case KMessageBox::No:
				// deny subscription
				task = new Jabber::JT_Presence(jabberClient->rootTask());

				task->sub(jid, "unsubscribed");
				task->go(true);

				break;

			case KMessageBox::Cancel:
				// leave user uninformed
				break;
		}

	}
	else
		if(type == "unsubscribed")
		{
			// a user deleted auth for us
			kdDebug(JABBER_DEBUG_GLOBAL) << "[JabberProtocol] " << jid.userHost()
										 << " deleted auth!" << endl;

			KMessageBox::information(0L,
							i18n("%1 unsubscribed you!").arg(jid.userHost()),
							i18n("Notification"));

			// delete the item from the roster
			Jabber::JT_Roster *task = new Jabber::JT_Roster(jabberClient->rootTask());

			task->remove(jid);
			task->go(true);
		}
}

void JabberProtocol::slotNewContact(const Jabber::RosterItem &item)
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

	QString debugStr = "[JabberProtocol] New Contact "
		+ item.jid().userHost() + " (Subscription::";

	switch(item.subscription().type())
	{
		case Jabber::Subscription::Both:    // both sides can see the contact
				debugStr += "Both | <->";
				break;

		case Jabber::Subscription::From:    // he can see us
				debugStr += "From | <--";
				break;

		case Jabber::Subscription::To:      // we can see him
				debugStr += "To | -->";
				break;

		case Jabber::Subscription::None:    // waiting for authorization
				debugStr += "None | ---";
				break;
	}

	debugStr += ") " + item.ask();

	kdDebug(JABBER_DEBUG_GLOBAL) << debugStr << endl;

	// add the contact to the GUI
	createAddContact(0L, item);
}

void JabberProtocol::slotContactDeleted(const Jabber::RosterItem &item)
{
	kdDebug(JABBER_DEBUG_GLOBAL) << "[JabberProtocol] Deleting contact "
								 << item.jid().userHost() << endl;

	if( !contacts()[ item.jid().userHost() ] )
	{
		kdDebug(JABBER_DEBUG_GLOBAL) << "[JabberProtocl] WARNING: slotContactDeleted() "
									 << "was asked to delete a non-existing contact."
									 << endl;
		return;
	}

	JabberContact *jc =
			static_cast<JabberContact*>( contacts()[ item.jid().userHost() ] );

	// this will also cause the contact to disappear from the metacontact
	delete jc;
}

void JabberProtocol::slotContactUpdated(const Jabber::RosterItem &item)
{
	kdDebug(JABBER_DEBUG_GLOBAL) << "[JabberProtocol] Status update for "
								 << item.jid().userHost() << endl;

	// sanity check
	if( !contacts()[ item.jid().userHost() ] )
	{
		kdDebug(JABBER_DEBUG_GLOBAL) << "[JabberProtocol] WARNING: slotContactUpdated() "
									 << "was requested to update a non-existing contact."
									 << endl;
		return;
	}

	// update the contact data
	static_cast<JabberContact*>(
					contacts()[ item.jid().userHost() ] )->slotUpdateContact( item );
}

void JabberProtocol::slotSettingsChanged()
{
	KGlobal::config()->setGroup("Jabber");

	QString userId = KGlobal::config()->readEntry("UserID", "");
	QString server = KGlobal::config()->readEntry("Server", "jabber.org");

	// set the title according to the current account
	actionStatusMenu->popupMenu()->changeTitle( menuTitleId ,
			userId + "@" + server );

}

void JabberProtocol::slotReceivedMessage(const Jabber::Message &message)
{
	QString userHost;
	JabberContact *contactFrom;

	userHost = message.from().userHost();
	contactFrom = static_cast<JabberContact *>( contacts()[ userHost ] );

	if ( userHost.isEmpty() )
	{
		// if the sender field is empty, it is a server message
		kdDebug(JABBER_DEBUG_GLOBAL) << "[JabberProtocol] New server message for us!"
				 << endl;

		KMessageBox::information(qApp->mainWidget(),
				message.body(), i18n("Jabber: Server message"));
	}
	else
	{
		kdDebug(JABBER_DEBUG_GLOBAL) << "[JabberProtocol] New message from '"
			 << userHost << "'" << endl;

		// see if the contact is actually in our roster
		if( !contactFrom )
		{
			// this case happens if we are getting a message from a chat room
			// FIXME: this can also happen with contacts we did not
			// previously subscribe to! (especially via transports etc)
			kdDebug(JABBER_DEBUG_GLOBAL) << "[JabberProtocol] Message received from an "
				<< "unknown contact, creating temporary contact." << endl;

			KopeteMetaContact *metaContact = new KopeteMetaContact();
			metaContact->setTemporary(true);

			contactFrom = createContact(userHost, userHost,
										QStringList(), metaContact, myContact->userId());

			KopeteContactList::contactList()->addMetaContact(metaContact);

		}

		// pass the message on to the contact
		contactFrom->slotReceivedMessage(message);
	}

}

void JabberProtocol::slotEmptyMail()
{

	if(!isConnected())
	{
		errorConnectFirst();
		return;
	}

	KLineEditDlg *dlg = new KLineEditDlg(i18n("Please enter a recipient:"),
				QString::null, qApp->mainWidget());

	QObject::connect(dlg, SIGNAL(okClicked()),
					this, SLOT(slotOpenEmptyMail()));

	dlg->show();
	dlg->raise();

}

void JabberProtocol::slotOpenEmptyMail()
{
	QString userHost = ((KLineEditDlg *)sender() )->text();

	if( !userHost.isEmpty() && !userHost.isNull() )
	{
		JabberContact *contact = static_cast<JabberContact*>( contacts()[userHost] );

		// if the contact is not in our contact list yet, create a new temporary one
		if(!contact)
		{
			KopeteMetaContact *metaContact = new KopeteMetaContact();
			metaContact->setTemporary(true);

			contact = createContact(userHost, userHost,
							QStringList(), metaContact, myContact->userId());

			KopeteContactList::contactList()->addMetaContact(metaContact);
		}

		contact->manager(true)->view(true, KopeteMessage::Email);
	}
}

void JabberProtocol::slotJoinNewChat()
{

	if(!isConnected())
	{
		errorConnectFirst();
		return;
	}

	dlgJabberChatJoin *dlg = new dlgJabberChatJoin(qApp->mainWidget());
	dlg->show();
	dlg->raise();

}

void JabberProtocol::slotGroupChatJoined(const Jabber::Jid &jid)
{
	kdDebug(JABBER_DEBUG_GLOBAL) << "[JabberProtocol] Joined group chat " << jid.full() << endl;


	// create new meta contact that holds the group chat contact
	KopeteMetaContact *mc = new KopeteMetaContact();
	mc->setTemporary(true);

	// the group chat object basically works like a JabberContact
	JabberGroupChat *groupChat = new JabberGroupChat(jid, QStringList(), this, mc, myContact->userId());

	// add the group chat class to the meta contact
	mc->addContact(groupChat);

	KopeteContactList::contactList()->addMetaContact(mc);

}

void JabberProtocol::slotGroupChatLeft(const Jabber::Jid &jid)
{
	kdDebug(JABBER_DEBUG_GLOBAL) << "[JabberProtocol] Left groupchat " << jid.full() << endl;

	delete static_cast<JabberGroupChat*>( contacts()[jid.userHost()] );
}

void JabberProtocol::slotGroupChatPresence(const Jabber::Jid &jid, const Jabber::Status &status)
{
	kdDebug(JABBER_DEBUG_GLOBAL) << "[JabberProtocol] Received groupchat presence for room " << jid.full() << endl;

	static_cast<JabberGroupChat*>( contacts()[jid.userHost()] )->updatePresence(jid, status);

}

void JabberProtocol::slotGroupChatError(const Jabber::Jid & /* jid */, int /* error */, QString & /* reason */ )
{
		kdDebug(JABBER_DEBUG_GLOBAL) << "[JabberProtocol] Group chat error!" << endl;

}

void JabberProtocol::slotResourceAvailable(const Jabber::Jid &jid,
				const Jabber::Resource &resource)
{

	kdDebug(JABBER_DEBUG_GLOBAL) << "[JabberProtocol] New resource available for "
								 << jid.userHost() << endl;

	if( !contacts()[ jid.userHost() ] )
	{
		kdDebug(JABBER_DEBUG_GLOBAL) << "[JabberProtocol] Trying to add a resource, but "
									 << "couldn't find an entry for " << jid.userHost()
									 << endl;
		return;
	}

	// By the way, this is SO ugly
	static_cast<JabberContact*>(
					contacts()[jid.userHost()])->slotResourceAvailable(
									jid, resource );

}

void JabberProtocol::slotResourceUnavailable(const Jabber::Jid &jid,
				const Jabber::Resource &resource)
{

	kdDebug(JABBER_DEBUG_GLOBAL) << "[JabberProtocol] Resource now unavailable for "
								 << jid.userHost() << endl;

	if( !contacts()[ jid.userHost() ] )
	{
		kdDebug(JABBER_DEBUG_GLOBAL) << "[JabberProtocol] Trying to remove a resource, "
									 << "but couldn't find an entry for "
									 << jid.userHost() << endl;
		return;
	}

	static_cast<JabberContact*>(
					contacts()[jid.userHost()])->slotResourceUnavailable(
									jid, resource );
}

void JabberProtocol::slotEditVCard()
{

	myContact->slotEditVCard();

}

void JabberProtocol::registerUser()
{

	kdDebug(JABBER_DEBUG_GLOBAL) << "[JabberProtocol] Registering user" << endl;

	// save the current preferences
	preferences->save();

	// set the flag to register an account during registration
	registerFlag = 1;

	// now connect, initiating the registration
	connect();
}

void JabberProtocol::slotRegisterUserDone()
{
	Jabber::JT_Register *task = (Jabber::JT_Register *)sender();

	if(task->success())
		KMessageBox::information(qApp->mainWidget(),
						i18n("Account successfully registered."),
						i18n("Account Registration"));
	else
		KMessageBox::information(qApp->mainWidget(),
						i18n("Unable to create account on the server."),
						i18n("Account Registration"));

	registerFlag = 0;

	disconnect();

}

void JabberProtocol::addContact(KopeteMetaContact *mc, const QString &userId)
{

	// first of all, create a Jabber::RosterItem from the information found
	// in the KopeteMetaContact
	Jabber::RosterItem item;
	KopeteGroupList groupList;
	QStringList groupStringList;

	item.setJid(Jabber::Jid(userId));
	item.setName(userId);

	groupList = mc->groups();
	for( KopeteGroup *g = groupList.first(); g; g = groupList.next() )
	{
		groupStringList.append( g->displayName() );
	}
	item.setGroups(groupStringList);

	createAddContact(mc, item);

	// add the new contact to our roster
	Jabber::JT_Roster *rosterTask =
			new Jabber::JT_Roster(jabberClient->rootTask());

	rosterTask->set(item.jid(), item.name(), item.groups());
	rosterTask->go(true);

	// send a subscription request
	subscribe(item.jid());

}

void JabberProtocol::updateContact(const Jabber::RosterItem &item)
{

	if(!isConnected())
	{
		errorConnectFirst();
		return;
	}

	Jabber::JT_Roster *rosterTask =
			new Jabber::JT_Roster(jabberClient->rootTask());

	rosterTask->set(item.jid(), item.name(), item.groups());
	rosterTask->go(true);

}

void JabberProtocol::removeContact(const Jabber::RosterItem &item)
{

	if(!isConnected())
	{
		errorConnectFirst();
		return;
	}

	Jabber::JT_Roster *rosterTask =
			new Jabber::JT_Roster(jabberClient->rootTask());

	rosterTask->remove(item.jid());
	rosterTask->go(true);

}

void JabberProtocol::slotGetServices()
{

	dlgJabberServices *dialog = new dlgJabberServices();

	dialog->show();
	dialog->raise();

}

#include "jabberprotocol.moc"
/*
 * Local variables:
 * mode: c++
 * c-indentation-style: k&r
 * c-basic-offset: 4
 * indent-tabs-mode: t
 * End:
 */

// vim: set noet ts=4 sts=4 sw=4:

