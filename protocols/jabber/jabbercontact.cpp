/***************************************************************************
                          jabbercontact.cpp  -  description
                             -------------------
    begin                : Fri Apr 12 2002
    copyright            : (C) 2002 by Daniel Stone
    email                : daniel@raging.dropbear.id.au
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <kdebug.h>
#include <klocale.h>
#include <qfont.h>

#include "kopetestdaction.h"
#include "jabbercontact.h"
#include "jabbermessagedialog.h"

// Constructor for no-groups
JabberContact::JabberContact(QString userid, QString name, QString group, JabberProtocol *protocol)
	: KopeteContact(protocol) {
	mProtocol = protocol;
	mName = name;
	mGroup =  group;
	mUserID = userid;
	hasLocalGroup = false;

	connect(protocol, SIGNAL(contactUpdated(QString, QString, QString, QString)), this, SLOT(slotUpdateContact(QString, QString, QString, QString)));
	connect(protocol, SIGNAL(nukeContacts(bool)), this, SLOT(slotDeleteMySelf(bool)));
  connect(protocol, SIGNAL(newMessage(QString, QString)), this, SLOT(slotNewMessage(QString, QString)));

	initContact(userid, name);
}

void JabberContact::setResource(QString resource) { mResource = resource; }

void JabberContact::initContact(QString, QString name) {
	setName (name);
	initActions();
	slotUpdateContact(mUserID, "", "Offline", "");
}

void JabberContact::initActions() {
	actionChat				= KopeteStdAction::sendMessage(this, SLOT(slotChatThisUser()), this, "actionChat");
	actionRemoveFromGroup	= new KAction(i18n("Remove From Group"), "edittrash", 0, this, SLOT(slotRemoveFromGroup()), this, "actionRemove");
	actionRemove			= KopeteStdAction::deleteContact(this, SLOT(slotRemoveThisUser()), this, "actionDelete");
	actionContactCopy		= new KListAction (i18n("Copy Contact"), "editcopy", 0, this, SLOT(slotCopyThisUser()), this, "actionCopy");
	actionContactMove		= KopeteStdAction::moveContact(this, SLOT(slotMoveThisUser()), this, "actionMove");
	actionHistory			= KopeteStdAction::viewHistory(this, SLOT(slotViewHistory()), this, "actionHistory");
  actionRename      = new KAction(i18n("Rename Contact"), "editrename", 0, this, SLOT(slotRenameContact()), this, "actionRename");
}

void JabberContact::showContextMenu(QPoint point, QString /*group*/) {
	popup = new KPopupMenu();
	popup->insertTitle(mUserID + " (" + mResource + ")");
	actionChat->plug(popup);
	popup->insertSeparator();
	actionHistory->plug(popup);
	popup->insertSeparator();
	actionRename->plug(popup);
	actionContactMove->plug(popup);
	actionContactCopy->plug(popup);
	actionRemoveFromGroup->plug(popup);
	actionRemove->plug(popup);
	popup->popup(QCursor::pos());
}

void JabberContact::slotUpdateContact (QString handle, QString resource, QString status, QString reason) {
	if (handle != mUserID)
		return;

	kdDebug() << "Jabber plugin: Contact - updating " << handle << " to " << status << "." << endl;

	if (status != QString("")) { mStatus = status; kdDebug() << "Jabber plugin: Updating status." << endl; }
	if (resource != QString("")) { mResource = resource; }
	if (reason != QString("")) { mReason = reason; }
	emit statusChanged();
}

void JabberContact::slotRenameContact() {
	kdDebug() << "Jabber plugin: Renaming contact." << endl;
	dlgRename = new dlgJabberRename;
	dlgRename->lblUserID->setText(mUserID);
	dlgRename->leNickname->setText(mName);
	connect(dlgRename->btnRename, SIGNAL(clicked()), this, SLOT(slotDoRenameContact()));
	dlgRename->show();
}

void JabberContact::slotDoRenameContact() {
	mName = dlgRename->leNickname->text();
	setName(mName);
	delete dlgRename;
	mProtocol->renameContact(mUserID, mName);
}

void JabberContact::slotDeleteMySelf(bool connected) {
	delete this;
}


JabberContact::ContactStatus JabberContact::status() const {
	if (mStatus == QString("online")) { return Online; }
	if (mStatus == QString("away") || mStatus == QString("xa") || mStatus == QString("dnd")) { return Away; }
	else { return Offline; }
}

QString JabberContact::statusText() const {
	return mStatus + " (" + mReason + ")";
}

QString JabberContact::statusIcon() const {
	if (mStatus == QString("online")) { return "jabber_online"; }
	if (mStatus == QString("away") || mStatus == QString("xa")) { return "jabber_away"; }
	if (mStatus == QString("dnd")) { return "jabber_na"; }
	return "jabber_offline";
}

void JabberContact::slotRemoveThisUser() {
	mProtocol->removeUser(mUserID);
	delete this;
}

void JabberContact::slotMoveThisUser() {
	mProtocol->moveUser(mUserID, actionContactMove->currentText(), mName, this);
	mGroup = actionContactMove->currentText();
}

int JabberContact::importance() const {
	if (mStatus == QString("online")) { return 20; }
	if (mStatus == QString("away")) { return 15; }
	if (mStatus == QString("xa")) { return 10; }
	if (mStatus == QString("dnd")) { return 12; }
	return 0;
}

void JabberContact::slotChatThisUser() {
  kdDebug() << "Jabber plugin: Opening chat with user " << mUserID << endl;
  msgDialog = new JabberMessageDialog(this, mProtocol);
  connect(this, SIGNAL(msgRecieved(QString, QString, QString, QString, QFont, QColor)), msgDialog, SLOT(slotMessageRecieved(QString, QString, QString, QString, QFont, QColor)));
  msgDialog->show();
}

void JabberContact::execute() { slotChatThisUser(); }

QString JabberContact::userID() { return mUserID; }
QString JabberContact::nickname() { return mName; }

void JabberContact::slotNewMessage (QString userID, QString message) {
  kdDebug() << "Jabber contact: Message recieved for " << userID << endl;
  if (userID != mUserID) { return; }
  kdDebug() << "Jabber contact: It's for us! *swoon*" << endl;
  emit msgRecieved(userID, mName, mName, message, QFont(), QColor());
  kdDebug() << "Jabber contact: end slotNewMessage" << endl;
}

#include "jabbercontact.moc"

// vim: noet ts=4 sts=4 sw=4:

