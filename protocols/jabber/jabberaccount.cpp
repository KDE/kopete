/***************************************************************************
                   jabberaccount.cpp  -  core Jabber account class
                             -------------------
    begin                : Sat Mär 8 2003
    copyright            : (C) 2003 by Daniel Stone <dstone@kde.org>
    			   Based on JabberProtocol by Daniel Stone and
			   Till Gerken <till@tantalo.net>.

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

#include <qstring.h>
#include <qapplication.h>
#include <qcursor.h>
#include <qmap.h>
#include <qtimer.h>
#include <qpixmap.h>

#include <kdebug.h>
#include <kgenericfactory.h>
#include <kmessagebox.h>
#include <kiconloader.h>
#include <kpopupmenu.h>
#include <kstandarddirs.h>
#include <klineeditdlg.h>

#include "../kopete/kopete.h"
#include "kopetecontact.h"
#include "kopetecontactlist.h"
#include "kopetemetacontact.h"
#include "kopetemessagemanager.h"
#include "kopeteaway.h"
#include "kopeteprotocol.h"
#include "kopeteaccount.h"
#include "kopeteplugin.h"
#include "addcontactpage.h"
#include "jabberprefs.h"
#include "dlgjabberstatus.h"
#include "dlgjabbersendraw.h"
#include "dlgjabberservices.h"
#include "dlgjabberchatjoin.h"
#include "jabberaddcontactpage.h"
#include "jabbermap.h"
#include "jabbergroupchat.h"

#include "jid.h"
#include "client.h"
#include "stream.h"
#include "tasks.h"
#include "types.h"
#include "vcard.h"

#include "jabberaccount.h"

#include <sys/utsname.h>


JabberAccount::JabberAccount(JabberProtocol * parent, const QString & accountId,
			     const char *name):KopeteAccount(parent, accountId,
							     name) {


    //protocol = parent;
    /* Create a new JabberContact for this account, to be returned from
     * myself(). */
    myContact = new JabberContact(accountId, accountId, QStringList(),
				  this, 0L,
				  accountId, "jabber_offline");

    jabberClient = 0L;
    registerFlag = 0;
    /* This is deleted in the destructor. */
    reasonDialog = 0L;
    /* This is not yet implemented. */
    sendRawDialog = 0L;

    //myContact = 0L;

    initialPresence = JabberProtocol::getJabberOnline();

    /* Setup actions. */
    initActions();
    setAccountId(accountId);
    userID = accountId.section('@',0,0);
    password = getPassword();

    QString serverInfo =  accountId.section('@',1);
    server = serverInfo.section(':',0,0);
    QString tport = serverInfo.section(':',1,1);
    port = tport.section('/',0,0).toUInt();
    resource = accountId.section('/',1,1);

    kdDebug(JABBER_DEBUG_GLOBAL) << "[JabberAccount] created\n";
    kdDebug(JABBER_DEBUG_GLOBAL) << "accountId:" << accountId << "\n";
    kdDebug(JABBER_DEBUG_GLOBAL) << "password:" <<  password << "\n";
    kdDebug(JABBER_DEBUG_GLOBAL) << "serverInfo:" << serverInfo << "\n";
    kdDebug(JABBER_DEBUG_GLOBAL) << "server:" << server << "\n";
    kdDebug(JABBER_DEBUG_GLOBAL) << "port:" << port << "\n";
    kdDebug(JABBER_DEBUG_GLOBAL) << "resource:" << resource << "\n";
}

JabberAccount::~JabberAccount() {
    disconnect();

    if (jabberClient) { delete jabberClient; jabberClient = 0L; }

    /* Kick the SSL library, which seems to *suck* *horribly*. */
    Jabber::Stream::unloadSSL();

    delete actionGoOnline; 
    delete actionGoChatty; 
    delete actionGoAway; 
    delete actionGoXA; 
    delete actionGoDND; 
    delete actionGoInvisible; 
    delete actionGoOffline; 
    delete actionServices; 
    delete actionSendRaw; 
    delete actionEditVCard;
//    delete actionEmptyMail;

    delete actionStatusMenu;

    delete sendRawDialog;

    delete myContact; }

KopeteContact *JabberAccount::myself() const { return myContact; }

//void JabberAccount::setStatus(KopeteOnlineStatus status, const QString &
//reason) { }

void JabberAccount::initActions() { 

	actionGoOnline = new KAction(i18n("Online"), "jabber_online", 0, this, 
	                             SLOT(slotGoOnline()), this, "actionJabberConnect"); 

	actionGoChatty = new KAction(i18n("Free to Chat"), "jabber_chatty", 0, this, 
				     SLOT(slotGoChatty()), this, "actionJabberChatty");

	actionGoAway = new KAction(i18n("Away"), "jabber_away", 0, this,
				   SLOT(slotGoAway()), this, "actionJabberAway"); 

	actionGoXA = new KAction(i18n("Extended Away"), "jabber_away", 0, this, 
	                         SLOT(slotGoXA()), this, "actionJabberXA"); 

	actionGoDND = new KAction(i18n("Do Not Disturb"), "jabber_na", 0, this, 
	                          SLOT(slotGoDND()), this, "actionJabberDND");

	actionGoInvisible = new KAction(i18n("Invisible"), "jabber_invisible", 0, this,
				        SLOT(slotGoInvisible()), this, "actionJabberInvisible"); 

	actionGoOffline = new KAction(i18n("Offline"), "jabber_offline", 0, this, SLOT(slotGoOffline()),
				      this, "actionJabberDisconnect"); 

	actionJoinChat = new KAction(i18n("Join Groupchat..."), "filenew", 0, this, 
                             SLOT(slotJoinNewChat()), this, "actionJoinChat"); 

	actionServices = new KAction(i18n("Services..."), "filenew", 0, this, 
                             SLOT(slotGetServices()), this, "actionJabberServices"); 

	actionSendRaw = new KAction(i18n("Send Raw Packet to Server..."), "filenew", 0, this,
                            SLOT(slotSendRaw()), this, "actionJabberSendRaw"); 
	actionEditVCard = new KAction(i18n("Edit User Info..."), "identity", 0, this, 
                              SLOT(slotEditVCard()), this, "actionEditVCard");
    //actionEmptyMail = new KAction(i18n("New Email Message..."), "filenew", 0,
    //this, SLOT(slotEmptyMail()), this, "actionEmptyMail");

	
    actionStatusMenu = new KActionMenu("Jabber", this);

    // will be overwritten in slotSettingsChanged to contain the active JID
    menuTitleId = actionStatusMenu->popupMenu()->insertTitle( myself()->onlineStatus().iconFor( myself() ), "Jabber", 1);

    actionStatusMenu->insert(actionGoOnline);
    actionStatusMenu->insert(actionGoChatty);
    actionStatusMenu->insert(actionGoAway);
    actionStatusMenu->insert(actionGoXA);
    actionStatusMenu->insert(actionGoDND);
    actionStatusMenu->insert(actionGoInvisible);
    actionStatusMenu->insert(actionGoOffline);

    actionStatusMenu->popupMenu()->insertSeparator();
    actionStatusMenu->insert(actionJoinChat);

    actionStatusMenu->popupMenu()->insertSeparator();
    actionStatusMenu->insert(actionServices);
    actionStatusMenu->insert(actionSendRaw);
    actionStatusMenu->insert(actionEditVCard);

    actionStatusMenu->popupMenu()->insertSeparator();
    //actionStatusMenu->insert(actionEmptyMail);

    /* make sure we load the SSL library if required. */
    if (pluginData(protocol(), "UseSSL") == "true") {
	/* Try to load QSSL library needed by libpsi.  FIXME: this is ugly
	 * because it uses fixed dirs!  FIXME FIXME FIXME FIXME FIXME FIXME
	 * PLEASE.  should check ldconfig or something.
	 *
	 * well, the real solution is the kde ssl stuff, but that's in the
	 * future. -ds */
	QStringList dirs;

	dirs += "/usr/lib"; dirs += "/usr/local/lib"; dirs +=
	QStringList::split(":", QString(getenv("LD_LIBRARY_PATH")));

	Jabber::Stream::loadSSL(dirs); }

    /* If we need to connect on startup, do it now. */
    if (pluginData(protocol(), "AutoConnect") == "true") QTimer::singleShot(0,
    this, SLOT(connect()));

    //myContact = new JabberContact(userID, userID, QStringList(i18n("Unknown")),
    //this, 0L, QString::null); }
    }

KActionMenu *JabberAccount::actionMenu() { return actionStatusMenu; }



/*
 *  Add a contact to Meta Contact
 */
bool JabberAccount::addContactToMetaContact(const QString & contactId, const
QString & displayName, KopeteMetaContact * kmc) {
	JabberContact *jc;
	
	if (!kmc) {
		kmc = new KopeteMetaContact();
        	kdDebug(JABBER_DEBUG_GLOBAL) << "[JabberAccount] New meta contacts";
	}

	//KopeteContactList::contactList()->addMetaContact(kmc);

	jc = new JabberContact(contactId, displayName, "Unknown", this, kmc, 0L);
	//kmc->addContact(jc);
	return true;
}

void JabberAccount::errorConnectFirst() {
KMessageBox::error(qApp->mainWidget(), i18n("Please connect first"),
i18n("Error")); }


void JabberAccount::connect() { 
kdDebug(JABBER_DEBUG_GLOBAL) << "[JabberAccount] connect()" << endl;

    /* Don't do anything if we are already connected. */
    if (isConnected()) return;

    /* This is dirty but has to be done: if a previous connection attempt
     * failed, psi doesn't handle recovering too well. we are not allowed to
     * call close in the slotConnected() slot since it causes a crash, so we
     * have to delete the psi backend altogether here for safety if it still
     * exists. */
    if (jabberClient) { 
    	jabberClient->close(); 
	delete jabberClient;
    	jabberClient = 0L; 
    }

    jabberClient = new Jabber::Client(this);

    /* This should only be done here to connect the signals, otherwise it is a
     * bad idea.
     *
     * Yes. Yes it is. And you wonder why I have a pathological hatred of
     * namespaces. -DS
     */
    using namespace Jabber;

    QObject::connect(jabberClient, SIGNAL(handshaken()),
		     this, SLOT(slotHandshaken()));
    QObject::connect(jabberClient,
		     SIGNAL(authFinished(bool, int, const QString &)), this,
		     SLOT(slotConnected(bool, int, const QString &)));

    QObject::connect(jabberClient, SIGNAL(closeFinished()),
		     this, SLOT(slotDisconnected()));

    QObject::connect(jabberClient,
		     SIGNAL(subscription(const Jid &, const QString &)), this,
		     SLOT(slotSubscription(const Jid &, const QString &)));

    QObject::connect(jabberClient, SIGNAL(rosterItemAdded(const RosterItem &)),
		     this, SLOT(slotNewContact(const RosterItem &)));

    QObject::connect(jabberClient,
		     SIGNAL(rosterItemUpdated(const RosterItem &)), this,
		     SLOT(slotContactUpdated(const RosterItem &)));

    QObject::connect(jabberClient,
		     SIGNAL(rosterItemRemoved(const RosterItem &)), this,
		     SLOT(slotContactDeleted(const RosterItem &)));

    QObject::connect(jabberClient,
		     SIGNAL(resourceAvailable(const Jid &, const Resource &)),
		     this,
		     SLOT(slotResourceAvailable
			  (const Jid &, const Resource &)));

    QObject::connect(jabberClient,
		     SIGNAL(resourceUnavailable(const Jid &, const Resource &)),
		     this,
		     SLOT(slotResourceUnavailable
			  (const Jid &, const Resource &)));

    QObject::connect(jabberClient, SIGNAL(messageReceived(const Message &)),
		     this, SLOT(slotReceivedMessage(const Message &)));

    QObject::connect(jabberClient, SIGNAL(groupChatJoined(const Jid &)),
		     this, SLOT(slotGroupChatJoined(const Jid &)));

    QObject::connect(jabberClient, SIGNAL(groupChatLeft(const Jid &)),
		     this, SLOT(slotGroupChatLeft(const Jid &)));

    QObject::connect(jabberClient,
		     SIGNAL(groupChatPresence(const Jid &, const Status &)),
		     this,
		     SLOT(slotGroupChatPresence(const Jid &, const Status &)));

    QObject::connect(jabberClient,
		     SIGNAL(groupChatError(const Jid &, int, const QString &)),
		     this,
		     SLOT(slotGroupChatError
			  (const Jid &, int, const QString &)));

    QObject::connect(jabberClient, SIGNAL(error(const StreamError &)),
		     this, SLOT(slotError(const StreamError &)));

    QObject::connect(jabberClient, SIGNAL(debugText(const QString &)),
		     this, SLOT(slotPsiDebug(const QString &)));
    utsname utsBuf;
    uname(&utsBuf);

    jabberClient->setClientName("Kopete (using libpsi)");
    jabberClient->setClientVersion("KOPETE_VERSION");
    jabberClient->setOSName(QString("%1 %2").arg(utsBuf.sysname,
						 1).arg(utsBuf.release, 2));


    if (userID.isEmpty()) {
	int r = KMessageBox::warningContinueCancel(qApp->mainWidget(),
						   i18n
						   ("<qt>You have not yet specified your Jabber username. "
						    "You can specify your Jabber settings in the Kopete "
						    "configuration dialog<br>"
						    "Do you want to configure Jabber now?</qt>"),
						   i18n
						   ("Jabber Plugin Not Configured Yet"),
						   KGuiItem(i18n
							    ("C&onfigure..."),
							    "configure"));
	//if (r != KMessageBox::Cancel)
	 //   preferences->activate();
	return;
    }


    /* Check if we are capable of using SSL if requested. */
    if (pluginData(protocol(), "UseSSL") == "true") {
	bool sslPossible = jabberClient->setSSLEnabled(true);

	if (!sslPossible) {
	    KMessageBox::error(qApp->mainWidget(),
			       i18n("SSL is not supported. This is most likely "
				    "because the QSSL library could not be found."),
			       i18n("SSL Error"));
	    return;
	}
    }

    /* Parse proxy settings. */
    QString proxyTypeStr = pluginData(protocol(), "ProxyType");
    int proxyType = Jabber::StreamProxy::None;

    if (proxyTypeStr == QString("HTTPS"))
	proxyType = Jabber::StreamProxy::HTTPS;
    else {
	if (proxyTypeStr == QString("SOCKS4"))
	    proxyType = Jabber::StreamProxy::SOCKS4;
	else if (proxyTypeStr == QString("SOCKS5"))
	    proxyType = Jabber::StreamProxy::SOCKS5;
    }

    Jabber::StreamProxy proxy(proxyType, pluginData(protocol(), "ProxyName"),
			      pluginData(protocol(), "ProxyPort").toInt());

    proxy.setUseAuth(pluginData(protocol(), "ProxyAuth"));
    proxy.setUser(pluginData(protocol(), "ProxyUser"));
    proxy.setPass(pluginData(protocol(), "ProxyPass"));

    jabberClient->setProxy(proxy);

    //delete myContact;

    /* Create a contact instance for self. */
    // Why would we do this???
    //myContact =
	//	new JabberContact(QString("%1@%2").arg(userID, 1).arg(server, 2),
	//			  userID, QStringList(i18n("Unknown")), this, 0L,
	//			  QString::null);

    /* Set the title according to the new changes. */
    actionStatusMenu->popupMenu()->changeTitle(menuTitleId,
					       userID + "@" + server);


    /* Now connect. */
    QString  jidDomain = userID.section("@", userID.find("@")) + "@" + server;

    kdDebug(JABBER_DEBUG_GLOBAL) << "[JabberAccount] Connecting to Jabber server " << server << ":" << port
	                         << " with jidDomain " << jidDomain << endl;

    jabberClient->connectToHost(server, port, jidDomain);

    emit connectionAttempt();
}

void JabberAccount::slotPsiDebug(const QString & msg) {
    kdDebug(JABBER_DEBUG_PROTOCOL) << "[JabberAccount] Psi: " << msg << endl;
}

void JabberAccount::slotHandshaken() {
    kdDebug(JABBER_DEBUG_GLOBAL) << "[JabberAccount] Performing login..." <<
	endl;

    if (registerFlag) {
	Jabber::JT_Register * task =
	    new Jabber::JT_Register(jabberClient->rootTask());
	QObject::connect(task, SIGNAL(finished()), this,
			 SLOT(slotRegisterUserDone()));
	task->reg(userID, password);
	task->go(true);
    }
    else {
	if (pluginData(protocol(), "AuthType") == QString("digest"))
	    jabberClient->authDigest(userID, password, resource);
	else
	    jabberClient->authPlain(userID, password, resource);

    }

}

void JabberAccount::slotConnected(bool success, int statusCode,
				  const QString & statusString) {
    if (success) {
	kdDebug(JABBER_DEBUG_GLOBAL) << "[JabberAccount] Connected to Jabber server." << endl;

	emit connected();
	myself()->setOnlineStatus(JabberProtocol::getJabberOnline());


	kdDebug(JABBER_DEBUG_GLOBAL) << "JabberOnline: " << JabberOnline.description() << "\n" << endl;		
	kdDebug(JABBER_DEBUG_GLOBAL) << "online status: " << myself()->onlineStatus().status() << "\n" << endl;		
	kdDebug(JABBER_DEBUG_GLOBAL) << "kopete offline status: " << KopeteOnlineStatus::Offline << "\n" << endl;		
	kdDebug(JABBER_DEBUG_GLOBAL) << "isConnected()?" << isConnected() << endl;

	/* Request roster. */
	jabberClient->rosterRequest();

	/* Since we are online now, set initial presence. Don't do this
	 * before the roster request or we will receive presence
	 * information before we have updated our roster with actual
	 * contacts from the server! (libpsi won't forward presence
	 * information in that case either). */
	kdDebug(JABBER_DEBUG_GLOBAL) << "[JabberAccount] Setting Presence." << endl;
	setPresence(initialPresence, myContact->reason());

	/* Initiate anti-idle timer (will be triggered every 120
	 * seconds). */
	jabberClient->setNoopTime(120000);
    }
    else {
	kdDebug(JABBER_DEBUG_GLOBAL) <<
	    "[JabberAccount] Connection failed! Status: " << statusCode << ", "
	    << statusString << endl;
	emit disconnected();
	KMessageBox::error(qApp->mainWidget(),
			   i18n("Connection failed with reason \"%1\"").
			   arg(statusString, 1), i18n("Connection Failed"));
    }

}


/*JabberProtocol *JabberAccount::protocol() {
    return parent;
}*/

void JabberAccount::disconnect() {
    kdDebug(JABBER_DEBUG_GLOBAL) << "[JabberAccount] disconnect() called" <<
	endl;

    if (isConnected()) {
	kdDebug(JABBER_DEBUG_GLOBAL) <<
	    "[JabberAccount] Still connected, closing connection..." << endl;
	/* Tell backend class to disconnect. */
	jabberClient->close();
    }

    /* FIXME:
     * We should delete the Jabber::Client instance here,
     * but active timers in psi prevent us from doing so.
     * (in a failed connection attempt, these timers will
     * try to access an already deleted object).
     * Instead, the instance will lurk until the next
     * connection attempt.
     */
    kdDebug(JABBER_DEBUG_GLOBAL) << "[JabberAccount] Disconnected." << endl;

    emit disconnected();

    /* It seems that we don't get offline notifications when going offline
     * with the protocol, so update all contacts manually. */
    for (QDictIterator <KopeteContact> it(contacts()); it.current(); ++it)
	static_cast<JabberContact *>(*it)->slotUpdatePresence(JabberOffline, "Disconnected");
}

void JabberAccount::slotConnect() {
    connect();
}

void JabberAccount::slotDisconnect() {
    disconnect();
}

void JabberAccount::slotDisconnected() {
    kdDebug(JABBER_DEBUG_GLOBAL) <<
	"[JabberAccount] Disconnected from Jabber server." << endl;

    /* FIXME:
     * We should delete the Jabber::Client instance here,
     * but timers etc prevent us from doing so. (Psi does
     * not like to be deleted from a slot).
     */

    emit disconnected();

    /* It seems that we don't get offline notifications when going offline
     * with the protocol, so update all contacts manually. */
    for (QDictIterator <KopeteContact> it(contacts()); it.current(); ++it)
	static_cast<JabberContact *>(*it)->slotUpdatePresence(JabberOffline, "disconnected");
}

void JabberAccount::slotError(const Jabber::StreamError & error) {
    /* Determine type of error. */
    switch (error.type()) {
    case Jabber::StreamError::DNS:
	KMessageBox::error(qApp->mainWidget(),
			   i18n
			   ("Connection to the Jabber server %1 for account %2 failed due to a DNS error (%1); check you typed the server name correctly.").
			   arg(server, 1).arg(userID, 2).arg(error.details(),
							     3),
			   i18n("Error Connecting to Jabber Server"));
	break;

    case Jabber::StreamError::Refused:
	KMessageBox::error(qApp->mainWidget(),
			   i18n
			   ("The connection was refused when attempting to contact the server %1 for the account %2; check both the server name and the port number.").
			   arg(server, 1).arg(userID, 2),
			   i18n("Error Connecting to Jabber Server"));
	break;

    case Jabber::StreamError::Timeout:
	KMessageBox::error(qApp->mainWidget(),
			   i18n
			   ("The connection to the Jabber server %1 for the account %2 timed out.").
			   arg(server, 1).arg(userID, 2),
			   i18n("Error Connecting to Jabber Server"));
	break;

    case Jabber::StreamError::Socket:
	KMessageBox::error(qApp->mainWidget(),
			   i18n
			   ("There was a socket error (%1); your connection to the Jabber server %2 for account %3 has been lost.").
			   arg(error.details(), 1).arg(server, 2).arg(userID,
								      3),
			   i18n("Error Connecting to Jabber Server"));
	break;

    case Jabber::StreamError::Disconnected:
	KMessageBox::error(qApp->mainWidget(),
			   i18n
			   ("The remote server %1 closed the connection for account %2, without specifying any error. This usually means that the server is broken.").
			   arg(server, 1).arg(userID, 2),
			   i18n("Error Connecting to Jabber Server"));
	break;

    case Jabber::StreamError::Handshake:
	KMessageBox::error(qApp->mainWidget(),
			   i18n
			   ("Connection to the Jabber server failed due to the handshake failing (%1); check that you typed your Jabber ID and password. Note that the Jabber ID now needs to be done in full user@domain form, not just the username.").
			   arg(error.details(), 1),
			   i18n("Error Connecting to Jabber Server"));
	break;

    case Jabber::StreamError::SSL:
	KMessageBox::error(qApp->mainWidget(),
			   i18n
			   ("Connection to the Jabber server failed due to a SSL error (%1); this usually means that the server's SSL implementation is broken.").
			   arg(error.details(), 1),
			   i18n("Error Connecting to Jabber Server"));
	break;

    case Jabber::StreamError::Proxy:
	KMessageBox::error(qApp->mainWidget(),
			   i18n
			   ("Connection to the Jabber server failed due to a proxy error (%1).").
			   arg(error.details(), 1),
			   i18n("Error Connecting to Jabber Server"));
	break;

    case Jabber::StreamError::Unknown:
    default:
	KMessageBox::error(qApp->mainWidget(),
			   i18n
			   ("An unknown error was encountered (%1); please report this error to kopete-devel@kde.org, along with what you were doing at the time.").
			   arg(error.details(), 1),
			   i18n("Error Connecting to Jabber Server"));
	break;
    }

    disconnect();
}

/* Set presence (usually called by dialog widget). */
void JabberAccount::setPresence(const KopeteOnlineStatus & status,
				const QString & reason, int priority) {
    if (isConnected()) {
	kdDebug(JABBER_DEBUG_GLOBAL) << "[JabberAccount] Connected & Setting Presence." << endl;
	Jabber::Status presence;

	presence.setPriority(priority);
	presence.setStatus(reason);
	presence.setIsAvailable(true);

	if (status == JabberProtocol::getJabberOnline())
	    presence.setShow("");
	else if (status == JabberProtocol::getJabberChatty())
	    presence.setShow("chat");
	else if (status == JabberProtocol::getJabberAway())
	    presence.setShow("away");
	else if (status == JabberProtocol::getJabberXA())
	    presence.setShow("xa");
	else if (status == JabberProtocol::getJabberDND())
	    presence.setShow("dnd");
	else if (status == JabberProtocol::getJabberInvisible())
	    presence.setIsInvisible(true);
	else {
	    kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo <<
		"Unknown presence status, " << "ignoring (status == " << status.
		description() << ")" << endl;
	    return;
	}

	kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Updating presence to \""
	    << presence.status()
	    << "\" with reason \"" << reason << endl;
	myContact->slotUpdatePresence(status, reason);

	Jabber::JT_Presence * task =
	    new Jabber::JT_Presence(jabberClient->rootTask());

	task->pres(presence);
	task->go(true);
    }
}

void JabberAccount::setAway( bool away, const QString &reason = QString::null ) {
   kdDebug(JABBER_DEBUG_GLOBAL) << "[JabberAccount] Setting away mode." << endl;
   setPresence(JabberProtocol::getJabberAway(), KopeteAway::getInstance()->message());
}

void JabberAccount::setAvailable(void) {
    kdDebug(JABBER_DEBUG_GLOBAL) <<
	"[JabberAccount] Coming back from away mode." << endl;
    slotGoOnline();
}

void JabberAccount::deserializeContact(KopeteMetaContact * metaContact,
				       const QMap < QString,
				       QString > &serializedData,
				       const QMap < QString,
				       QString > & /* addressBookData */ ) {
    kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Deserializing data for metacontact " 
                                 << metaContact->displayName() << endl;

    new JabberContact(serializedData["contactId"], serializedData["displayName"], 
                      QStringList::split(',', serializedData ["groups"]),
		      this, metaContact, serializedData["identityId"]);

}


// Appears to be in kopeteprotocol as virutal
//AddContactPage *JabberAccount::createAddContactWidget(QWidget * parent) {
 //   return new JabberAddContactPage(this, parent);
//}

void JabberAccount::slotGoOnline() {
    kdDebug(JABBER_DEBUG_GLOBAL) << "[JabberAccount] Going online!" << endl;

    if (!isConnected()) {
    	kdDebug(JABBER_DEBUG_GLOBAL) << "Trying to go online!" << endl;
	/* We are not connected yet, so connect now. */
	initialPresence = JabberProtocol::getJabberOnline();
	connect();
    }

    setPresence(JabberProtocol::getJabberOnline(), "");
}

void JabberAccount::slotGoOffline() {
    kdDebug(JABBER_DEBUG_GLOBAL) << "[JabberAccount] Going offline." << endl;

    disconnect();
}

void JabberAccount::slotGoChatty() {
    kdDebug(JABBER_DEBUG_GLOBAL) << "[JabberAccount] Setting 'chatty' mode." <<
	endl;

    if (!isConnected()) {
	/* We are not connected yet, so connect now. */
	initialPresence = JabberProtocol::getJabberChatty();
	connect();
    }

    setPresence(JabberProtocol::getJabberChatty(), "");

}

void JabberAccount::slotGoAway() {
    kdDebug(JABBER_DEBUG_GLOBAL) << "[JabberAccount] Setting away mode." <<
	endl;

    if (!isConnected()) {
	/* We are not connected yet, so connect now. */
	initialPresence = JabberProtocol::getJabberAway();
	connect();
    }

    /* Kill old reason dialog if it's still in memory. */
    if (reasonDialog != 0L)
	delete reasonDialog;

    /* TODO: Fix this to work with KopeteAwayDialog. */
    reasonDialog = new dlgJabberStatus(this, JabberAway, qApp->mainWidget());
}

void JabberAccount::slotGoXA() {
    kdDebug(JABBER_DEBUG_GLOBAL) <<
	"[JabberAccount] Setting extended away mode." << endl;

    if (!isConnected()) {
	/* We are not connected yet, so connect now. */
	initialPresence = JabberProtocol::getJabberXA();
	connect();
    }

    /* Kill old reason dialog if it's still in memory. */
    if (reasonDialog != 0L)
	delete reasonDialog;

    /* TODO: Fix this to work with KopeteAwayDialog. */
    reasonDialog = new dlgJabberStatus(this, JabberXA, qApp->mainWidget());
}

void JabberAccount::slotGoDND() {
    kdDebug(JABBER_DEBUG_GLOBAL) <<
	"[JabberAccount] Setting do not disturb mode." << endl;

    if (!isConnected()) {
	/* We are not connected yet, so connect now. */
	initialPresence = JabberProtocol::getJabberDND();
	connect();
    }

    /* Kill old reason dialog if it's still in memory. */
    if (reasonDialog != 0L)
	delete reasonDialog;

    /* TODO: Fix this to work with KopeteAwayDialog. */
    reasonDialog = new dlgJabberStatus(this, JabberDND, qApp->mainWidget());

}

void JabberAccount::slotGoInvisible() {
    kdDebug(JABBER_DEBUG_GLOBAL) << "[JabberAccount] Setting invisible mode." <<
	endl;

    if (!isConnected()) {
	/* We are not connected yet, so connect now. */
	initialPresence = JabberProtocol::getJabberInvisible();
	connect();
    }

    setPresence(JabberProtocol::getJabberInvisible());
}

void JabberAccount::slotSendRaw() {
    /* Check if we're connected. */
    if (!isConnected()) {
	errorConnectFirst();
	return;
    }

    /* Check if we have a dialog already displayed. */
    if (sendRawDialog == 0L) {
	sendRawDialog = new dlgJabberSendRaw(jabberClient, qApp->mainWidget());
    }
    sendRawDialog->show();
}

void JabberAccount::subscribe(const Jabber::Jid & jid) {

    if (!isConnected()) {
	errorConnectFirst();
	return;
    }

    Jabber::JT_Presence * task =
	new Jabber::JT_Presence(jabberClient->rootTask());

    task->sub(jid, "subscribe");
    task->go(true);
}

void JabberAccount::subscribed(const Jabber::Jid & jid) {
    if (!isConnected()) {
	errorConnectFirst();
	return;
    }

    Jabber::JT_Presence * task =
	new Jabber::JT_Presence(jabberClient->rootTask());

    task->sub(jid, "subscribed");
    task->go(true);
}

void JabberAccount::sendPresenceToNode(const KopeteOnlineStatus & pres,
				       const QString & userID) {

    if (!isConnected()) {
	errorConnectFirst();
	return;
    }

    Jabber::JT_Presence * task =
	new Jabber::JT_Presence(jabberClient->rootTask());

    Jabber::Jid jid(userID);
    Jabber::Status status;

    if (pres == JabberOnline)
	status.setShow("");
    else if (pres == JabberChatty)
	status.setShow("chat");
    else if (pres == JabberAway)
	status.setShow("away");
    else if (pres == JabberXA)
	status.setShow("xa");
    else if (pres == JabberDND)
	status.setShow("dnd");
    else if (pres == JabberInvisible) {
	status.setShow("away");
	status.setIsInvisible(true);
    }
    else
	status.setShow("away");

    task->pres(jid, status);
    task->go(true);
}

JabberContact *JabberAccount::createContact(const QString & jid,
					    const QString & alias,
					    const QStringList & groups,
					    KopeteMetaContact * metaContact,
					    const QString & identity) {

    JabberContact *jc =
	new JabberContact(jid, alias, groups, this, metaContact, identity);
    //metaContact->addContact(jc);
    return jc;
}


void JabberAccount::createAddContact(KopeteMetaContact * mc,
				     const Jabber::RosterItem & item) {


	
	kdDebug(JABBER_DEBUG_GLOBAL) << "findContact " << protocol()->pluginId() << " "
				     << myContact->contactId() << " " << item.jid().userHost() << "\n" << endl;

    if (!mc) {
	mc = KopeteContactList::contactList()->findContact(protocol()->pluginId(),
							   myContact->contactId(),
							   item.jid().userHost());

	if (mc) {
	    JabberContact *jc = (JabberContact *) mc->findContact(protocol()->pluginId(),
								  myContact->
								  contactId(),
								  item.jid().
								  userHost());

	    if (jc) {
		/* Existing contact, update data. */
		kdDebug(JABBER_DEBUG_GLOBAL) << "[JabberAccount] Contact "
		    << item.jid().userHost()
		    << " already exists, updating" << endl;
		jc->slotUpdateContact(item);
		return;
	    }
	    else {
		kdDebug(JABBER_DEBUG_GLOBAL) <<
		    "[JabberAccount]****Warning**** : " << item.jid().userHost()
		    << " already exists, and can be found" << endl;
	    }
	}
    }

    kdDebug(JABBER_DEBUG_GLOBAL) << "[JabberAccount] Adding contact "
	<< item.jid().userHost()
	<< " ..." << endl;

    bool isContactInList = true;

    if (!mc) {
	isContactInList = false;
	mc = new KopeteMetaContact();
	QStringList groups = item.groups();
	for (QStringList::Iterator it = groups.begin(); it != groups.end();
	     ++it)
	    mc->addToGroup(KopeteContactList::contactList()->getGroup(*it));
    }

    QString contactName;

    if (item.name().isNull() || item.name().isEmpty())
	contactName = item.jid().userHost();
    else
	contactName = item.name();

    createContact(item.jid().userHost(), contactName,
		  item.groups(), mc, myContact->contactId());

    if (!isContactInList)
	KopeteContactList::contactList()->addMetaContact(mc);
}

void JabberAccount::slotSubscription(const Jabber::Jid & jid,
				     const QString & type) {
    kdDebug(JABBER_DEBUG_GLOBAL) << "[JabberAccount] slotSubscription("
	<< jid.userHost() << ", " << type << ");" << endl;

    if (type == "subscribe") {
	/* A new user wants to subscribe. */
	kdDebug(JABBER_DEBUG_GLOBAL) << "[JabberAccount] slotSubscription(): "
	    << jid.userHost()
	    << " is asking for authorization to subscribe." << endl;

	switch (KMessageBox::questionYesNoCancel(qApp->mainWidget(),
						 i18n
						 ("The Jabber user %1 wants to add you to their "
						  "contact list; do you want to authorize them? "
						  "Selecting Cancel will ignore the request.").
						 arg(jid.userHost(), 1),
						 i18n("Authorize Jabber User?"),
						 i18n("Authorize"),
						 i18n("Deny"))) {
	    Jabber::JT_Presence * task;
	    KopeteMetaContact *mc;
	case KMessageBox::Yes:
	    /* Authorize user. */
	    subscribed(jid);

	    /* Is the user already in our contact list? */
	    mc = KopeteContactList::contactList()->findContact(protocol()->pluginId(),
							       myContact->
							       contactId(),
							       jid.userHost());

	    /* If it is not, ask the user if he wants to subscribe in return. */
	    if (!mc && (KMessageBox::questionYesNo(qApp->mainWidget(),
						   i18n
						   ("Do you want to add %1 to your contact "
						    "list in return?").arg(jid.
									   userHost
									   (),
									   1),
						   i18n("Add Jabber User?"))
			== KMessageBox::Yes))
		/* Subscribe to user's presence. */
		subscribe(jid);
	    break;

	case KMessageBox::No:
	    /* Reject subscription. */
	    task = new Jabber::JT_Presence(jabberClient->rootTask());

	    task->sub(jid, "unsubscribed");
	    task->go(true);

	    break;

	case KMessageBox::Cancel:
	    /* Leave the user in the dark. */
	    break;
	}

    }
    else if (type == "unsubscribed") {
	/* Someone else removed us from their roster. */
	kdDebug(JABBER_DEBUG_GLOBAL) << "[JabberAccount] " << jid.userHost()
	    << " deleted auth!" << endl;

	KMessageBox::information(0L,
				 i18n
				 ("The Jabber user %1 removed %2's subscription to them. This account will no longer be able to view their online/offline status.").
				 arg(jid.userHost(), 1).arg(userID, 2),
				 i18n("Notification"));

	/* Delete the item from the roster. */
	Jabber::JT_Roster * task =
	    new Jabber::JT_Roster(jabberClient->rootTask());

	task->remove(jid);
	task->go(true);
    }
}

void JabberAccount::slotNewContact(const Jabber::RosterItem & item) {
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

    QString debugStr =
	"[JabberAccount] New Contact " + item.jid().userHost() +
	" (Subscription::";

    switch (item.subscription().type()) {
    case Jabber::Subscription::Both:	// both sides can see the contact
	debugStr += "Both | <->";
	break;

    case Jabber::Subscription::From:	// he can see us
	debugStr += "From | <--";
	break;

    case Jabber::Subscription::To:	// we can see him
	debugStr += "To | -->";
	break;

    case Jabber::Subscription::None:	// waiting for authorization
	debugStr += "None | ---";
	break;
    }

    debugStr += ") " + item.ask();

    kdDebug(JABBER_DEBUG_GLOBAL) << debugStr << endl;

    /* Add the contact to the GUI. */
    createAddContact(0L, item);
}

void JabberAccount::slotContactDeleted(const Jabber::RosterItem & item) {
    kdDebug(JABBER_DEBUG_GLOBAL) << "[JabberAccount] Deleting contact "
	<< item.jid().userHost() << endl;

    if (!contacts()[item.jid().userHost()]) {
	kdDebug(JABBER_DEBUG_GLOBAL) <<
	    "[JabberProtocl] WARNING: slotContactDeleted() " <<
	    "was asked to delete a non-existing contact." << endl;
	return;
    }

    JabberContact *jc =
	static_cast<JabberContact *>(contacts()[item.jid().userHost()]);

    /* This will also cause the contact to disappear from the metacontact. */
    delete jc;
}

void JabberAccount::slotContactUpdated(const Jabber::RosterItem & item) {
    kdDebug(JABBER_DEBUG_GLOBAL) << "[JabberAccount] Status update for "
	<< item.jid().userHost() << endl;

    /* Sanity check. */
    if (!contacts()[item.jid().userHost()]) {
	kdDebug(JABBER_DEBUG_GLOBAL) <<
	    "[JabberAccount] WARNING: slotContactUpdated() " <<
	    "was requested to update a non-existing contact." << endl;
	return;
    }
    // update the contact data
    static_cast<JabberContact *>(contacts()[item.jid().userHost()])->slotUpdateContact(item);
}

void JabberAccount::slotSettingsChanged() {
    /* Set the title to the current account. */
    actionStatusMenu->popupMenu()->changeTitle(menuTitleId, userID);

}

void JabberAccount::slotReceivedMessage(const Jabber::Message & message) {
    QString userHost;
    JabberContact *contactFrom;

    userHost = message.from().userHost();
    contactFrom = static_cast<JabberContact *>(contacts()[userHost]);

    if (userHost.isEmpty()) {
	/* If the sender field is empty, it is a server message.
	 *
	 * When I wrote it, this made sense, but should it be displayed
	 * in a KopeteEmailWindow now? -DS */
	kdDebug(JABBER_DEBUG_GLOBAL) <<
	    "[JabberAccount] New server message for us!" << endl;

	KMessageBox::information(qApp->mainWidget(), message.body(),
				 i18n("Jabber: Server Message"));
    }
    else {
	kdDebug(JABBER_DEBUG_GLOBAL) << "[JabberAccount] New message from '"
	    << userHost << "'" << endl;

	/* See if the contact is actually in our roster. */
	if (!contactFrom) {
	    /* So, either it's a group chat, or we're not subscribed
	     * to them. Either way. */
	    kdDebug(JABBER_DEBUG_GLOBAL) <<
		"[JabberAccount] Message received from an " <<
		"unknown contact, creating temporary contact." << endl;

	    KopeteMetaContact *metaContact = new KopeteMetaContact();
	    metaContact->setTemporary(true);

	    contactFrom =
		createContact(userHost, userHost, QStringList(), metaContact,
			      myContact->contactId());

	    KopeteContactList::contactList()->addMetaContact(metaContact);
	}

	/* Pass the message on to the contact. */
	contactFrom->slotReceivedMessage(message);
    }

}

void JabberAccount::slotGetOneShotRecipient() {
    if (!isConnected()) {
	errorConnectFirst();
	return;
    }

    KLineEditDlg *dlg =
	new KLineEditDlg(i18n("Please enter a recipient:"), QString::null,
			 qApp->mainWidget());

    QObject::connect(dlg, SIGNAL(okClicked()), this, SLOT(slotNewOneShot()));

    dlg->show();
    dlg->raise();
}

void JabberAccount::slotNewOneShot() {
    if (!sender()) {
	slotGetOneShotRecipient();
    }

    QString userHost = ((KLineEditDlg *) sender())->text();

    if (!userHost.isEmpty() && !userHost.isNull()) {
	JabberContact *contact =
	    static_cast<JabberContact *>(contacts()[userHost]);

	/* If the contact is not in our contact list yet, create a temporary one. */
	if (!contact) {
	    KopeteMetaContact *metaContact = new KopeteMetaContact();
	    metaContact->setTemporary(true);

	    contact =
		createContact(userHost, userHost, QStringList(), metaContact,
			      myContact->contactId());
	    KopeteContactList::contactList()->addMetaContact(metaContact);
	}

	contact->manager(true)->view(true, KopeteMessage::Email);
    }
}

void JabberAccount::slotJoinNewChat() {
    if (!isConnected()) {
	errorConnectFirst();
	return;
    }

    dlgJabberChatJoin *dlg = new dlgJabberChatJoin(qApp->mainWidget());
    dlg->show();
    dlg->raise();
}

void JabberAccount::slotGroupChatJoined(const Jabber::Jid & jid) {
    kdDebug(JABBER_DEBUG_GLOBAL) << "[JabberAccount] Joined group chat " << jid.
	full() << endl;

    /* Create new meta contact that holds the group chat contact. */
    KopeteMetaContact *mc = new KopeteMetaContact();
    mc->setTemporary(true);

    /* The group chat object basically works like a JabberContact. */
    JabberGroupChat *groupChat =
	new JabberGroupChat(jid, QStringList(), this, mc, myContact->contactId());

    /* Add the group chat class to the meta contact. */
    mc->addContact(groupChat);

    KopeteContactList::contactList()->addMetaContact(mc);
}

void JabberAccount::slotGroupChatLeft(const Jabber::Jid & jid) {
    kdDebug(JABBER_DEBUG_GLOBAL) << "[JabberAccount] Left groupchat " << jid.
	full() << endl;
    delete static_cast<JabberGroupChat *>(contacts()[jid.userHost()]);
}

void JabberAccount::slotGroupChatPresence(const Jabber::Jid & jid,
					  const Jabber::Status & status) {
    kdDebug(JABBER_DEBUG_GLOBAL) <<
	"[JabberAccount] Received groupchat presence for room " << jid.
	full() << endl;
    static_cast<JabberGroupChat *>(contacts()[jid.userHost()])->updatePresence(jid, status);
}

void JabberAccount::slotGroupChatError(const Jabber::Jid & jid, int error,
				       QString & reason) {
    /* FIXME: Present this to the user, damnit! */
    kdDebug(JABBER_DEBUG_GLOBAL) << "[JabberAccount] Group chat error - room "
	<< jid.
	userHost() << " had error " << error << " (" << reason << ")!" << endl;
}

void JabberAccount::slotResourceAvailable(const Jabber::Jid & jid,
					  const Jabber::Resource & resource) {

    kdDebug(JABBER_DEBUG_GLOBAL) <<
	"[JabberAccount] New resource available for " << jid.userHost() << endl;

    if (!contacts()[jid.userHost()]) {
	kdDebug(JABBER_DEBUG_GLOBAL) <<
	    "[JabberAccount] Trying to add a resource, but " <<
	    "couldn't find an entry for " << jid.userHost()
	    << endl;
	return;
    }

    static_cast<JabberContact *>(contacts()[jid.userHost()])->slotResourceAvailable(jid, resource);
}

void JabberAccount::slotResourceUnavailable(const Jabber::Jid & jid,
					    const Jabber::Resource & resource) {

    kdDebug(JABBER_DEBUG_GLOBAL) <<
	"[JabberAccount] Resource now unavailable for " << jid.
	userHost() << endl;

    if (!contacts()[jid.userHost()]) {
	kdDebug(JABBER_DEBUG_GLOBAL) <<
	    "[JabberAccount] Trying to remove a resource, " <<
	    "but couldn't find an entry for " << jid.userHost() << endl;
	return;
    }

    static_cast<JabberContact *>(contacts()[jid.userHost()])->slotResourceUnavailable(jid, resource);
}

void JabberAccount::slotEditVCard() {
    myContact->slotEditVCard();
}

void JabberAccount::registerUser() {
    kdDebug(JABBER_DEBUG_GLOBAL) << "[JabberAccount] Registering user " <<
	userID << " on server " << server << "." << endl;

    /* Save the current preferences. */
    //preferences->save();

    /* Set the flag to register an account during registration. */
    registerFlag = 1;

    /* Now connect, initiating the registration. */
    kdDebug(JABBER_DEBUG_GLOBAL) << "[JabberAccount] Register: Connect() " << endl;
    connect();
}

void JabberAccount::slotRegisterUserDone() {
    Jabber::JT_Register * task = (Jabber::JT_Register *) sender();

    if (task->success())
	KMessageBox::information(qApp->mainWidget(),
				 i18n("Account successfully registered."),
				 i18n("Account Registration"));
    else {
	KMessageBox::information(qApp->mainWidget(),
				 i18n
				 ("Unable to create account on the server."),
				 i18n("Account Registration"));

	disconnect();
    }
    registerFlag = 0;
}

/*void JabberAccount::addContact(KopeteMetaContact * mc, const QString & userID) {
    /* First of all, create a Jabber::RosterItem from the information found
     * in the KopeteMetaContact. 

    Jabber::RosterItem item;
    KopeteGroupList groupList;
    QStringList groupStringList;

    item.setJid(Jabber::Jid(userID));
    item.setName(userID);

    groupList = mc->groups();
    for (KopeteGroup * g = groupList.first(); g; g = groupList.next())
	groupStringList.append(g->displayName());
    item.setGroups(groupStringList);

    createAddContact(mc, item);

     Add the new contact to our roster. 
    Jabber::JT_Roster * rosterTask =
	new Jabber::JT_Roster(jabberClient->rootTask());

    rosterTask->set(item.jid(), item.name(), item.groups());
    rosterTask->go(true);

     Send a subscription request. 
    subscribe(item.jid());
}*/

void JabberAccount::updateContact(const Jabber::RosterItem & item) {
    if (!isConnected()) {
	errorConnectFirst();
	return;
    }

    Jabber::JT_Roster * rosterTask =
	new Jabber::JT_Roster(jabberClient->rootTask());

    rosterTask->set(item.jid(), item.name(), item.groups());
    rosterTask->go(true);
}

void JabberAccount::removeContact(const Jabber::RosterItem & item) {
    if (!isConnected()) {
	errorConnectFirst();
	return;
    }

    Jabber::JT_Roster * rosterTask =
	new Jabber::JT_Roster(jabberClient->rootTask());

    rosterTask->remove(item.jid());
    rosterTask->go(true);
}

QString JabberAccount::getResource() {
	return resource;
}

QString JabberAccount::getServer() {
	return server;
}

int JabberAccount::getPort() {
	return port;
}

void JabberAccount::setResource(QString r) {
	resource = r;
}

void JabberAccount::setServer(QString s) {
	server = s;
}

void JabberAccount::setPort(int p) {
	port = p;
}

void JabberAccount::slotGetServices() {
    dlgJabberServices *dialog = new dlgJabberServices();

    dialog->show();
    dialog->raise();
}

#include "jabberaccount.moc"
