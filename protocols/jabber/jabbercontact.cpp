/***************************************************************************
                          jabbercontact.cpp  -  description
                             -------------------
    begin                : Fri Apr 12 2002
    copyright            : (C) 2002 by Daniel Stone
    email                : dstone@kde.org
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
#include <kconfig.h>

#include "kopetestdaction.h"
#include "jabbercontact.h"
#include "kopetemessage.h"
#include "kopetemessagemanager.h"
#include "jabberprotocol.h"
#include "jabcommon.h"

JabberContact::JabberContact(QString userID, QString name, QString group, JabberProtocol *protocol) : KopeteContact(protocol) {
    mProtocol = protocol;
    if (name.isNull()) { name = userID; hasLocalName = false; }
    setName(name);
    mGroup = group;
    mUserID = userID;
	if (mGroup == QString("")) { hasLocalGroup = false; }
	else { hasLocalGroup = true; }
	hasResource = false;
    connect(mProtocol, SIGNAL(contactUpdated(QString, QString, int, QString)), this, SLOT(slotUpdateContact(QString, QString, int, QString)));
    connect(mProtocol, SIGNAL(nukeContacts(bool)), this, SLOT(slotDeleteMySelf(bool)));
	connect(mProtocol, SIGNAL(resourceAvailable(const Jid &, const JabResource &)), this, SLOT(slotResourceAvailable(const Jid &, const JabResource &)));
	connect(mProtocol, SIGNAL(resourceUnavailable(const Jid &)), this, SLOT(slotResourceUnavailable(const Jid &)));

	historyDialog = 0L;

    initContact(userID, name);
}

void JabberContact::initContact(QString, QString)
{
    initActions();
    slotUpdateContact(mUserID, "", STATUS_OFFLINE, "");
	theContacts.append(this);
	mMsgManagerKCW = 0L;
	mMsgManagerKEW = 0L;
}

KopeteMessageManager* JabberContact::msgManagerKEW() {
	if (mMsgManagerKEW != 0L)
		return mMsgManagerKEW;
	else {
		mMsgManagerKEW = kopeteapp->sessionFactory()->create(mProtocol->myself(), theContacts, mProtocol, "jabber_logs/" + mUserID +".log", KopeteMessageManager::Email);
		connect(mMsgManagerKEW, SIGNAL(messageSent(const KopeteMessage)), this, SLOT(slotSendMsgKEW(const KopeteMessage)));
		return mMsgManagerKEW;
	}
}

KopeteMessageManager* JabberContact::msgManagerKCW() {
	if (mMsgManagerKCW != 0L)
		return mMsgManagerKCW;
	else {
		mMsgManagerKCW = kopeteapp->sessionFactory()->create(mProtocol->myself(), theContacts, mProtocol, "jabber_logs/" + mUserID +".log", KopeteMessageManager::ChatWindow);
		connect(mMsgManagerKCW, SIGNAL(messageSent(const KopeteMessage)), this, SLOT(slotSendMsgKCW(const KopeteMessage)));
		return mMsgManagerKCW;
	}
}

void JabberContact::initActions() {
    actionChat = KopeteStdAction::sendMessage(this, SLOT(slotChatThisUser()), this, "actionChat");
	actionMessage = new KAction(i18n("Send Email Message"), "mail_generic", 0, this, SLOT(slotEmailUser()), this, "actionMessage");
    actionRemoveFromGroup = new KAction(i18n("Remove From Group"), "edittrash", 0, this, SLOT(slotRemoveFromGroup()), this, "actionRemove");
    actionRemove = KopeteStdAction::deleteContact(this, SLOT(slotRemoveThisUser()), this, "actionDelete");
    actionContactMove = KopeteStdAction::moveContact(this, SLOT(slotMoveThisUser()), this, "actionMove");
    actionHistory = KopeteStdAction::viewHistory(this, SLOT(slotViewHistory()), this, "actionHistory");
    actionRename = new KAction(i18n("Rename Contact"), "editrename", 0, this, SLOT(slotRenameContact()), this, "actionRename");
	actionSelectResource = new KSelectAction(i18n("Select Resource"), "selectresource", 0, this, SLOT(slotSelectResource()), this, "actionSelectResource");
}

void JabberContact::showContextMenu(QPoint, QString)
{
	popup = new KPopupMenu();
	popup->insertTitle(userID() + " (" + mResource + ")");
	
	KGlobal::config()->setGroup("Jabber");
	if (KGlobal::config()->readBoolEntry("EmailDefault", false))
	{
		actionMessage->plug(popup);
		actionChat->plug(popup);
	}
	else
	{
		actionChat->plug(popup);
		actionMessage->plug(popup);
	}
	
	if (mStatus != STATUS_OFFLINE)
	{
		QStringList items;
		int activeItem = 0;
		items.append("Automatic (best resource)");
		JabberResource *tmpBestResource = bestResource();
		items.append(tmpBestResource->resource());
		int i = 1;
		for (JabberResource *tmpResource = resources.first(); tmpResource; tmpResource = resources.next(), i++)
		{                                    
			if ( tmpResource != tmpBestResource )
				items.append(tmpResource->resource());
			
			if ( hasResource && (tmpResource->resource() == activeResource->resource()) )
			{
				kdDebug() << "[JabberContact] Woot woot, it's the same resource, activating(ish) item " << i << endl;
				activeItem = i;
			}
		}
		actionSelectResource->setItems(items);
		if (!hasResource || !activeItem) {
			actionSelectResource->setCurrentItem(0);
		}
		else {
			actionSelectResource->setCurrentItem(activeItem);
			kdDebug() << "[JabberContact] Item " << activeItem << " fully activated." << endl;
		}
		actionSelectResource->plug(popup);
	}
	popup->insertSeparator();
    actionHistory->plug(popup);
    popup->insertSeparator();
    actionRename->plug(popup);
    actionContactMove->plug(popup);
    actionRemoveFromGroup->plug(popup);
    actionRemove->plug(popup);
    popup->popup(QCursor::pos());
}

void JabberContact::slotUpdateContact(QString handle, QString resource, int newStatus, QString reason)
{
	if (handle != userID())
		return;

	kdDebug() << "[JabberContact] Updating Contact " << handle << " to " << newStatus << endl;

	if (newStatus != -1)
		mStatus = newStatus;

	if (resource != QString(""))
		mResource = resource;

	if (reason != QString(""))
		mReason = reason;

	emit statusChanged();
	emit statusChanged( this, status() );
}

void JabberContact::slotRenameContact() {
    kdDebug() << "[JabberContact] Renaming contact." << endl;
    dlgRename = new dlgJabberRename;
    dlgRename->lblUserID->setText(userID());
    dlgRename->leNickname->setText(name());
    connect(dlgRename->btnRename, SIGNAL(clicked()), this, SLOT(slotDoRenameContact()));
    dlgRename->show();
}

void JabberContact::slotDoRenameContact() {
	QString name = dlgRename->leNickname->text();
	if (name == QString("")) { hasLocalName = false; name = mUserID; }
	else { hasLocalName = true; }
	setName(name);
    
	delete dlgRename;
	mProtocol->renameContact(userID(), hasLocalName ? name : QString(""), hasLocalGroup ? mGroup : QString(""));
}

void JabberContact::slotDeleteMySelf(bool) {
    delete this;
}


JabberContact::ContactStatus JabberContact::status() const
{
	if (mStatus == STATUS_ONLINE)
	{
		return Online;
    }
    if (mStatus == STATUS_AWAY || mStatus == STATUS_XA || mStatus == STATUS_DND)
	{
		return Away;
    }
	else
	{
		return Offline;
	}
}

QString JabberContact::statusText() const {
    return mStatus + " (" + mReason + ")";
}

QString JabberContact::statusIcon() const {
    if (mStatus == STATUS_ONLINE) {
		return "jabber_online";
    }
    if (mStatus == STATUS_AWAY || mStatus == STATUS_XA) {
		return "jabber_away";
    }
    if (mStatus == STATUS_DND) {
		return "jabber_na";
    }
    return "jabber_offline";
}

void JabberContact::slotRemoveThisUser() {
    mProtocol->removeUser(userID());
    delete this;
}

void JabberContact::slotMoveThisUser() {
	if (!(mGroup = actionContactMove->currentText())) {
		hasLocalGroup = false;
	}
	else {
		hasLocalGroup = true;
	}
	mProtocol->moveUser(userID(), mGroup, name(), this);
}

int JabberContact::importance() const {
    if (mStatus == STATUS_ONLINE) {
		return 20;
    }
    if (mStatus == STATUS_AWAY) {
		return 15;
    }
    if (mStatus == STATUS_XA) {
		return 12;
    }
    if (mStatus == STATUS_DND) {
		return 10;
    }
    return 0;
}

void JabberContact::slotChatThisUser() {
    kdDebug() << "[JabberContact] Opening chat with user " << userID() << endl;
	msgManagerKCW()->readMessages();
}

void JabberContact::slotEmailUser() {
	kdDebug() << "[JabberContact] Opening message with user " << userID() << endl;
	msgManagerKEW()->readMessages();
	msgManagerKEW()->slotSendEnabled(true);
}

void JabberContact::execute() {
	KGlobal::config()->setGroup("Jabber");
	if (KGlobal::config()->readBoolEntry("EmailDefault", false)) {
		slotEmailUser();
	}
	else {
		slotChatThisUser();
	}
}

void JabberContact::slotNewMessage(const JabMessage &message) {
	QString theirUserID = QString("%1@%2").arg(message.from.user(), 1).arg(message.from.host());
	
	kdDebug() << "[JabberContact] Message recieved from " << theirUserID << endl;
    if (theirUserID != userID()) {
		return;
    }

	kdDebug() << "[JabberContact] It's for us! *swoon*" << endl;
	KopeteContactPtrList contactList;
	kdDebug() << "[JabberContact] Past new KCL" << endl;
	contactList.append(mProtocol->myself());
	KopeteMessage jabberMessage(this, contactList, message.body, message.subject, KopeteMessage::Inbound);
	kdDebug() << "[JabberContact] Past new KM" << endl;
	if (message.type == JABMESSAGE_CHAT)
		msgManagerKCW()->appendMessage(jabberMessage);
	else
		msgManagerKEW()->appendMessage(jabberMessage);
		msgManagerKEW()->slotSendEnabled(false);
	kdDebug() << "[JabberContact] end slotNewMessage" << endl;
}

void JabberContact::slotViewHistory() {
    if (historyDialog == 0L) {
		historyDialog = new KopeteHistoryDialog(QString("jabber_logs/%1.log").arg(userID()), name(), true, 50, 0, "JabberHistoryDialog");
		connect(historyDialog, SIGNAL(closing()), this, SLOT(slotCloseHistoryDialog()));
    }
}

void JabberContact::slotCloseHistoryDialog() {
    delete historyDialog;
	historyDialog = 0L;
}

void JabberContact::slotSendMsgKEW(const KopeteMessage message) {
	JabMessage jabMessage;
	JabberContact *to = dynamic_cast<JabberContact *>(message.to().first());
	const JabberContact *from = dynamic_cast<const JabberContact *>(message.from());
	kdDebug() << "[JabberContact] slotSendMsg called: to " << to->userID() << ", from " << from->userID() << ", body: " << message.body() << "." << endl;
	if (hasResource) {
		jabMessage.to = QString("%1/%2").arg(to->userID(), 1).arg(activeResource->resource(), 2);
	}
	else {
		jabMessage.to = to->userID();
	}
	jabMessage.from = from->userID();
	jabMessage.body = message.body();
	jabMessage.subject = message.subject();
	mProtocol->slotSendMsg(jabMessage);
	msgManagerKEW()->appendMessage(message);
}

void JabberContact::slotSendMsgKCW(const KopeteMessage message) {
	JabMessage jabMessage;
	JabberContact *to = dynamic_cast<JabberContact *>(message.to().first());
	const JabberContact *from = dynamic_cast<const JabberContact *>(message.from());
	kdDebug() << "[JabberContact] slotSendMsg called: to " << to->userID() << ", from " << from->userID() << ", body: " << message.body() << "." << endl;
	if (hasResource) {
		jabMessage.to = QString("%1/%2").arg(to->userID(), 1).arg(activeResource->resource(), 2);
	}
	else {
		jabMessage.to = to->userID();
	}
	jabMessage.from = from->userID();
	jabMessage.body = message.body();
	jabMessage.type = JABMESSAGE_CHAT;
	jabMessage.subject = "";
	mProtocol->slotSendMsg(jabMessage);
	msgManagerKCW()->appendMessage(message);
}

void JabberContact::slotResourceAvailable(const Jid &jid, const JabResource &resource) {
	QString theirJID = QString("%1@%2").arg(jid.user(), 1).arg(jid.host(), 2);
	kdDebug() << "[JabberContact] New resource - they want " << theirJID << ", we're " << userID() << endl;
	if (theirJID != userID()) { return; }
	kdDebug() << "[JabberContact] Adding new resource '" << resource.name << "' for " << userID() << endl;
	for (JabberResource *tmpResource = resources.first(); tmpResource; tmpResource = resources.next()) {
//		msgManager()->removeResource(this, tmpResource->resource());
		if (tmpResource->resource() == jid.resource()) {
			/* Yes, it's a hack. AIUI, Psi will emit resourceAvailable() whenever a certain resource
			 * changes status, which means we can't really avoid this, unless we want multiple instances
			 * of the same resource in this list, every single time. Ugh, no thanks. I want my programs
			 * to *work*, thankyouverymuch.
			 *
			 * Well, OK, maybe it isn't a hack. */
			kdDebug() << "[JabberContact] Resource " << tmpResource->resource() << " already added ... b0rk b0rk b0rk!" << endl;
			resources.remove();
		}
	}
	JabberResource *newResource = new JabberResource(resource.name, resource.priority, resource.timeStamp, resource.status, resource.statusString);
	resources.append(newResource);
	JabberResource *tmpBestResource = bestResource();
	kdDebug() << "[JabberContact] Best resource is now " << tmpBestResource->resource() << "." << endl;
	slotUpdateContact(theirJID, tmpBestResource->resource(), tmpBestResource->status(), tmpBestResource->reason());
//	msgManager()->addResource(this, tmpBestResource->resource());
	for (JabberResource *tmpResource = resources.first(); tmpResource; tmpResource = resources.next()) {
		if (tmpResource != tmpBestResource) {
//			msgManager()->addResource(this, tmpResource->resource());
		}
	}
	if (hasResource == false) {
		activeResource = tmpBestResource;
	}
}

void JabberContact::slotResourceUnavailable(const Jid &jid) {
	QString theirJID = QString("%1@%2").arg(jid.user(), 1).arg(jid.host(), 2);
	kdDebug() << "[JabberContact] Removing resource - they want " << theirJID << ", we're " << userID() << endl;
	if (theirJID != userID()) { return; }
	kdDebug() << "[JabberContact] Removing resource '" << jid.resource() << "' for " << userID() << endl;
	JabberResource *resource;
	for (resource = resources.first(); resource; resource = resources.next()) {
		if (resource->resource() == jid.resource()) {
			kdDebug() << "[JabberContact] Got a match in " << resource->resource() << ", removing." << endl;
			if (resources.remove()) {
				kdDebug() << "[JabberContact] Successfully removed, there are now " << resources.count() << " resources!" << endl;
			}
			else {
				kdDebug() << "[JabberContact] Ack! Couldn't remove the resource. Bugger!" << endl;
			}
			break;
		}
	}
	JabberResource *newResource = bestResource();
	if (!newResource) {
		kdDebug() << "[JabberContact] No best resource! User is offline." << endl;
		slotUpdateContact(theirJID, "", STATUS_OFFLINE, "");
	}
	else {
		kdDebug() << "[JabberContact] Best resource is now " << newResource->resource() << "." << endl;
		slotUpdateContact(theirJID, newResource->resource(), newResource->status(), newResource->reason());
	}
	if (hasResource == false || activeResource == resource) {
		/* No good sending to a deleted resource. */
		activeResource = bestResource();
		hasResource = false;
	}
}

void JabberContact::slotSelectResource() {
	if (actionSelectResource->currentItem() == 0) {
		hasResource = false;
		activeResource = bestResource();
		kdDebug() << "[JabberContact] Removing active resource, trusting bestResource()." << endl;
	}
	else {
		hasResource = true;
		JabberResource *resource;
		QString selectedResource = actionSelectResource->currentText();
		kdDebug() << "[JabberContact] Moving to resource " << selectedResource << endl;
		for (resource = resources.first(); resource; resource = resources.next()) {
			if (resource->resource() == selectedResource) {
				activeResource = resource;
				kdDebug() << "[JabberContact] New active resource is " << resource->resource() << endl;
				break;
			}
		}
	}
}

JabberResource *JabberContact::bestResource() {
	JabberResource *resource, *tmpResource;
	for (resource = tmpResource = resources.first(); tmpResource; tmpResource = resources.next()) {
		kdDebug() << "[JabberContact] Processing resource " << tmpResource->resource() << endl;
		if (tmpResource->priority() > resource->priority()) {
			kdDebug() << "[JabberContact] Got better resource " << tmpResource->resource() << " through better priority." << endl;
			resource = tmpResource;
		}
		else if (tmpResource->priority() == resource->priority()) {
			if (tmpResource->timestamp() > resource->timestamp()) {
				kdDebug() << "[JabberContact] Got better resource " << tmpResource->resource() << " through newer timestamp." << endl;
				resource = tmpResource;
			}
			else {
				kdDebug() << "[JabberContact] Discarding resource " << tmpResource->resource() << " with older timestamp." << endl;
			}
		}
		else {
			kdDebug() << "[JabberContact] Discarding resource " << tmpResource->resource() << " with worse priority." << endl;
		}
	}
	return resource;
}

void JabberContact::slotRemoveFromGroup() {
	mProtocol->moveUser(userID(), mGroup = QString(""), name(), this);
	hasLocalGroup = false;
}

JabberResource::JabberResource() { 
	kdDebug() << "Jabber resource: New Jabber resource (no params)." << endl;
}

JabberResource::JabberResource(const QString &resource, const int &priority, const QDateTime &timestamp, const int &status, const QString &reason) {
	kdDebug() << QString("Jabber resource: New Jabber resource (resource %1, priority %2, timestamp %3).").arg(resource, 1).arg(priority, 2).arg(timestamp.toString("yyyyMMddhhmmss"), 3) << endl;
	mResource = resource;
	mPriority = priority;
	mTimestamp = timestamp;
	mStatus = status;
	mReason = reason;
}

JabberResource::~JabberResource() {
}

#include "jabbercontact.moc"

// vim: noet ts=4 sts=4 sw=4:
