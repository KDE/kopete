/***************************************************************************
                          jabberprotocol.cpp  -  Jabber Plugin
                             -------------------
    begin                : Fri Apr 12 2002
    copyright            : (C) 2002 by Daniel Stone
    email                : daniel@raging.dropbear.id.au
 ***************************************************************************

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qcursor.h>

#include <kdebug.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <ksimpleconfig.h>
#include <kstandarddirs.h>

#include "kopete.h"
#include "jabberprotocol.h"
#include "statusbaricon.h"

JabberProtocol::JabberProtocol(): QObject(0, "JabberProtocol"), KopeteProtocol() {
	kdDebug() << "\nJabber plugin: Loading ..." << endl;

	initIcons();
	initActions();

	authContact = new KMessageBox;
	protocol = new KJabber;
	connect(protocol, SIGNAL(contactUpdated(QString, QString, QString, QString)), this, SLOT(slotContactUpdated(QString, QString, QString, QString)));
	connect(protocol, SIGNAL(userWantsAuth(QString)), this, SLOT(slotUserWantsAuth(QString)));
	connect(protocol, SIGNAL(newContact(QString, QString, QString)), this, SLOT(slotNewContact(QString, QString, QString)));

	statusBarIcon = new StatusBarIcon();
	QObject::connect(statusBarIcon, SIGNAL(rightClicked(const QPoint)), this, SLOT(slotIconRightClicked(const QPoint)));
	statusBarIcon->setPixmap(offlineIcon);

	mPrefs = new JabberPreferences("jabber_protocol_32", this);
	connect(mPrefs, SIGNAL(saved(void)), this, SLOT(settingsChanged(void)));

	KGlobal::config()->setGroup("Jabber");
	if ((KGlobal::config()->readEntry("UserID", "") == "" ) || (KGlobal::config()->readEntry("Password", "") == "" )) 	{
		QString emptyText = i18n( "<qt>If you have a Jabber account, please configure it in the Kopete Settings. If you don't, please create one with another app; registering is not yet supported in Kopete.</qt>" );
		QString emptyCaption = i18n( "No Jabber Configuration found!" );

		KMessageBox::error(kopeteapp->mainWindow(), emptyText, emptyCaption);
	}
	mIsConnected = false;
	slotSettingsChanged();
	/** Autoconnect if is selected in config */
	if (KGlobal::config()->readBoolEntry("AutoConnect", "0")) {
		Connect();
	}
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
	if( kopeteapp->statusBar() )
	{
		kopeteapp->statusBar()->removeWidget(statusBarIcon);
		delete statusBarIcon;
	}

	emit protocolUnloading();
	return true;
}

void JabberProtocol::Connect() {
	if (!isConnected()) {
		kdDebug() << "Jabber plugin: Attempting to connect to Jabber server " << m_Server << ":" << m_Port << endl;
		kdDebug() << "Jabber plugin: Using UserID " << m_Username << " with password " << m_Password << endl;
 		protocol->Connect(m_Server, m_Port, m_Username, m_Password, m_Resource);
		mIsConnected = true;
		statusBarIcon->setPixmap(onlineIcon);
	}
	else if (isAway()) { /* They're really away, and they want to un-away. */
		slotGoOnline();
	}
	else { /* Nope, just your regular crack junky. */
		kdDebug() << "Jabber plugin: Ignoring connect request (already connected)." << endl;
	}
}

void JabberProtocol::Disconnect() {
	if (isConnected()) {
		protocol->Disconnect();
		kdDebug() << "Jabber plugin: Disconnected." << endl;
		mIsConnected = false;
		statusBarIcon->setPixmap(offlineIcon);
		emit nukeContacts(false);
	}
	else { /* Again, what's with the crack? Sheez. */
		kdDebug() << "Jabber plugin: Ignoring disconnect request (not connected)." << endl;
	}
}

void JabberProtocol::slotConnect() {
	Connect();
}

void JabberProtocol::slotDisconnect() {
	Disconnect();
}

bool JabberProtocol::isConnected() const {
	return mIsConnected;
}

void JabberProtocol::setAway(void) {
	slotSetAway();
}

void JabberProtocol::setAvailable(void) {
	slotGoOnline();
}

bool JabberProtocol::isAway(void) const {
  if (isConnected()) {
    return (protocol->getStatus() == "away" || protocol->getStatus() == "xa" || protocol->getStatus() == "dnd");
  }
  else {
    return false;
  }
}

QString JabberProtocol::protocolIcon() const {
	return "jabber_protocol_32";
}

AddContactPage *JabberProtocol::createAddContactWidget(QWidget *parent) {
  return new JabberAddContactPage(this, parent);
}

void JabberProtocol::initIcons() {
	KIconLoader *loader = KGlobal::iconLoader();
	KStandardDirs dir;

	onlineIcon = QPixmap(loader->loadIcon("jabber_online", KIcon::User));
	offlineIcon = QPixmap(loader->loadIcon("jabber_offline", KIcon::User));
	awayIcon = QPixmap(loader->loadIcon("jabber_away", KIcon::User));
	naIcon = QPixmap(loader->loadIcon("jabber_na", KIcon::User));
	connectingIcon = QMovie(dir.findResource("data","kopete/pics/jabber_connecting.mng"));
}

void JabberProtocol::initActions() {
	actionGoOnline = new KAction (i18n("Online"), "jabber_online", 0, this, SLOT(slotConnect()), this, "actionJabberConnect" );
	actionGoAway = new KAction (i18n("Away"), "jabber_away", 0, this, SLOT(slotSetAway()), this, "actionJabberConnect");
	actionGoXA = new KAction(i18n("Extended Away"), "jabber_away", 0, this, SLOT(slotSetXA()), this, "actionJabberConnect");
	actionGoDND = new KAction(i18n("Do Not Disturb"), "jabber_na", 0, this, SLOT(slotSetDND()), this, "actionJabberConnect");
	actionGoOffline = new KAction (i18n("Offline"), "jabber_offline", 0, this, SLOT(slotDisconnect()), this, "actionJabberConnect" );
	actionStatusMenu = new KActionMenu("Jabber",this);
	actionStatusMenu->insert(actionGoOnline);
	actionStatusMenu->insert(actionGoAway);
	actionStatusMenu->insert(actionGoXA);
	actionStatusMenu->insert(actionGoDND);
	actionStatusMenu->insert(actionGoOffline);
	actionStatusMenu->plug(kopeteapp->systemTray()->contextMenu(), 1);
}

void JabberProtocol::slotConnecting() { /* Aw, look at the cute widdle MNG. */
	statusBarIcon->setMovie(connectingIcon);
}

void JabberProtocol::slotConnected() {
	mIsConnected = true;
	kdDebug() << "Jabber plugin: Connected to Jabber server." << endl;
	slotGoOnline();
}

void JabberProtocol::slotDisconnected() {
	mIsConnected = false;
}

void JabberProtocol::slotGoOnline() {
	kdDebug() << "Jabber plugin: Going online!" << endl;
	if (!isConnected()) {
		Connect();
	}
	else {
		protocol->setPresence("online", "");
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
	protocol->setPresence("away", "");
	statusBarIcon->setPixmap(awayIcon);
}

void JabberProtocol::slotSetXA()
{
	kdDebug() << "Jabber plugin: Setting extended away mode." << endl;
	protocol->setPresence("xa", "");
	statusBarIcon->setPixmap(awayIcon);
}

void JabberProtocol::slotSetDND()
{
	kdDebug() << "Jabber plugin: Setting do not disturb mode." << endl;
	protocol->setPresence("dnd", "");
	statusBarIcon->setPixmap(naIcon);
}


void JabberProtocol::slotIconRightClicked (const QPoint) {
	QString handle = m_Username + "@" + m_Server;

	popup = new KPopupMenu(statusBarIcon);
	popup->insertTitle(handle);
	actionGoOnline->plug (popup);
	actionGoOffline->plug (popup);
	actionGoAway->plug (popup);
	actionGoXA->plug (popup);
	actionGoDND->plug (popup);
	popup->popup(QCursor::pos());
}

void JabberProtocol::removeUser (QString userID) {
	kdDebug() << "Jabber plugin: Protocol removing user " << userID << endl;
	protocol->removeUser(userID);
}

void JabberProtocol::renameContact(QString userID, QString name) {
	kdDebug() << "Jabber plugin: Protocol renaming user " << userID << " to " << name << endl;
	protocol->renameUser(userID, name);
}

void JabberProtocol::moveUser(QString userID, QString group, QString name, JabberContact *contact) {
	kdDebug() << "Jabber plugin: Protocol moving user " << userID << " to " << group << endl;
	protocol->moveUser(userID, name, group);
	kopeteapp->contactList()->moveContact(contact, group);
}

void JabberProtocol::addContact(QString userID) {
	kdDebug() << "Jabber plugin: Protocol adding user " << userID << endl;
	protocol->addUser(userID);
}

void JabberProtocol::slotUserWantsAuth(QString userID) {
	if (authContact->questionYesNo(kopeteapp->mainWindow(), i18n("The Jabber user ") + userID + i18n(" wants to add you to your contact list. Do you want to authorize them?"), i18n("Authorize Jabber user?")) == 3) {
		protocol->authUser(userID);
	}
}

void JabberProtocol::slotContactUpdated(QString userID, QString name, QString status, QString reason) {
	emit contactUpdated(userID, name, status, reason);
}

void JabberProtocol::slotNewContact(QString userID, QString name, QString group) {
	if (group == QString("")) {
		group = i18n("Unknown");
	}
	kopeteapp->contactList()->addContact(new JabberContact(userID, name, group, this), group);
}

void JabberProtocol::slotSettingsChanged() {
	m_Username = KGlobal::config()->readEntry("UserID", "");
	m_Password = KGlobal::config()->readEntry("Password", "");
	m_Resource = KGlobal::config()->readEntry("Resource", "Kopete");
	m_Server = KGlobal::config()->readEntry("Server", "jabber.org");
	m_Port = KGlobal::config()->readNumEntry("Port", 5222);
}

#include "jabberprotocol.moc"

// vim: set ts=4 sts=4 sw=4 noet:
