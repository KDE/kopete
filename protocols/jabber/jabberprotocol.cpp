/***************************************************************************
                          jabberprotocol.cpp  -  Jabber Plugin
                             -------------------
    begin                : Fri Apr 12 2002
    copyright            : (C) 2002 by Daniel Stone, Till Gerken,
                           Kopete Development team
    email                : dstone@kde.org, till@tantalo.net
 ***************************************************************************

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <kdebug.h>
#include <kgenericfactory.h>
#include <kconfig.h>
#include <kmessagebox.h>
#include <kstatusbar.h>
#include <kiconloader.h>
#include <kpopupmenu.h>
#include <kstandarddirs.h>
#include <klineeditdlg.h>

#include <qstringlist.h>
#include <qmap.h>
#include <qpixmap.h>
#include <qmovie.h>
#include <qcursor.h>

#include <psi/client.h>
#include <psi/stream.h>
#include <psi/tasks.h>
#include <psi/types.h>
#include <psi/vcard.h>

#include <sys/utsname.h>

#include "kopete.h"
#include "systemtray.h"   // I believe that this belongs into kopete.h
#include "kopetecontact.h"
#include "kopetecontactlist.h"
#include "kopetemetacontact.h"
#include "kopetemessagemanager.h"
#include "kopeteaway.h"
#include "kopeteprotocol.h"
#include "kopeteplugin.h"
#include "addcontactpage.h"
#include "systemtray.h"
#include "jabbercontact.h"
#include "jabberprefs.h"
#include "dlgjabberstatus.h"
#include "dlgjabbersendraw.h"
#include "dlgjabberservices.h"
#include "jabberaddcontactpage.h"
#include "jabbermap.h"
#include "jabbermessagemanager.h"
#include "jabberprotocol.h"

JabberProtocol *JabberProtocol::protocolInstance = 0;

K_EXPORT_COMPONENT_FACTORY(kopete_jabber, KGenericFactory<JabberProtocol>);

JabberProtocol::JabberProtocol(QObject *parent, QString name, QStringList) : KopeteProtocol(parent, name)
{
	kdDebug() << "[JabberProtocol] Loading ..." << endl;

	// this is meant to be a singleton, so we will check if we have been loaded before
	if (protocolInstance)
	{
		kdDebug() << "[JabberProtocol] Warning: Protocol already loaded, not initializing again." << endl;
		return;
	}

	protocolInstance = this;

	jabberClient = 0L;
	registerFlag = 0;

	// these dialogs will be re-instantiated when needed and deleted after
	// they have served their purpose
	reasonDialog = 0L;
	sendRawDialog = 0L;
	
	myContact = 0L;

	initialPresence = STATUS_ONLINE;

	preferences = new JabberPreferences("jabber_protocol_32", this);
	connect (preferences, SIGNAL(saved()), this, SLOT(slotSettingsChanged()));

	// read the Jabber ID from Kopete's configuration
	KGlobal::config()->setGroup("Jabber");

	// setup actions
	initActions();

	// read remaining settings from configuration file
	slotSettingsChanged();
	
}

JabberProtocol::~JabberProtocol()
{

}

void JabberProtocol::errorConnectFirst()
{

	KMessageBox::error(qApp->mainWidget(), i18n("Please connect first"), i18n("Error"));

}

KActionMenu *JabberProtocol::protocolActions()
{

	return actionStatusMenu;

}

void JabberProtocol::initActions()
{

	actionGoOnline = new KAction(i18n("Online"), "jabber_online", 0, this, SLOT(slotGoOnline()), this, "actionJabberConnect");
	actionGoAway = new KAction(i18n("Away"), "jabber_away", 0, this, SLOT(slotGoAway()), this, "actionJabberAway");
	actionGoXA = new KAction(i18n("Extended Away"), "jabber_away", 0, this, SLOT(slotGoXA()), this, "actionJabberXA");
	actionGoDND = new KAction(i18n("Do Not Disturb"), "jabber_na", 0, this, SLOT(slotGoDND()), this, "actionJabberDND");
	actionGoInvisible = new KAction(i18n("Invisible"), "jabber_offline", 0, this, SLOT(slotGoInvisible()), this, "actionJabberInvisible");
	actionGoOffline = new KAction(i18n("Offline"), "jabber_offline", 0, this, SLOT(slotGoOffline()), this, "actionJabberDisconnect");
	actionServices = new KAction(i18n("Services..."), "filenew", 0, this, SLOT(slotGetServices()), this, "actionJabberServices");
	actionSendRaw = new KAction(i18n("Send raw packet to Server..."), "filenew", 0, this, SLOT(slotSendRaw()), this, "actionJabberSendRaw");
	actionEditVCard = new KAction(i18n("Edit User Info..."), "identity", 0, this, SLOT(slotEditVCard()), this, "actionEditVCard");
	actionEmptyMail = new KAction(i18n("New email message..."), "filenew", 0, this, SLOT(slotEmptyMail()), this, "actionEmptyMail");

	actionStatusMenu = new KActionMenu("Jabber", this);

	// will be overwritten in slotSettingsChanged, maybe there is a better way (gogo) ??? -DS
	menuTitleId = actionStatusMenu->popupMenu()->insertTitle("");

	actionStatusMenu->insert(actionGoOnline);
	actionStatusMenu->insert(actionGoAway);
	actionStatusMenu->insert(actionGoXA);
	actionStatusMenu->insert(actionGoDND);
	actionStatusMenu->insert(actionGoInvisible);
	actionStatusMenu->insert(actionGoOffline);

	actionStatusMenu->popupMenu()->insertSeparator();
	actionStatusMenu->insert(actionServices);
	actionStatusMenu->insert(actionSendRaw);
	actionStatusMenu->insert(actionEditVCard);
	actionStatusMenu->popupMenu()->insertSeparator();
	actionStatusMenu->insert(actionEmptyMail);

}

void JabberProtocol::init()
{

	// initialize icon that sits in Kopete's status bar
	setStatusIcon("jabber_offline");

	KGlobal::config()->setGroup("Jabber");

	// make sure we load the SSL library if required
	if(KGlobal::config()->readBoolEntry("UseSSL", "0"))
	{
		// try to load QSSL library needed by libpsi
		// FIXME: this is ugly because it uses fixed dirs! should check ldconfig or something
		QStringList dirs;

		dirs += "/usr/lib";
		dirs += "/usr/local/lib";
		dirs += QStringList::split(":", QString(getenv("LD_LIBRARY_PATH")));

		Jabber::Stream::loadSSL(dirs);
	}

	// if we need to connect on startup, do it now
	if (KGlobal::config()->readBoolEntry( "AutoConnect", false ) )
		Connect();

}

bool JabberProtocol::unload()
{
	
	kdDebug() << "[JabberProtocol] Unload..." << endl;
	
	Disconnect();

	// kick the SSL library
	Jabber::Stream::unloadSSL();
	
	// make sure that the next attempt to load Jabber
	// re-initializes the protocol class
	protocolInstance = 0L;

	delete actionGoOnline;
	delete actionGoAway;
	delete actionGoXA;
	delete actionGoDND;
	delete actionGoInvisible;
	delete actionGoOffline;

	delete actionServices;
	delete actionSendRaw;
	delete actionEditVCard;
	delete actionEmptyMail;
	
	delete actionStatusMenu;

	return KopeteProtocol::unload();

}

void JabberProtocol::Connect()
{
	
	kdDebug() << "[JabberProtocol] Connect()" << endl;

	// don't do anything if we are already connected
	if (isConnected())
		return;

	// this is dirty but has to be done:
	// if a previous connection attempt failed, psi
	// doesn't handle recovering too well. we are not
	// allowed to call close in the slotConnected() slot
	// since it causes a crash, so we have to delete the
	// psi backend altogether here for safety if it still
	// exists
	if(jabberClient)
	{
		jabberClient->close();
		delete jabberClient;
		jabberClient = 0L;
	}
		
	// instantiate new Psi backend class for handling the protocol and setup slots and signals
	if(!jabberClient)
	{
		jabberClient = new Jabber::Client(this);

		// this should only be done here to connect the signals,
		// otherwise it is a bad idea
		using namespace Jabber;
		
		connect(jabberClient, SIGNAL(handshaken()), this, SLOT(slotHandshaken()));
		connect(jabberClient, SIGNAL(authFinished(bool, int, const QString &)), this, SLOT(slotConnected(bool, int, const QString &)));
		
		connect(jabberClient, SIGNAL(closeFinished()), this, SLOT(slotDisconnected()));

		connect(jabberClient, SIGNAL(subscription(const Jid &, const QString &)), this, SLOT(slotSubscription(const Jid &, const QString &)));
		
		connect(jabberClient, SIGNAL(rosterItemAdded(const RosterItem &)), this, SLOT(slotNewContact(const RosterItem &)));
		connect(jabberClient, SIGNAL(rosterItemUpdated(const RosterItem &)), this, SLOT(slotContactUpdated(const RosterItem &)));
		connect(jabberClient, SIGNAL(rosterItemRemoved(const RosterItem &)), this, SLOT(slotContactDeleted(const RosterItem &)));

		connect(jabberClient, SIGNAL(resourceAvailable(const Jid &, const Resource &)), this, SLOT(slotResourceAvailable(const Jid &, const Resource &)));
		connect(jabberClient, SIGNAL(resourceUnavailable(const Jid &, const Resource &)), this, SLOT(slotResourceUnavailable(const Jid &, const Resource &)));

		connect(jabberClient, SIGNAL(messageReceived(const Message &)), this, SLOT(slotNewMessage(const Message &)));

		connect(jabberClient, SIGNAL(error(const StreamError &)), this, SLOT(slotError(const StreamError &)));

		connect(jabberClient, SIGNAL(debugText(const QString &)), this, SLOT(slotPsiDebug(const QString &)));

		utsname utsBuf;
		uname(&utsBuf);
		
		jabberClient->setClientName("Kopete Jabber Plugin");
		jabberClient->setClientVersion("0.5.1");
		jabberClient->setOSName(QString("%1 %2").arg(utsBuf.sysname, 1).arg(utsBuf.release, 2));

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
				i18n( "Jabber plugin not configured yet" ),
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
							   i18n("SSL is not supported. This is most likely because the QSSL library could not be found."),
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
	
	Jabber::StreamProxy proxy(proxyType, KGlobal::config()->readEntry("ProxyName", ""), KGlobal::config()->readNumEntry("ProxyPort", 8080));
	proxy.setUseAuth(KGlobal::config()->readBoolEntry("ProxyAuth", false));
	proxy.setUser(KGlobal::config()->readEntry("ProxyUser", ""));
	proxy.setPass(KGlobal::config()->readEntry("ProxyPass", ""));

	jabberClient->setProxy(proxy);
	
	if(myContact)
	{
		contactMap.remove(myContact->userId());
		delete myContact;
	}

	// create a contact instance for self
	myContact = new JabberContact(QString("%1@%2").arg(userId, 1).arg(server, 2), userId, QStringList(i18n("Unknown")), this, 0L, QString::null);

	contactMap.insert(myContact->userId(), myContact);

	// set the title according to the new changes
	actionStatusMenu->popupMenu()->changeTitle( menuTitleId , userId + "@" + server );

	kdDebug() << "[JabberProtocol] Connecting to Jabber server " << server << ":" << port << endl;
	kdDebug() << "                 with UserID " << userId << endl;

	// now connect
	jabberClient->connectToHost(server, port);

	// play movie to indicate connection attempt
	setStatusIcon("jabber_connecting");

}

void JabberProtocol::slotPsiDebug(const QString &msg)
{

	kdDebug() << "[JabberProtocol] Psi: " << msg << endl;

}

void JabberProtocol::slotHandshaken()
{

	kdDebug() << "[JabberProtocol] Performing login..." << endl;

	if(registerFlag)
	{
		Jabber::JT_Register *task = new Jabber::JT_Register(jabberClient->rootTask());
		connect(task, SIGNAL(finished()), this, SLOT(slotRegisterUserDone()));

		task->reg(KGlobal::config()->readEntry("UserID", ""), KGlobal::config()->readEntry("Password", ""));

		task->go(true);
	}
	else
	{
		KGlobal::config()->setGroup("Jabber");

		if(KGlobal::config()->readEntry("AuthType", "digest") == QString("digest"))
		{
			jabberClient->authDigest(KGlobal::config()->readEntry("UserID", ""),
						KGlobal::config()->readEntry("Password", ""),
						KGlobal::config()->readEntry("Resource", "Kopete"));
		}
		else
		{
			jabberClient->authPlain(KGlobal::config()->readEntry("UserID", ""),
						KGlobal::config()->readEntry("Password", ""),
						KGlobal::config()->readEntry("Resource", "Kopete"));
		}
	}

}

void JabberProtocol::slotConnected(bool success, int statusCode, const QString &statusString)
{

	if(success)
	{
		kdDebug() << "[JabberProtocol] Connected to Jabber server." << endl;

		setStatusIcon("jabber_online");

		// request roster
		jabberClient->rosterRequest();

		// since we are online now, set initial presence
		// don't do this before the roster request or we will receive
		// presence information before we have updated our roster
		// with actual contacts from the server! (libpsi won't forward
		// presence information in that case either)
		setPresence(initialPresence, myContact->reason());

		// initiate anti-idle timer (will be triggered every 10 seconds)
		jabberClient->setNoopTime(10000);
	}
	else
	{
		kdDebug() << "[JabberProtocol] Connection failed! Status: " << statusCode << ", " << statusString << endl;
		setStatusIcon("jabber_offline");
		KMessageBox::error(qApp->mainWidget(), i18n("Connection failed with reason \"%1\"").arg(statusString, 1), i18n("Connection Failed"));
	}

}


JabberProtocol *JabberProtocol::protocol()
{
	
	// return current instance
	return protocolInstance;
	
}

void JabberProtocol::Disconnect()
{
	
	if (isConnected())
	{
		// tell backend class to disconnect
		jabberClient->close();
	}

	jabberClient->deleteLater();
	jabberClient = 0L;

	kdDebug() << "[JabberProtocol] Disconnected." << endl;

	setStatusIcon("jabber_offline");

	// it seems that we don't get offline notifications
	// when going offline with the protocol, so update
	// all contacts manually
	for(JabberContactMap::iterator it = contactMap.begin(); it != contactMap.end(); it++)
	{
		it.data()->slotUpdatePresence(STATUS_OFFLINE, "");
	}
	
}

void JabberProtocol::slotConnect()
{
	
	Connect();
	
}

void JabberProtocol::slotDisconnect()
{

	Disconnect();

}

void JabberProtocol::slotDisconnected()
{
	
	kdDebug() << "[JabberProtocol] Disconnected from Jabber server." << endl;

	jabberClient->deleteLater();
	jabberClient = 0L;

	setStatusIcon("jabber_offline");

}

void JabberProtocol::slotError(const Jabber::StreamError &error)
{

	// determine type of error
	switch(error.type())
	{
		case Jabber::StreamError::DNS:
			KMessageBox::error(qApp->mainWidget(), i18n("DNS error (%1)").arg(error.details(), 1),
						i18n("Error Connecting to Jabber Server"));
			break;

		case Jabber::StreamError::Refused:
			KMessageBox::error(qApp->mainWidget(), i18n("Connection refused (%1)").arg(error.details(), 1),
						i18n("Error Connecting to Jabber Server"));
			break;

		case Jabber::StreamError::Timeout:
			KMessageBox::error(qApp->mainWidget(), i18n("Timeout (%1)").arg(error.details(), 1),
						i18n("Error Connecting to Jabber Server"));
			break;

		case Jabber::StreamError::Socket:
			KMessageBox::error(qApp->mainWidget(), i18n("Socket error (%1)").arg(error.details(), 1),
						i18n("Error Connecting to Jabber Server"));
			break;

		case Jabber::StreamError::Disconnected:
			KMessageBox::error(qApp->mainWidget(), i18n("Remote server closed connection (%1)").arg(error.details(), 1),
						i18n("Error Connecting to Jabber Server"));
			break;

		case Jabber::StreamError::Handshake:
			KMessageBox::error(qApp->mainWidget(), i18n("Handshake failed (%1)").arg(error.details(), 1),
						i18n("Error Connecting to Jabber Server"));
			break;

		case Jabber::StreamError::SSL:
			KMessageBox::error(qApp->mainWidget(), i18n("SSL error (%1)").arg(error.details(), 1),
						i18n("Error Connecting to Jabber Server"));
			break;

		case Jabber::StreamError::Proxy:
			KMessageBox::error(qApp->mainWidget(), i18n("Proxy error (%1)").arg(error.details(), 1),
						i18n("Error Connecting to Jabber Server"));
			break;

		case Jabber::StreamError::Unknown:
		default:
			KMessageBox::error(qApp->mainWidget(), i18n("An unknown error was encountered (%1)").arg(error.details(), 1),
						i18n("Error Connecting to Jabber Server"));
			break;

	}

	Disconnect();

}

bool JabberProtocol::isConnected() const
{

	if(!jabberClient)
		return false;

	return(jabberClient->isAuthenticated());

}

/*
 * Set presence (usually called by dialog widget)
 */
void JabberProtocol::setPresence(Presence status, const QString &reason, int priority)
{

	if (isConnected())
	{
		Jabber::Status presence;

		presence.setPriority(priority);
		presence.setStatus(reason);
		presence.setIsAvailable(true);
		
		switch(status)
		{
			case STATUS_AWAY:
						setStatusIcon("jabber_na");
						presence.setShow("away");
						break;
						
			case STATUS_XA:
						setStatusIcon("jabber_na");
						presence.setShow("xa");
						break;

			case STATUS_DND:
						setStatusIcon("jabber_na");
						presence.setShow("dnd");
						break;
			case STATUS_INVISIBLE:
						setStatusIcon("jabber_offline");
						presence.setIsInvisible(true);
						break;

			case STATUS_ONLINE:
						setStatusIcon("jabber_online");
						presence.setShow("chat");
						break;

			default:
						kdDebug() << "[JabberProtocol] Unknown presence status, ignoring (status == " << status << ")" << endl;
						break;
		}

		kdDebug() << "[JabberProtocol] setPresence() called, updating to \"" << presence.status() << "\" with reason \"" << reason << endl;

		myContact->slotUpdatePresence(status, reason);
		
		Jabber::JT_Presence *task = new Jabber::JT_Presence(jabberClient->rootTask());
		task->pres(presence);
		task->go(true);

	}

}

void JabberProtocol::setAway(void)
{

	kdDebug() << "[JabberProtocol] Setting globally away." << endl;
    
	setPresence(STATUS_AWAY, KopeteAway::getInstance()->message());

}

void JabberProtocol::setAvailable(void)
{
    
	slotGoOnline();

}

KopeteContact* JabberProtocol::myself() const
{

	return myContact;

}

bool JabberProtocol::isAway(void) const
{

	return (myContact->status() != JabberContact::Online);

}

QString JabberProtocol::protocolIcon() const
{
    
	return "jabber_protocol_32";

}

void JabberProtocol::serialize(KopeteMetaContact *mc ) 
{
	QStringList addressList;
	QStringList data;
	kdDebug() << "[JabberProtocol] Serializing data for metacontact " << mc->displayName() << endl;

	QPtrList<KopeteContact> contacts = mc->contacts();

	// iterate through all contacts that the metacontact has
	for(KopeteContact *c = contacts.first(); c; c = contacts.next())
	{
		if(c->protocol()->id() != this->id())
			continue;

		JabberContact *jc = (JabberContact *)c;
		
		data << jc->identityId() << jc->userId() << jc->displayName() << jc->groups().join(",");

		addressList << jc->userId();
	}

	QString addresses = addressList.join(",");
//	if(!addresses.isEmpty()) //if there are no contact, then erase old data
	mc->setAddressBookField(this, "messaging/jabber", addresses);
	mc->setPluginData(this , data );
}

void JabberProtocol::deserialize(KopeteMetaContact *contact, const QStringList &data)
{

	kdDebug() << "[JabberProtocol] Deserializing data for metacontact " << contact->displayName() << endl;

	for(unsigned i = 0; i < data.count(); i += 4)
	{
		QString identityId = data[i + 0];
		QString userId = data[i + 1];
		QString displayName = data[i + 2];
		QStringList groups;

		// make sure that we don't segfault if no groups are supplied
		if((data.count() - i) >= 4)
			QStringList groups = QStringList::split(",", data[i + 3]);

		kdDebug() << "[JabberProtocol] Deserialized subcontact " << userId << endl;
			
		JabberContact *jc = new JabberContact(userId, displayName, groups, this, contact, identityId);
	
		metaContactMap.insert(jc, contact);
		contactMap.insert(userId, jc);
	
		connect(jc, SIGNAL(contactDestroyed(KopeteContact *)), this, SLOT(slotContactDestroyed(KopeteContact*)));
		connect(jc, SIGNAL(chatUser(JabberContact *)), this, SLOT(slotChatUser(JabberContact *)));
		connect(jc, SIGNAL(emailUser(JabberContact *)), this, SLOT(slotEmailUser(JabberContact *)));

		contact->addContact(jc);
	}

}

QStringList JabberProtocol::addressBookFields() const
{

	return QStringList("messaging/jabber");

}

void JabberProtocol::slotContactDestroyed(KopeteContact *contact)
{

	metaContactMap.remove((JabberContact *)contact);
	contactMap.remove(((JabberContact *)contact)->userId());

}
 
/*
 * Notification slot in case one of the monitored address book fields changed
 */
/*
void JabberProtocol::addressBookFieldChanged(KopeteMetaContact *contact, const QString &key)
{

}
*/

AddContactPage *JabberProtocol::createAddContactWidget(QWidget *parent)
{

	return new JabberAddContactPage(this, parent);

}

void JabberProtocol::slotGoOnline()
{

	kdDebug() << "[JabberProtocol] Going online!" << endl;

	if (!isConnected())
	{
		// we are not connected yet, so connect now
		initialPresence = STATUS_ONLINE;
		Connect();
	}

	setPresence(STATUS_ONLINE, "");

}

void JabberProtocol::slotGoOffline()
{
    
	kdDebug() << "[JabberProtocol] Going offline." << endl;
    
	Disconnect();

}

void JabberProtocol::slotGoAway()
{
	
	kdDebug() << "[JabberProtocol] Setting away mode." << endl;

	if (!isConnected())
	{
		// we are not connected yet, so connect now
		initialPresence = STATUS_AWAY;
		Connect();
	}

	// kill old reason dialog if it's still in memory
	if (reasonDialog != 0L)
		delete reasonDialog;
	
	reasonDialog = new dlgJabberStatus(this, STATUS_AWAY, qApp->mainWidget());
	
}

void JabberProtocol::slotGoXA()
{

	kdDebug() << "[JabberProtocol] Setting extended away mode." << endl;

	if (!isConnected())
	{
		// we are not connected yet, so connect now
		initialPresence = STATUS_XA;
		Connect();
	}

	// kill old reason dialog if it's still in memory
	if (reasonDialog != 0L)
		delete reasonDialog;
	
	reasonDialog = new dlgJabberStatus(this, STATUS_XA, qApp->mainWidget());

}

void JabberProtocol::slotGoDND()
{
    
	kdDebug() << "[JabberProtocol] Setting do not disturb mode." << endl;

	if (!isConnected())
	{
		// we are not connected yet, so connect now
		initialPresence = STATUS_DND;
		Connect();
	}

	// kill old reason dialog if it's still in memory
	if (reasonDialog != 0L)
		delete reasonDialog;

	reasonDialog = new dlgJabberStatus(this, STATUS_DND, qApp->mainWidget());

}

void JabberProtocol::slotGoInvisible()
{

	kdDebug() << "[JabberProtocol] Setting invisible mode." << endl;

	if (!isConnected())
	{
		// we are not connected yet, so connect now
		initialPresence = STATUS_INVISIBLE;
		Connect();
	}

	setPresence(STATUS_INVISIBLE);

}

void JabberProtocol::slotSendRaw()
{

	if(!isConnected())
	{
		errorConnectFirst();
		return;
	}

	// kill old dialog if it's still in memory
	if (sendRawDialog != 0L)
		delete sendRawDialog;

	sendRawDialog = new dlgJabberSendRaw(this, qApp->mainWidget());

}

void JabberProtocol::sendRawMessage(const QString &packet)
{

	kdDebug() << "--- Sending raw message ---" << endl << packet << "---------------------------" << endl ;

	jabberClient->send(packet);

}

void JabberProtocol::subscribe(const Jabber::Jid &jid)
{

	if(!isConnected())
	{
		errorConnectFirst();
		return;
	}

	Jabber::JT_Presence *task = new Jabber::JT_Presence(jabberClient->rootTask());

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

	Jabber::JT_Presence *task = new Jabber::JT_Presence(jabberClient->rootTask());

	task->sub(jid, "subscribed");
	task->go(true);

}

void JabberProtocol::sendPresenceToNode(const Presence &pres,const QString &userId )
{

	if(!isConnected())
	{
		errorConnectFirst();
		return;
	}

	Jabber::JT_Presence *task = new Jabber::JT_Presence(jabberClient->rootTask());
	Jabber::Jid jid(userId);
	Jabber::Status status;

	switch(pres)
	{
		case STATUS_ONLINE:
			status.setShow("chat");
			break;
		case STATUS_AWAY:
			status.setShow("away");
			break;
		case STATUS_XA:
			status.setShow("xa");
			break;
		case STATUS_DND:
			status.setShow("dnd");
			break;
		case STATUS_INVISIBLE:
			status.setShow("away");
			status.setIsInvisible(true);
			break;
		default:
			status.setShow("away");
			break;
	}

	task->pres(jid, status);
	task->go(true);

}

JabberContact *JabberProtocol::createContact(const QString &jid, const QString &alias, const QStringList &groups, KopeteMetaContact *metaContact, const QString &identity)
{

	JabberContact *jc = new JabberContact(jid, alias, groups, this, metaContact, identity);

	connect(jc, SIGNAL(contactDestroyed(KopeteContact *)), this, SLOT(slotContactDestroyed(KopeteContact *)));
	connect(jc, SIGNAL(chatUser(JabberContact *)), this, SLOT(slotChatUser(JabberContact *)));
	connect(jc, SIGNAL(emailUser(JabberContact *)), this, SLOT(slotEmailUser(JabberContact *)));

	metaContact->addContact(jc );

	contactMap.insert(jid, jc);

	metaContactMap.insert(jc, metaContact);

	return jc;

}


void JabberProtocol::createAddContact(KopeteMetaContact *mc, const Jabber::RosterItem &item)
{
	if(!mc)
	{
		mc = KopeteContactList::contactList()->findContact(this->id(), myContact->userId(), item.jid().userHost());

		if(mc)
		{
			JabberContact *jc = (JabberContact *)mc->findContact(this->id(), myContact->userId(), item.jid().userHost());

			if(jc)
			{
				// existing contact, update data
				kdDebug() << "[JabberProtocol] Contact " << item.jid().userHost() << " already exists, updating" << endl;

				jc->slotUpdateContact(item);

				// Due to the fact that we serialize and deserialize contacts
				// on startup, they usually exist in the GUI roster but not in
				// our contact map associations yet after connecting.
				// Make sure that we propagate the associations into the maps
				// now.
				contactMap.insert(item.jid().userHost(), jc);
				metaContactMap.insert(jc, mc);

				return;
			}
			else
			{
				kdDebug() << "[JabberProtocol]****Warning**** : " << item.jid().userHost() << " already exists, and can be found" << endl;
			}
		}
	}

	kdDebug() << "[JabberProtocol] Adding contact " << item.jid().userHost() << " ..." << endl;

	bool isContactInList=true;;
		
	if(!mc)
	{
		isContactInList = false;
		mc = new KopeteMetaContact();
		QStringList groups=item.groups();
		for( QStringList::Iterator it = groups.begin(); it != groups.end(); ++it )
		{
			mc->addToGroup( KopeteContactList::contactList()->getGroup(*it) );
		}
	}

	QString contactName;
		
	if(item.name().isNull() || item.name().isEmpty())
		contactName = item.jid().userHost();
	else
		contactName = item.name();

	createContact(item.jid().userHost(), contactName, item.groups(), mc, myContact->userId());

	if(!isContactInList)
		KopeteContactList::contactList()->addMetaContact(mc);

}

void JabberProtocol::slotSubscription(const Jabber::Jid &jid, const QString &type)
{
	kdDebug() << "[JabberProtocol] slotSubscription(" << jid.userHost() << ", " << type << ");" << endl;

	if(type == "subscribe")
	{
		// a new user wants to subscribe
		kdDebug() << "[JabberProtocol] slotSubscription(): " << jid.userHost() << " asks for authorization to subscribe." << endl;

		switch(KMessageBox::questionYesNoCancel(qApp->mainWidget(),
			i18n("The Jabber user %1 wants to add you to their contact list. Do you want to authorize them? Selecting cancel will ignore the request.").arg(jid.userHost(), 1),
			i18n("Authorize Jabber User?"), i18n("Authorize"), i18n("Deny")))
		{
			Jabber::JT_Presence *task;
			KopeteMetaContact *mc;

			case KMessageBox::Yes:
					// authorize user
					subscribed(jid);

					// is the user already in our contact list?
					mc = KopeteContactList::contactList()->findContact(this->id(), myContact->userId(), jid.userHost());

					// if it is not, ask the user if he wants to subscribe in return
					if(!mc && (KMessageBox::questionYesNo(qApp->mainWidget(),
						i18n("Do you want to add %1 to your contact list in return?").arg(jid.userHost(), 1),
						i18n("Add Jabber User?")) == KMessageBox::Yes))
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
		kdDebug() << "[JabberProtocol] " << jid.userHost() << " deleted auth!" << endl;

		KMessageBox::information(0L, i18n("%1 unsubscribed you!").arg(jid.userHost()), i18n("Notification"));

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

	QString debugStr = "[JabberProtocol] New Contact " + item.jid().userHost() + " (Subscription::";
	
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

	kdDebug() << debugStr << endl;
	
	// add the contact to the GUI
	createAddContact(0L, item);

}

void JabberProtocol::slotContactDeleted(const Jabber::RosterItem &item)
{

	kdDebug() << "[JabberProtocol] Deleting contact " << item.jid().userHost() << endl;

	if(!contactMap.contains(item.jid().userHost()))
	{
		kdDebug() << "[JabberProtocl] WARNING: slotContactDeleted() was asked to delete a non-existing contact." << endl;
		return;
	}

	JabberContact *jc = contactMap[item.jid().userHost()];

	// delete contact
	metaContactMap.remove(jc);
	contactMap.remove(item.jid().userHost());

	// this will also cause the contact to disappear from the metacontact
	delete jc;

}

void JabberProtocol::slotContactUpdated(const Jabber::RosterItem &item)
{

	kdDebug() << "[JabberProtocol] Status update for " << item.jid().userHost() << endl;

	// sanity check
	if(!contactMap.contains(item.jid().userHost()))
	{
		kdDebug() << "[JabberProtocol] WARNING: slotContactUpdated() was requested to update a non-existing contact." << endl;
		return;
	}

	// update the contact data
	contactMap[item.jid().userHost()]->slotUpdateContact(item);

}

void JabberProtocol::slotSettingsChanged()
{

	KGlobal::config()->setGroup("Jabber");

	QString userId = KGlobal::config()->readEntry("UserID", "");
	QString server = KGlobal::config()->readEntry("Server", "jabber.org");

	// set the title according to the current account
	actionStatusMenu->popupMenu()->changeTitle( menuTitleId , userId + "@" + server );

}

void JabberProtocol::slotSendMessage(Jabber::Message message)
{

	// just pass the message on to the Psi backend if we are connected
	if (isConnected())
		jabberClient->sendMessage(message);

}

JabberMessageManager *JabberProtocol::createMessageManager(JabberContact *to, KopeteMessageManager::WidgetType type)
{

	KopeteContactPtrList chatMembers;
	chatMembers.append(to);

	JabberMessageManager *manager = new JabberMessageManager(myself(), chatMembers, type);

	connect(manager, SIGNAL(dying(KopeteMessageManager *)), this, SLOT(slotMessageManagerDeleted(KopeteMessageManager *)));
	
	return manager;

}

void JabberProtocol::slotMessageManagerDeleted(KopeteMessageManager *manager)
{

	kdDebug() << "[JabberProtocol] slotMessageManagerDeleted() message manager deleted, removing from map" << endl;

	for(JabberMessageManagerMap::iterator it = messageManagerMap.begin(); it != messageManagerMap.end(); it++)
	{
		// static cast required unfortunately
		if(it.data() == (JabberMessageManager *)manager)
		{
			kdDebug() << "[JabberProtocol] slotMessageManagerDeleted() found a match in " << it.key() << endl;
			messageManagerMap.remove(it);
		}
	}

}

void JabberProtocol::slotNewMessage(const Jabber::Message &message)
{

	if (message.from().userHost().isEmpty())
	{
		// if the sender field is empty, it is a server message
		kdDebug() << "[JabberProtocol] New server message for us!" << endl;
		
		KMessageBox::information(qApp->mainWidget(), message.body(), i18n("Jabber: Server message"));
	}
	else
	{
		kdDebug() << "[JabberProtocol] New message from '" << message.from().userHost() << "'" << endl;

		// see if the contact is actually in our roster
		if (!contactMap.contains(message.from().userHost()))
		{
			// this should never happen - but _could_ happen if we are getting a message from
			// someone via a transport who hasn't previously subscribed with us (FIXME?)
			kdDebug() << "[JabberProtocol] Message received from an unknown contact, ignoring." << endl;
			return;
		}

		// now we know that the contact is in our roster, see
		// if we already have a message manager for this contact
		JabberMessageManager *manager = 0L;

		KGlobal::config()->setGroup("Jabber");
		bool emailType = KGlobal::config()->readBoolEntry("EmailDefault", false);

		if(!messageManagerMap.contains(message.from().userHost()))
		{
			// we don't have a widget yet, create one
			if((message.type() == "chat") && !emailType)
				manager = createMessageManager(contactMap[message.from().userHost()], KopeteMessageManager::ChatWindow);
			else
				manager = createMessageManager(contactMap[message.from().userHost()], KopeteMessageManager::Email);

			// add this manager to the map
			messageManagerMap[message.from().userHost()] = manager;
		}
		else
		{
			manager = messageManagerMap[message.from().userHost()];

			// we have a message manager, determine its type
			if(((message.type() == "chat") && !emailType && (manager->widgetType() != KopeteMessageManager::ChatWindow))
			  || ((message.type() == "normal") && (manager->widgetType() != KopeteMessageManager::Email)))
			{
				// we have a message manager with an incorrect type, so reinstantiate it
				delete manager;

				if((message.type() == "chat") && !emailType)
					manager = createMessageManager(contactMap[message.from().userHost()], KopeteMessageManager::ChatWindow);
				else
					manager = createMessageManager(contactMap[message.from().userHost()], KopeteMessageManager::Email);

				messageManagerMap[message.from().userHost()] = manager;
			}
		}

		KopeteContactPtrList contactList;
		contactList.append(myself());

		KopeteMessage newMessage(message.timeStamp(), contactMap[message.from().userHost()], contactList, message.body(), message.subject(), KopeteMessage::Inbound, KopeteMessage::PlainText);

			
		// pass the message on to the manager
		manager->appendMessage(newMessage);

	}

}

void JabberProtocol::slotEmptyMail()
{

	if(!isConnected())
	{
		errorConnectFirst();
		return;
	}

	KLineEditDlg *dlg = new KLineEditDlg(i18n("Please enter a recipient"), QString::null, qApp->mainWidget());
	connect(dlg, SIGNAL(okClicked()), this, SLOT(slotOpenEmptyMail()));
	dlg->show();
	dlg->raise();

}

void JabberProtocol::slotOpenEmptyMail()
{
	KLineEditDlg *dlg = (KLineEditDlg *)sender();

	KopeteMetaContact *metaContact = new KopeteMetaContact();
	metaContact->setTemporary(true);

	JabberContact *contact = createContact(dlg->text(), dlg->text(), QStringList(), metaContact, myContact->userId());

	KopeteContactList::contactList()->addMetaContact(metaContact);

	messageManagerMap[contact->id()] = createMessageManager(contact, KopeteMessageManager::Email);

	messageManagerMap[contact->id()]->readMessages();

}

void JabberProtocol::slotChatUser(JabberContact *contact)
{

	kdDebug() << "[JabberProtocol] slotChatUser() trying to open a new message window..." << endl;

	if(!messageManagerMap.contains(contact->id()))
	{
		kdDebug() << "[JabberProtocol] slotChatUser() no old window, creating new one" << endl;
		messageManagerMap[contact->id()] = createMessageManager(contact, KopeteMessageManager::ChatWindow);
	}

	messageManagerMap[contact->id()]->readMessages();

}

void JabberProtocol::slotEmailUser(JabberContact *contact)
{

	kdDebug() << "[JabberProtocol] slotEmailUser() trying to open a new message window..." << endl;

	if(!messageManagerMap.contains(contact->id()))
	{
		kdDebug() << "[JabberProtocol] slotEmailUser() no old window, creating new one" << endl;
		messageManagerMap[contact->id()] = createMessageManager(contact, KopeteMessageManager::Email);
	}

	messageManagerMap[contact->id()]->readMessages();

}

void JabberProtocol::slotResourceAvailable(const Jabber::Jid &jid, const Jabber::Resource &resource)
{

	kdDebug() << "[JabberProtocol] New resource available for " << jid.userHost() << endl;

	if(!contactMap.contains(jid.userHost()))
	{
		kdDebug() << "[JabberProtocol] Trying to add a resource, but couldn't find an entry for " << jid.userHost() << endl;
		return;
	}

	contactMap[jid.userHost()]->slotResourceAvailable(jid, resource);

}

void JabberProtocol::slotResourceUnavailable(const Jabber::Jid &jid, const Jabber::Resource &resource)
{

	kdDebug() << "[JabberProtocol] Resource now unavailable for " << jid.userHost() << endl;

	if(!contactMap.contains(jid.userHost()))
	{
		kdDebug() << "[JabberProtocol] Trying to remove a resource, but couldn't find an entry for " << jid.userHost() << endl;
		return;
	}

	contactMap[jid.userHost()]->slotResourceUnavailable(jid, resource);

}

void JabberProtocol::slotRetrieveVCard(const Jabber::Jid &jid)
{

	if(!isConnected())
	{
		errorConnectFirst();
		return;
	}

	Jabber::JT_VCard *task = new Jabber::JT_VCard(jabberClient->rootTask());
	
	// signal to ourselves when the vCard data arrived
	connect(task, SIGNAL(finished()), this, SLOT(slotGotVCard()));

	task->get(jid);

	// don't autodelete as we want to use the task data
	// when it arrived
	task->go(false);

}

void JabberProtocol::slotGotVCard()
{
	Jabber::JT_VCard *vCard = (Jabber::JT_VCard *) sender();

	kdDebug() << "[JabberProtocol] slotGotVCard() success: " << vCard->success() << ", jid: " << vCard->jid().userHost() << ", myjid: " << myContact->userId() << ", incomplete: " << vCard->vcard().isIncomplete() << endl;
	
	if (!vCard->success() || (vCard->vcard().isIncomplete()) && (vCard->jid().userHost() != myContact->userId()))
	{
		// unsuccessful, or incomplete
		KMessageBox::error(qApp->mainWidget(), i18n("Unable to retrieve vCard for %1").arg(vCard->jid().userHost()));
		return;
	}

	if (!contactMap.contains(vCard->jid().userHost()))
	{
		kdDebug() << "[JabberProtocol] slotGotVCard received a vCard - but couldn't find JID " << vCard->jid().userHost() << " in the list!" << endl;
		return;
	}
	
	contactMap[vCard->jid().userHost()]->slotGotVCard(vCard);

	delete vCard;

}

void JabberProtocol::slotEditVCard()
{

	myContact->slotEditVCard();

}

void JabberProtocol::slotSaveVCard(QDomElement &vCardXML)
{

	if(!isConnected())
	{
		errorConnectFirst();
		return;
	}

	Jabber::JT_VCard *task = new Jabber::JT_VCard(jabberClient->rootTask());
	Jabber::VCard vCard = Jabber::VCard();
	
	vCard.fromXml(vCardXML);
	
	task->set(vCard);
	task->go(true);

}

void JabberProtocol::registerUser()
{

	kdDebug() << "[JabberProtocol] Registering user" << endl;

	// save the current preferences
	preferences->save();

	// set the flag to register an account during registration
	registerFlag = 1;

	// now connect, initiating the registration
	Connect();
}

void JabberProtocol::slotRegisterUserDone()
{
	Jabber::JT_Register *task = (Jabber::JT_Register *)sender();

	if(task->success())
		KMessageBox::information(qApp->mainWidget(), i18n("Account successfully registered."), i18n("Account Registration"));
	else
		KMessageBox::information(qApp->mainWidget(), i18n("Unable to create account on the server."), i18n("Account Registration"));

	registerFlag = 0;
		
	Disconnect();

}

void JabberProtocol::addContact(KopeteMetaContact *mc, const QString &userId)
{

	// first of all, create a Jabber::RosterItem from the information found
	// in the KopeteMetaContact
	Jabber::RosterItem item;

	item.setJid(Jabber::Jid(userId));
	item.setName(userId);
	item.setGroups(mc->groups().toStringList());

	createAddContact(mc, item);
	
	// add the new contact to our roster
	Jabber::JT_Roster *rosterTask = new Jabber::JT_Roster(jabberClient->rootTask());
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

	Jabber::JT_Roster *rosterTask = new Jabber::JT_Roster(jabberClient->rootTask());
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

	Jabber::JT_Roster *rosterTask = new Jabber::JT_Roster(jabberClient->rootTask());
	rosterTask->remove(item.jid());
	rosterTask->go(true);

}

void JabberProtocol::slotGetServices()
{

	DlgJabberServices *dialog = new DlgJabberServices();

	dialog->show();
	dialog->raise();

}

#include "jabberprotocol.moc"


/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:
