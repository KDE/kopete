/***************************************************************************
                          wpcontact.cpp  -  description
                             -------------------
    begin                : Fri Apr 12 2002
    copyright            : (C) 2002 by Gav Wood
    email                : gav@indigoarchive.net
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

// Local Includes
#include "wpcontact.h"
#include "wpprotocol.h"
#include "wpdebug.h"

// Kopete Includes
#include "kopetestdaction.h"
#include "kopetemessage.h"
#include "kopetemessagemanager.h"
#include "kopetecontactlist.h"

// Qt Includes
#include <qdatetime.h>
#include <qregexp.h>

// KDE Includes
#include <kconfig.h>

WPContact::WPContact(const QString &userID, const QString &displayName(), const QString &group, WPProtocol *protocol) : KopeteContact(protocol)
{
	DEBUG(WPDMETHOD, "WPContact::WPContact(" << userID << ", " << displayName() << ", " << group << ", <protocol>)");

	setDisplayName(displayName().isNull() ? userID : name);
	mProtocol = protocol;
	mGroup = group;
	mUserID = userID;

//	connect(mProtocol, SIGNAL(contactUpdated(QString, QString, int, QString)), this, SLOT(slotUpdateContact(QString, QString, int, QString)));
//	connect(mProtocol, SIGNAL(nukeContacts(bool)), this, SLOT(slotDeleteMySelf(bool)));
	connect(&checkStatus, SIGNAL(timeout()), this, SLOT(slotCheckStatus()));
	checkStatus.start(1000, false);

	initActions();
	slotUpdateContact(userID, STATUS_OFFLINE);
	mMsgManagerKEW = 0;
	mMsgManagerKCW = 0;
	historyDialog = 0;
}

void WPContact::slotCheckStatus()
{
	int oldStatus = mStatus;
	mStatus = mProtocol->checkHost(mUserID) ? STATUS_ONLINE : STATUS_OFFLINE;
	if(oldStatus != mStatus)
		emit statusChanged();
}

KopeteMessageManager *WPContact::msgManagerKEW()
{
	DEBUG(WPDMETHOD, "WPContact::msgManager()");

	if(!mMsgManagerKEW)
	{	QPtrList<KopeteContact> singleContact;
		singleContact.append(this);
		mMsgManagerKEW = kopeteapp->sessionFactory()->create(mProtocol->myself(), singleContact, mProtocol, "wp_logs/" + mUserID +".log", KopeteMessageManager::Email);
		connect(mMsgManagerKEW, SIGNAL(messageSent(const KopeteMessage&)), this, SLOT(slotSendMsgKEW(const KopeteMessage&)));
	}
	return mMsgManagerKEW;
}

KopeteMessageManager *WPContact::msgManagerKCW()
{
	DEBUG(WPDMETHOD, "WPContact::msgManager()");

	if(!mMsgManagerKCW)
	{	QPtrList<KopeteContact> singleContact;
		singleContact.append(this);
		mMsgManagerKCW = kopeteapp->sessionFactory()->create(mProtocol->myself(), singleContact, mProtocol, "wp_logs/" + mUserID +".log", KopeteMessageManager::ChatWindow);
		connect(mMsgManagerKCW, SIGNAL(messageSent(const KopeteMessage&)), this, SLOT(slotSendMsgKCW(const KopeteMessage&)));
	}
	return mMsgManagerKCW;
}

void WPContact::initActions()
{
	DEBUG(WPDMETHOD, "WPContact::initActions()");

	actionChat = KopeteStdAction::sendMessage(this, SLOT(slotChatThisUser()), this, "actionChat");
	actionMessage = new KAction(i18n("Send Email Message"), "mail_generic", 0, this, SLOT(slotEmailUser()), this, "actionMessage");
	actionRemoveFromGroup = new KAction(i18n("Remove From Group"), "edittrash", 0, this, SLOT(slotRemoveFromGroup()), this, "actionRemove");
	actionRemove = KopeteStdAction::deleteContact(this, SLOT(slotRemoveThisUser()), this, "actionDelete");
//	actionContactMove = KopeteStdAction::moveContact(this, SLOT(slotMoveThisUser()), this, "actionMove");
	actionHistory = KopeteStdAction::viewHistory(this, SLOT(slotViewHistory()), this, "actionHistory");
//	actionRedisplayName() = new KAction(i18n("Rename Contact"), "editrename", 0, this, SLOT(slotRenameContact()), this, "actionRename");
}

void WPContact::showContextMenu(QPoint position, QString group)
{
	DEBUG(WPDMETHOD, "WPContact::showContextMenu(<position>, " << group << ")");

	popup = new KPopupMenu();	// XXX: Needs deleting at some time?
	popup->insertTitle(mUserID);
	KGlobal::config()->setGroup("WinPopup");
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
	popup->insertSeparator();
	actionHistory->plug(popup);
	popup->insertSeparator();

//	actionRedisplayName()->plug(popup);
//	actionContactMove->plug(popup);
	actionRemoveFromGroup->plug(popup);
	actionRemove->plug(popup);
	popup->popup(position);//QCursor::pos());
}

void WPContact::slotUpdateContact(QString handle, int status)
{
	DEBUG(WPDMETHOD, "WPContact::slotUpdateContact(" << handle << ", " << status << ")");

    if(handle != userID())
		return;
	if(status != -1)
		mStatus = status;
    emit statusChanged();
}

void WPContact::slotRedisplayName()Contact()
{
	DEBUG(WPDMETHOD, "WPContact::slotRedisplayName()Contact()");

/*	kdDebug() << "WP contact: Renaming contact." << endl;
    dlgRedisplayName() = new dlgWPRename;
    dlgRedisplayName()->lblUserID->setText(userID());
    dlgRedisplayName()->leNickname->setText(name());
    connect(dlgRedisplayName()->btnRename, SIGNAL(clicked()), this,
	    SLOT(slotDoRedisplayName()Contact()));
    dlgRedisplayName()->show();
*/
}

void WPContact::slotDoRedisplayName()Contact()
{
	DEBUG(WPDMETHOD, "WPContact::slotDoRedisplayName()Contact()");
/*	QString displayName() = dlgRename->leNickname->text();
	if (displayName() == QString("")) { hasLocalName = false; name = mUserID; }
	else { hasLocalName = true; }
	setDisplayName(displayName());

	delete dlgRedisplayName();
	mProtocol->redisplayName()Contact(userID(), hasLocalName ? name : QString(""), hasLocalGroup ? mGroup : QString(""));
*/
}

void WPContact::slotDeleteMySelf(bool)
{
	DEBUG(WPDMETHOD, "WPContact::slotDeleteMyself()");
	delete this;
}

WPContact::ContactStatus WPContact::status() const
{
	DEBUG(WPDMETHOD, "WPContact::status()");
	if(mStatus == STATUS_ONLINE)
		return Online;
	if(mStatus == STATUS_AWAY)
		return Away;
	return Offline;
}

QString WPContact::statusText() const
{
	DEBUG(WPDMETHOD, "WPContact::statusText()");

	if(mStatus == STATUS_ONLINE)
		return "Online";
	if(mStatus == STATUS_AWAY)
		return "Away";
	return "Offline";
}

QString WPContact::statusIcon() const
{
	DEBUG(WPDMETHOD, "WPContact::statusIcon()");

	if(mStatus == STATUS_ONLINE)
		return "wp_available";
	if(mStatus == STATUS_AWAY)
		return "wp_away";
	return "wp_offline";
}

void WPContact::slotRemoveThisUser()
{
	DEBUG(WPDMETHOD, "WPContact::slotRemoveThisUser()");
//	mProtocol->removeUser(mUserID);
//	delete this;	// use one-shot timer instead?
}

void WPContact::slotRemoveFromGroup()
{
	DEBUG(WPDMETHOD, "WPContact::slotRemoveFromGroup()");
//	mProtocol->moveUser(mUserID, mGroup = QString(""), displayName()(), this);
}

void WPContact::slotMoveThisUser()
{
	DEBUG(WPDMETHOD, "WPContact::slotMoveThisUser()");
/*	if (!(mGroup = actionContactMove->currentText())) {
		hasLocalGroup = false;
	}
	else {
		hasLocalGroup = true;
	}
	mProtocol->moveUser(userID(), mGroup, displayName()(), this);
*/
}

int WPContact::importance() const
{
	DEBUG(WPDMETHOD, "WPContact::importance()");
	if(mStatus == STATUS_ONLINE)
		return 20;
    if(mStatus == STATUS_AWAY)
		return 15;
	return 0;
}

void WPContact::slotChatThisUser()
{
	DEBUG(WPDMETHOD, "WPContact::slotChatThisUser()");
	msgManagerKCW()->readMessages();
}

void WPContact::slotEmailUser()
{
	DEBUG(WPDMETHOD, "WPContact::slotChatThisUser()");
	msgManagerKEW()->readMessages();
	msgManagerKEW()->slotSendEnabled(true);
}

void WPContact::execute()
{
	DEBUG(WPDMETHOD, "WPContact::execute()");
	slotChatThisUser();
}

void WPContact::slotNewMessage(const QString &Body, const QDateTime &Arrival)
{
	DEBUG(WPDMETHOD, "WPContact::slotNewMessage(" << Body << ", " << Arrival.toString() << ")");

	QPtrList<KopeteContact> contactList;
	contactList.append(mProtocol->myself());

	QRegExp subj("^Subject: ([^\n]*)\n(.*)$");
	if(subj.search(Body) == -1)
		msgManagerKCW()->appendMessage(KopeteMessage(this, contactList, Body, KopeteMessage::Inbound));
	else
	{
		msgManagerKEW()->appendMessage(KopeteMessage(this, contactList, subj.cap(2), subj.cap(1), KopeteMessage::Inbound));
		msgManagerKEW()->slotSendEnabled(false);
	}
}

void WPContact::slotViewHistory()
{
	if(!historyDialog)
	{
		historyDialog = new KopeteHistoryDialog(QString("wp_logs/%1.log").arg(mUserID), displayName()(), true, 50, 0, "WPHistoryDialog");
		connect(historyDialog, SIGNAL(closing()), this, SLOT(slotCloseHistoryDialog()));
	}
}

void WPContact::slotCloseHistoryDialog()
{
	delete historyDialog;
	historyDialog = 0;
}

void WPContact::slotSendMsgKEW(const KopeteMessage& message)
{
	DEBUG(WPDMETHOD, "WPContact::slotSendMsg(<message>)");
	QString Message = message.body();
	if(message.subject() != "")
		Message = "Subject: " + message.subject() + "\n" + Message;
	mProtocol->slotSendMessage(Message, dynamic_cast<WPContact *>(message.to().first())->userID());
	msgManagerKEW()->appendMessage(message);
}

void WPContact::slotSendMsgKCW(const KopeteMessage& message)
{
	DEBUG(WPDMETHOD, "WPContact::slotSendMsg(<message>)");
	mProtocol->slotSendMessage(message.body(), dynamic_cast<WPContact *>(message.to().first())->userID());
	msgManagerKCW()->appendMessage(message);
}

#include "wpcontact.moc"

// vim: noet ts=4 sts=4 sw=4:
/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

