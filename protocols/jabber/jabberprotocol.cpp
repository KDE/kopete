/***************************************************************************
                          jabberprotocol.cpp  -  Jabber Plugin
                             -------------------
    begin                : Fri Apr 12 2002
    copyright            : (C) 2002 by Daniel Stone
    email                : dstone@kde.org
 ***************************************************************************

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *                                                                         *
 *   Kudos to my lappy for letting me hack on the way to Sunbury, and CBV  *
 *   for lending it to me. :)                                              *
 *																		   *
 ***************************************************************************/

#include <qcursor.h>
#include <qmap.h>

#include <kdebug.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <ksimpleconfig.h>
#include <kstandarddirs.h>

#include "kopete.h"
#include "jabberprotocol.h"
#include "jabcommon.h"
#include "statusbaricon.h"
#include "dlgjabberstatus.h"

JabberProtocol::JabberProtocol():QObject(0, "JabberProtocol"),
KopeteProtocol()
{
    kdDebug() << "Jabber plugin: Loading ..." << endl;

    initIcons();
    initActions();

    authContact = new KMessageBox;
	reasonDialog = 0L;
    protocol = new Jabber;
	connect(protocol, SIGNAL(connected()), this, SLOT(slotConnected()));
	connect(protocol, SIGNAL(disconnected()), this, SLOT(slotDisconnected()));
	connect(protocol, SIGNAL(contactNew(JabRosterEntry *)), this, SLOT(slotNewContact(JabRosterEntry *)));
	connect(protocol, SIGNAL(contactChanged(JabRosterEntry *)), this, SLOT(slotContactUpdated(JabRosterEntry *)));
	connect(protocol, SIGNAL(resourceAvailable(const Jid &, const JabResource &)), this, SIGNAL(resourceAvailable(const Jid &, const JabResource &)));
	connect(protocol, SIGNAL(resourceUnavailable(const Jid &)), this, SIGNAL(resourceUnavailable(const Jid &)));
	connect(protocol, SIGNAL(authRequest(const Jid &)), this, SLOT(slotUserWantsAuth(const Jid &)));
	connect(protocol, SIGNAL(messageReceived(const JabMessage &)), this, SLOT(slotNewMessage(const JabMessage &)));
	connect(protocol, SIGNAL(error(JabError *)), this, SLOT(slotError(JabError *)));

    statusBarIcon = new StatusBarIcon();
    QObject::connect(statusBarIcon, SIGNAL(rightClicked(const QPoint)), this, SLOT(slotIconRightClicked(const QPoint)));
    statusBarIcon->setPixmap(offlineIcon);

    mPrefs = new JabberPreferences("jabber_protocol_32", this);
    connect(mPrefs, SIGNAL(saved(void)), this, SLOT(slotSettingsChanged(void)));

    KGlobal::config()->setGroup("Jabber");
    if ((KGlobal::config()->readEntry("UserID", "") == "")
	     || (KGlobal::config()->readEntry("Password", "") == "")) {
		QString emptyText = i18n("<qt>If you have a Jabber account, please \
		configure it in the Kopete Settings. If you don't, you can register \
		in the settings.</qt>");
		QString emptyCaption = i18n("No Jabber Configuration found!");
		KMessageBox::error(kopeteapp->mainWindow(), emptyText, emptyCaption);
    }

	mIsConnected = false;
	doRegister = false;
	/* Call slotSettingsChanged() to get it all registered. */
	slotSettingsChanged();
   
	if (KGlobal::config()->readBoolEntry("AutoConnect", "0"))
		Connect();
}

JabberProtocol::~JabberProtocol()
{
}

void JabberProtocol::init()
{
}

bool JabberProtocol::unload()
{
    kdDebug() << "Jabber plugin: Unloading...\n";
    if (kopeteapp->statusBar()) {
		kopeteapp->statusBar()->removeWidget(statusBarIcon);
		delete statusBarIcon;
    }

    emit protocolUnloading();
    return true;
}

void JabberProtocol::Connect()
{
    if (!isConnected()) {
		kdDebug() << "Jabber plugin: Attempting to connect to Jabber server "
			<< mServer << ":" << mPort << endl;
		kdDebug() << "Jabber plugin: Using UserID " << mUsername <<
		    " with password " << mPassword << endl;
		KGlobal::config()->setGroup("Jabber");
		protocol->setSSLEnabled(KGlobal::config()->readBoolEntry("UseSSL", "0"));
		if (doRegister) {
			kdDebug() << "Jabber plugin: Registering user for the first time! Welcome to Jabber :)" << endl;
			protocol->accRegister();
		}
		else {
			kdDebug() << "Jabber plugin: Nope, just another day at the off^W^W^Wnormal login." << endl;
			protocol->login(mStatus, "", 1, true);
		}
		slotConnecting();
    } 
	else if (isAway()) {	/* They're really away, and they want to un-away. */
		slotGoOnline();
    } 
	else {			/* Nope, just your regular crack junky. */
		kdDebug() << "Jabber plugin: Ignoring connect request (already connected)." << endl;
    }
}

void JabberProtocol::Disconnect()
{
    if (isConnected()) {
		protocol->disc();
		kdDebug() << "Jabber plugin: Disconnected." << endl;
		mIsConnected = false;
		statusBarIcon->setPixmap(offlineIcon);
		emit nukeContacts(false);
    } 
	else {			/* Again, what's with the crack? Sheez. */
		kdDebug() << "Jabber plugin: Ignoring disconnect request (not connected)." << endl;
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

void JabberProtocol::slotConnecting()
{				/* Aw, look at the cute widdle MNG. */
    statusBarIcon->setMovie(connectingIcon);
}

void JabberProtocol::slotConnected()
{
    mIsConnected = true;
    kdDebug() << "Jabber plugin: Connected to Jabber server." << endl;
	if (mStatus == STATUS_AWAY || mStatus == STATUS_XA) { statusBarIcon->setPixmap(awayIcon); }
	else if (mStatus == STATUS_DND) { statusBarIcon->setPixmap(naIcon); }
	else { statusBarIcon->setPixmap(onlineIcon); }
	myContact = new JabberContact(QString("%1@%2").arg(mUsername, 1).arg(mServer, 2), mUsername, i18n("Unknown"), this);
}

void JabberProtocol::slotDisconnected()
{
    mIsConnected = false;
	kdDebug() << "Jabber plugin: Disconnected from Jabber server." << endl;
	statusBarIcon->setPixmap(offlineIcon);
	emit nukeContacts(false);
}

void JabberProtocol::slotError(JabError *error) { /* "Bugger." */
	switch(error->type) {
		case JABERR_CONNECT:
			KMessageBox::error(kopeteapp->mainWindow(), i18n("Sorry, but there was an error connecting to the Jabber server (%1).").arg(error->msg, 1), i18n("Error connecting to Jabber server"));
		case JABERR_AUTH:   /* FIXME FIXME FIXME FIXME!!! */
		case JABERR_CREATE: /* Isn't red-on-orange just so PRETTY? */
		default: /* o/~ cause i'm that fool who broke the key/i'm unlockable so don't check me/
					    i got weight on my shoulders and things on my mind/ the sky is falling, and i'm falling behind */
			KMessageBox::error(kopeteapp->mainWindow(), i18n("Sorry, but you were disconnected for an unspecified reason (%1).").arg(error->type, 1), i18n("Disconnected from Jabber server"));
	}
}

bool JabberProtocol::isConnected() const
{
    return mIsConnected;
}

void JabberProtocol::setAway(void)
{
    setPresence(STATUS_AWAY, "Automatically set away due to being idle.");
}

void JabberProtocol::setAvailable(void)
{
    slotGoOnline();
}

bool JabberProtocol::isAway(void) const
{
	kdDebug() << "Jabber plugin: isAway: " << protocol->status() << endl;
	if (isConnected()) {
		return (protocol->status() == STATUS_AWAY || protocol->status() == STATUS_XA || protocol->status() == STATUS_DND);
    }
	else {
		return false; /* Well, technically we're not away, but then again, we're not online either, so this
					   * statistic isn't *really* that useful. */
    }
}

QString JabberProtocol::protocolIcon() const
{
    return "jabber_protocol_32";
}

AddContactPage *JabberProtocol::createAddContactWidget(QWidget *parent)
{
    return new JabberAddContactPage(this, parent);
}

void JabberProtocol::initIcons()
{
    KIconLoader *loader = KGlobal::iconLoader();
    KStandardDirs dir;

    onlineIcon = QPixmap(loader->loadIcon("jabber_online", KIcon::User));
    offlineIcon = QPixmap(loader->loadIcon("jabber_offline", KIcon::User));
    awayIcon = QPixmap(loader->loadIcon("jabber_away", KIcon::User));
    naIcon = QPixmap(loader->loadIcon("jabber_na", KIcon::User));
    connectingIcon =
	QMovie(dir.
	       findResource("data", "kopete/pics/jabber_connecting.mng"));
}

void JabberProtocol::initActions()
{
    actionGoOnline =
	new KAction(i18n("Online"), "jabber_online", 0, this,
		    SLOT(slotConnect()), this, "actionJabberConnect");
    actionGoAway =
	new KAction(i18n("Away"), "jabber_away", 0, this,
		    SLOT(slotSetAway()), this, "actionJabberConnect");
    actionGoXA =
	new KAction(i18n("Extended Away"), "jabber_away", 0, this,
		    SLOT(slotSetXA()), this, "actionJabberConnect");
    actionGoDND =
	new KAction(i18n("Do Not Disturb"), "jabber_na", 0, this,
		    SLOT(slotSetDND()), this, "actionJabberConnect");
    actionGoOffline =
	new KAction(i18n("Offline"), "jabber_offline", 0, this,
		    SLOT(slotDisconnect()), this, "actionJabberConnect");
    actionStatusMenu = new KActionMenu("Jabber", this);
    actionStatusMenu->insert(actionGoOnline);
    actionStatusMenu->insert(actionGoAway);
    actionStatusMenu->insert(actionGoXA);
    actionStatusMenu->insert(actionGoDND);
    actionStatusMenu->insert(actionGoOffline);
    actionStatusMenu->plug(kopeteapp->systemTray()->contextMenu(), 1);
}

void JabberProtocol::slotGoOnline()
{
    kdDebug() << "Jabber plugin: Going online!" << endl;
    if (!isConnected()) {
		mStatus = STATUS_ONLINE;
		Connect();
    }
	else {
		setPresence(STATUS_ONLINE, "");
    }
    statusBarIcon->setPixmap(onlineIcon);
}

void JabberProtocol::slotGoOffline()
{
    kdDebug() << "Jabber plugin: Going offline." << endl;
    Disconnect();
    statusBarIcon->setPixmap(offlineIcon);
}

void JabberProtocol::slotSetAway()
{
    kdDebug() << "Jabber plugin: Setting away mode." << endl;
	if (reasonDialog != 0L) {
		delete reasonDialog;
	}
	reasonDialog = new dlgJabberStatus(this, STATUS_AWAY, kopeteapp->mainWindow());
}

void JabberProtocol::slotSetXA()
{
    kdDebug() << "Jabber plugin: Setting extended away mode." << endl;
	if (reasonDialog != 0L) {
		delete reasonDialog;
	}
	reasonDialog = new dlgJabberStatus(this, STATUS_XA, kopeteapp->mainWindow());
}

void JabberProtocol::slotSetDND()
{
    kdDebug() << "Jabber plugin: Setting do not disturb mode." << endl;
	if (reasonDialog != 0L) {
		delete reasonDialog;
	}
	reasonDialog = new dlgJabberStatus(this, STATUS_DND, kopeteapp->mainWindow());
}

void JabberProtocol::setPresence(int status, QString reason, int priority) {
	protocol->setPresence(status, reason, priority);
	if (status == STATUS_AWAY || status == STATUS_XA) { statusBarIcon->setPixmap(awayIcon); }
	else if (status == STATUS_DND) { statusBarIcon->setPixmap(naIcon); }
	else if (status == STATUS_ONLINE) { statusBarIcon->setPixmap(onlineIcon); }
	else { /* What You Say!! */
		kdDebug() << "Jabber plugin: setPresence as confused as a virgin in a brothel (status == " << status << ")!!" << endl;
		protocol->setPresence(STATUS_ONLINE, "", priority);
		kdDebug() << "Jabber plugin: Took corrective measures - went online. Someone needs a good LARTing." << endl;
		statusBarIcon->setPixmap(onlineIcon);
	}
}

void JabberProtocol::slotIconRightClicked(const QPoint)
{
    QString handle = mUsername + "@" + mServer;
    popup = new KPopupMenu(statusBarIcon);
    popup->insertTitle(handle);
    actionGoOnline->plug(popup);
    actionGoOffline->plug(popup);
    actionGoAway->plug(popup);
    actionGoXA->plug(popup);
    actionGoDND->plug(popup);
    popup->popup(QCursor::pos());
}

void JabberProtocol::removeUser(QString userID)
{
    kdDebug() << "Jabber plugin: Protocol removing user " << userID << endl;
    protocol->unsubscribed(userID);
	protocol->unsubscribe(userID);
}

void JabberProtocol::renameContact(QString userID, QString name, QString group)
{
    kdDebug() << "Jabber plugin: Protocol renaming user " << userID << " to " << name << endl;
    protocol->setRoster(userID, name, group);
}

void JabberProtocol::moveUser(QString userID, QString group, QString name,
			      JabberContact *contact)
{
	QString localGroup;
	if (group == QString("")) { 
		kdDebug() << "Jabber plugin: Protocol moving user " << userID << " out of a group." << endl;
		localGroup = i18n("Unknown");
	}
	else { 
		kdDebug() << "Jabber plugin: Protocol moving user " << userID << " to " << group << endl;
		localGroup = group;
	}
    protocol->setRoster(userID, name, group);
    kopeteapp->contactList()->moveContact(contact, localGroup);
}

void JabberProtocol::addContact(QString userID)
{
    kdDebug() << "Jabber plugin: Protocol adding user " << userID << endl;
    protocol->subscribe(userID);
}

void JabberProtocol::slotUserWantsAuth(const Jid &jid)
{
	QString userID = QString("%1@%2").arg(jid.user(), 1).arg(jid.host(), 2);
	kdDebug() << "Jabber plugin: " << userID << " wants auth!" << endl;
	if (authContact->
		questionYesNo(kopeteapp->mainWindow(), i18n("The Jabber user ") + userID +
	    i18n(" wants to add you to your contact list. Do you want to \
			   authorize them?"),
		i18n("Authorize Jabber user?")) == 3) {
			protocol->subscribed(userID);
    }
}

void JabberProtocol::slotContactUpdated(JabRosterEntry *contact) {
    emit contactUpdated(contact->jid.latin1(), contact->nick.latin1(), contact->localStatus(), contact->unavailableStatusString.latin1());
}

void JabberProtocol::slotNewContact(JabRosterEntry *contact) {
	/* !!KLUDGE!!.
	 * o/~ don't mean to brag, don't mean to boast/but I'm intercontinental when I eat French toast */
	if (contactList[contact->jid]) { return; }
	QString group = *(contact->groups.begin());
	JabberContact *jabContact = new JabberContact(contact->jid, contact->nick, group ? group : QString(""), this);
	kdDebug() << "Jabber plugin: Adding contact " << contact->jid << " ..." << endl;
	contactList[contact->jid] = jabContact;
	kdDebug() << "Jabber plugin: b00y4!" << endl;
	kdDebug() << "Jabber plugin: So, that's " << (contactList[contact->jid])->userID() << ", eh?" << endl;
	kopeteapp->contactList()->addContact(jabContact, group ? group : i18n("Unknown"));
	slotContactUpdated(contact); /* More kludges! Ugh. */
}

void JabberProtocol::slotSettingsChanged()
{
    mUsername = KGlobal::config()->readEntry("UserID", "");
    mPassword = KGlobal::config()->readEntry("Password", "");
    mResource = KGlobal::config()->readEntry("Resource", "Kopete");
    mServer = KGlobal::config()->readEntry("Server", "jabber.org");
    mPort = KGlobal::config()->readNumEntry("Port", 5222);
	protocol->setHost(mServer, mPort);
	protocol->setAccount(mUsername, mPassword, mResource);
}

void JabberProtocol::slotSendMsg(JabMessage message)
{
    protocol->sendMessage(message);
}

void JabberProtocol::slotNewMessage(const JabMessage &message)
{
	QString jid = QString("%1@%2").arg(message.from.user(), 1).arg(message.from.host(), 2);
	if (message.from.user() == QString("")) {
		kdDebug() << "Jabber protocol: New server message for us!" << endl;
		KMessageBox::information(kopeteapp->mainWindow(), message.body, i18n("Jabber: Server message"));
	}
	else {
		kdDebug() << "Jabber protocol: New message from '" << jid << "'" << endl;
		JabberContact *ourContact = contactList[jid];
		if (!ourContact) {
			kdDebug() << "Jabber protocol: Crap! No contact, gack." << endl;
			return;
		}
		ourContact->slotNewMessage(message);
	}
}

void JabberProtocol::registerUser() {
	mPrefs->save();
	doRegister = true;
	Connect();
	doRegister = false;
}

#include "jabberprotocol.moc"

// vim: set ts=4 sts=4 sw=4 noet:
