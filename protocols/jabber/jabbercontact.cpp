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

#include "kopetestdaction.h"
#include "jabbercontact.h"
#include "jabbermessage.h"
#include "kopetemessagemanager.h"
#include "jabberprotocol.h"
#include "jabcommon.h"

JabberContact::JabberContact(QString userid, QString name, QString group, JabberProtocol *protocol) : KopeteContact(protocol) {
    mProtocol = protocol;
    mName = name;
    mGroup = group;
    mUserID = userid;
    if (mGroup == QString("")) { hasLocalGroup = false; }
	else { hasLocalGroup = true; }
    connect(protocol,
	    SIGNAL(contactUpdated(QString, QString, int, QString)),
	    this,
	    SLOT(slotUpdateContact(QString, QString, int, QString)));
    connect(protocol, SIGNAL(nukeContacts(bool)), this,
	    SLOT(slotDeleteMySelf(bool)));
	connect(protocol, SIGNAL(resourceAvailable(const Jid &, const JabResource &)), this,
		SLOT(slotResourceAvailable(const Jid &, const JabResource &)));
	connect(protocol, SIGNAL(resourceUnavailable(const Jid &)), this,
		SLOT(slotResourceUnavailable(const Jid &)));
    connect(protocol, SIGNAL(newMessage(const JabMessage &)), this,
	    SLOT(slotNewMessage(const JabMessage &)));

	historyDialog = 0L;

    initContact(userid, name);
}

void JabberContact::initContact(QString, QString name) {
    setName(name);
    initActions();
    slotUpdateContact(mUserID, "", STATUS_OFFLINE, "");
	theContacts.append(this);
	msgManager = kopeteapp->sessionFactory()->create(mProtocol->myself(), theContacts);
	connect(msgManager, SIGNAL(messageSent(const KopeteMessage &)), this, SLOT(slotSendMsg(const KopeteMessage &)));
}

void JabberContact::initActions() {
    actionChat =
	KopeteStdAction::sendMessage(this, SLOT(slotChatThisUser()), this,
				     "actionChat");
    actionRemoveFromGroup =
	new KAction(i18n("Remove From Group"), "edittrash", 0, this,
		    SLOT(slotRemoveFromGroup()), this, "actionRemove");
    actionRemove =
	KopeteStdAction::deleteContact(this, SLOT(slotRemoveThisUser()),
				       this, "actionDelete");
    actionContactMove =
	KopeteStdAction::moveContact(this, SLOT(slotMoveThisUser()), this,
				     "actionMove");
    actionHistory =
	KopeteStdAction::viewHistory(this, SLOT(slotViewHistory()), this,
				     "actionHistory");
    actionRename =
	new KAction(i18n("Rename Contact"), "editrename", 0, this,
		    SLOT(slotRenameContact()), this, "actionRename");
}

void JabberContact::showContextMenu(QPoint, QString /*group */) {
    popup = new KPopupMenu();
    popup->insertTitle(mUserID + " (" + mResource + ")");
    actionChat->plug(popup);
    popup->insertSeparator();
    actionHistory->plug(popup);
    popup->insertSeparator();
    actionRename->plug(popup);
    actionContactMove->plug(popup);
    actionRemoveFromGroup->plug(popup);
    actionRemove->plug(popup);
    popup->popup(QCursor::pos());
}

void JabberContact::slotUpdateContact(QString handle, QString resource, int status, QString reason) {
    if (handle != mUserID)
		return;
    kdDebug() << "Jabber contact: Contact - updating " << handle << " to " << status << "." << endl;
	if (status != -1) {
		mStatus = status;
	}
    if (resource != QString("")) {
		mResource = resource;
    }
    if (reason != QString("")) {
		mReason = reason;
    }
    emit statusChanged();
}

void JabberContact::slotRenameContact() {
    kdDebug() << "Jabber contact: Renaming contact." << endl;
    dlgRename = new dlgJabberRename;
    dlgRename->lblUserID->setText(mUserID);
    dlgRename->leNickname->setText(mName);
    connect(dlgRename->btnRename, SIGNAL(clicked()), this,
	    SLOT(slotDoRenameContact()));
    dlgRename->show();
}

void JabberContact::slotDoRenameContact() {
    mName = dlgRename->leNickname->text();
    setName(mName);
    delete dlgRename;
    if (hasLocalGroup) { 
		mProtocol->renameContact(mUserID, mName, mGroup);
	}
	else {
		mProtocol->renameContact(mUserID, mName, "");
	}
}

void JabberContact::slotDeleteMySelf(bool) {
    delete this;
}


JabberContact::ContactStatus JabberContact::status() const {
    if (mStatus == STATUS_ONLINE) {
		return Online;
    }
    if (mStatus == STATUS_AWAY || mStatus == STATUS_XA || mStatus == STATUS_DND) {
		return Away;
    } else {
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
    mProtocol->removeUser(mUserID);
    delete this;
}

void JabberContact::slotMoveThisUser() {
    mProtocol->moveUser(mUserID, actionContactMove->currentText(), mName, this);
    mGroup = actionContactMove->currentText();
	hasLocalGroup = true;
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
    kdDebug() << "Jabber contact: Opening chat with user " << mUserID <<
	endl;
	kdDebug() << "Jabber contact: It's for us! *swoon*" << endl;
	msgManager->readMessages();
}

void JabberContact::execute() {
    slotChatThisUser();
}

void JabberContact::slotNewMessage(const JabMessage &message) {
	QString userID = QString("%1@%2").arg(message.from.user(), 1).arg(message.from.host());
	JabberMessage jabberMessage(this, userID, message.subject, message.body);
	
	kdDebug() << "Jabber contact: Message recieved from " << userID << endl;
    if (userID != mUserID) {
		return;
    }
	
	kdDebug() << "Jabber contact: It's for us! *swoon*" << endl;
	if (msgManager == 0L) {
	}

	msgManager->appendMessage(jabberMessage);
	msgManager->readMessages();
	kdDebug() << "Jabber contact: end slotNewMessage" << endl;
}

void JabberContact::slotViewHistory() {
    if (historyDialog == 0L) {
	historyDialog =
	    new KopeteHistoryDialog(QString("kopete/jabber_logs/%1.log").
				    arg(mUserID), name(), true, 50, 0,
				    "JabberHistoryDialog");
	connect(historyDialog, SIGNAL(closing()), this,
		SLOT(slotCloseHistoryDialog()));
    }
}

void JabberContact::slotCloseHistoryDialog() {
    delete historyDialog;
	historyDialog = 0L;
}

void JabberContact::slotSendMsg(const KopeteMessage &message) {
	kdDebug() << "Jabber contact: slotSendMsg called: to " << message.to() << ", from " << message.from() << ", body: " << message.body() << "." << endl;
	JabMessage jabMessage;
	jabMessage.to = message.to();
	jabMessage.from = message.from();
	jabMessage.body = message.body();
	jabMessage.type = JABMESSAGE_CHAT;
	jabMessage.subject = "";
	mProtocol->slotSendMsg(jabMessage);
	msgManager->appendMessage(message);
	msgManager->readMessages();
}

void JabberContact::slotResourceAvailable(const Jid &jid, const JabResource &resource) {
	QString theirJID = QString("%1@%2").arg(jid.user(), 1).arg(jid.host(), 2);
	kdDebug() << "Jabber contact: New resource - they want " << theirJID << ", we're " << mUserID << endl;
	if (theirJID != mUserID) { return; }
	kdDebug() << "Jabber contact: Adding new resource '" << resource.name << "' for " << mUserID << endl;
	for (JabberResource *tmpResource = resources.first(); tmpResource; tmpResource = resources.next()) {
		if (tmpResource->resource() == jid.resource()) {
			/* Yes, it's a hack. AIUI, Psi will emit resourceAvailable() whenever a certain resource
			 * changes status, which means we can't really avoid this, unless we want multiple instances
			 * of the same resource in this list, every single time. Ugh, no thanks. I want my programs
			 * to *work*, thankyouverymuch.
			 *
			 * Well, OK, maybe it isn't a hack. */
			kdDebug() << "Jabber contact: Resource " << tmpResource->resource() << " already added ... b0rk b0rk b0rk!" << endl;
			resources.remove();
		}
	}
	resources.append(new JabberResource(resource.name, resource.priority, resource.timeStamp, resource.status, resource.statusString));	
	JabberResource *newResource = bestResource();
	kdDebug() << "Jabber contact: Best resource is now " << newResource->resource() << "." << endl;
	slotUpdateContact(theirJID, newResource->resource(), newResource->status(), newResource->reason());
}

void JabberContact::slotResourceUnavailable(const Jid &jid) {
	QString theirJID = QString("%1@%2").arg(jid.user(), 1).arg(jid.host(), 2);
	kdDebug() << "Jabber contact: Removing resource - they want " << theirJID << ", we're " << mUserID << endl;
	if (theirJID != mUserID) { return; }
	kdDebug() << "Jabber contact: Removing resource '" << jid.resource() << "' for " << mUserID << endl;
	JabberResource *resource;
	for (resource = resources.first(); resource; resource = resources.next()) {
		if (resource->resource() == jid.resource()) {
			kdDebug() << "Jabber contact: Got a match in " << resource->resource() << ", removing." << endl;
			if (resources.remove()) {
				kdDebug() << "Jabber contact: Successfully removed, there are now " << resources.count() << "resources!" << endl;
			}
			else {
				kdDebug() << "Jabber contact: Ack! Couldn't remove the resource. Bugger!" << endl;
			}
			break;
		}
	}
	JabberResource *newResource = bestResource();
	if (!newResource) {
		kdDebug() << "Jabber contact: No best resource! User is offline." << endl;
		slotUpdateContact("", "", 0, "");
	}
	else {
		kdDebug() << "Jabber contact: Best resource is now " << newResource->resource() << "." << endl;
		slotUpdateContact(theirJID, newResource->resource(), newResource->status(), newResource->reason());
	}
}

JabberResource *JabberContact::bestResource() {
	JabberResource *resource, *tmpResource;
	for (resource = tmpResource = resources.first(); tmpResource; tmpResource = resources.next()) {
		kdDebug() << "Jabber contact: Processing resource " << tmpResource->resource() << endl;
		if (tmpResource->priority() > resource->priority()) {
			kdDebug() << "Jabber contact: Got better resource " << tmpResource->resource() << " through better priority." << endl;
			resource = tmpResource;
		}
		else if (tmpResource->priority() == resource->priority()) {
			if (tmpResource->timestamp() > resource->timestamp()) {
				kdDebug() << "Jabber contact: Got better resource " << tmpResource->resource() << " through newer timestamp." << endl;
				resource = tmpResource;
			}
			else {
				kdDebug() << "Jabber contact: Discarding resource " << tmpResource->resource() << " with older timestamp." << endl;
			}
		}
		else {
			kdDebug() << "Jabber contact: Discarding resource " << tmpResource->resource() << " with worse priority." << endl;
		}
	}
	return resource;
}

void JabberContact::slotRemoveFromGroup() {
	mProtocol->moveUser(mUserID, mGroup = "", mName, this);
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
