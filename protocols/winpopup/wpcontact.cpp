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

// Qt Includes
#include <qdatetime.h>
#include <qfont.h>
#include <qregexp.h>

// KDE Includes
#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>
#include <kpopupmenu.h>

// Kopete Includes
#include "kopetestdaction.h"
#include "kopetemessage.h"
#include "kopetemessagemanager.h"
#include "kopetecontactlist.h"
#include "kopetemetacontact.h"

// Local Includes
#include "wpcontact.h"
#include "wpprotocol.h"
#include "wpdebug.h"

WPContact::WPContact( const QString &userID, WPProtocol *protocol, KopeteMetaContact *parent ) : KopeteContact( protocol->id(), parent )
{
	DEBUG(WPDMETHOD, "WPContact::WPContact(" << userID << ", <protocol>, <parent>)");

	setDisplayName(userID);
	mProtocol = protocol;
	mUserID = userID;
	myMetaContact = parent;

	connect(mProtocol, SIGNAL(contactUpdated(QString, QString, int, QString)), this, SLOT(slotUpdateContact(QString, QString, int, QString)));
	connect(mProtocol, SIGNAL(nukeContacts(bool)), this, SLOT(slotDeleteContact()));
	connect(&checkStatus, SIGNAL(timeout()), this, SLOT(slotCheckStatus()));
	checkStatus.start(1000, false);

	initActions();
	slotUpdateContact(userID, STATUS_OFFLINE);
	mMsgManagerKEW = 0;
	mMsgManagerKCW = 0;
	historyDialog = 0;

	myActionCollection = 0;	// ughhhhhhh
}

void WPContact::slotCheckStatus()
{
//	DEBUG(WPDMETHOD, "WPContact::slotCheckStatus()");

	int oldStatus = mStatus;
	mStatus = mProtocol->checkHost(mUserID) ? STATUS_ONLINE : STATUS_OFFLINE;
	if(oldStatus != mStatus)
		emit statusChanged();
}

void WPContact::moveToGroup(const QString &from, const QString &to)
{
	DEBUG(WPDMETHOD, "WPContact::moveToGroup(" << from << ", " << to << ")");
}

void WPContact::slotDeleteContact()
{
	DEBUG(WPDMETHOD, "WPContact::slotDeleteContact()");

	delete this;
}

void WPContact::slotUserInfo()
{
	DEBUG(WPDMETHOD, "WPContact::slotUserInfo()");
}

KopeteMessageManager *WPContact::msgManagerKEW()
{
	DEBUG(WPDMETHOD, "WPContact::msgManagerKEW()");

	if(!mMsgManagerKEW)
	{	QPtrList<KopeteContact> singleContact;
		singleContact.append(this);
		mMsgManagerKEW = kopeteapp->sessionFactory()->create(mProtocol->myself(), singleContact, mProtocol, "wp_logs/" + mUserID +".log", KopeteMessageManager::Email);
		connect(mMsgManagerKEW, SIGNAL(messageSent(const KopeteMessage&,KopeteMessageManager*)),
			this, SLOT(slotSendMsgKEW(const KopeteMessage&)));
	}
	return mMsgManagerKEW;
}

KopeteMessageManager *WPContact::msgManagerKCW()
{
	DEBUG(WPDMETHOD, "WPContact::msgManagerKCW()");

	if(!mMsgManagerKCW)
	{	QPtrList<KopeteContact> singleContact;
		singleContact.append(this);
		mMsgManagerKCW = kopeteapp->sessionFactory()->create(mProtocol->myself(), singleContact, mProtocol, "wp_logs/" + mUserID +".log", KopeteMessageManager::ChatWindow);
		connect(mMsgManagerKCW, SIGNAL(messageSent(const KopeteMessage&,KopeteMessageManager*)),
			this, SLOT(slotSendMsgKCW(const KopeteMessage&)));
	}
	return mMsgManagerKCW;
}

void WPContact::initActions()
{
	DEBUG(WPDMETHOD, "WPContact::initActions()");

/*	actionChat = KopeteStdAction::sendMessage(this, SLOT(slotChatThisUser()), this, "actionChat");
	actionMessage = new KAction(i18n("Send Email Message"), "mail_generic", 0, this, SLOT(slotEmailUser()), this, "actionMessage");
	actionRemoveFromGroup = new KAction(i18n("Remove From Group"), "edittrash", 0, this, SLOT(slotRemoveFromGroup()), this, "actionRemove");
	actionRemove = KopeteStdAction::deleteContact(this, SLOT(slotRemoveThisUser()), this, "actionDelete");
	actionHistory = KopeteStdAction::viewHistory(this, SLOT(slotViewHistory()), this, "actionHistory");*/
}

/*void WPContact::showContextMenu(const QPoint& position, const QString& group)
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

	popup->popup(position);

}*/

void WPContact::slotUpdateContact(QString handle, int status)
{
	DEBUG(WPDMETHOD, "WPContact::slotUpdateContact(" << handle << ", " << status << ")");

    if(handle != userID())
		return;
	if(status != -1)
		mStatus = status;
    emit statusChanged();
}

WPContact::ContactStatus WPContact::status() const
{
//	DEBUG(WPDMETHOD, "WPContact::status()");
	if(mStatus == STATUS_ONLINE)
		return Online;
	if(mStatus == STATUS_AWAY)
		return Away;
	return Offline;
}

QString WPContact::statusText() const
{
//	DEBUG(WPDMETHOD, "WPContact::statusText()");

	if(mStatus == STATUS_ONLINE)
		return "Online";
	if(mStatus == STATUS_AWAY)
		return "Away";
	return "Offline";
}

QString WPContact::statusIcon() const
{
//	DEBUG(WPDMETHOD, "WPContact::statusIcon()");

	if(mStatus == STATUS_ONLINE)
		return "wp_available";
	if(mStatus == STATUS_AWAY)
		return "wp_away";
	return "wp_offline";
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
		historyDialog = new KopeteHistoryDialog(QString("wp_logs/%1.log").arg(mUserID), displayName(), true, 50, 0, "WPHistoryDialog");
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

QString WPContact::id() const
{
	return mUserID;
}

QString WPContact::data() const
{
	return mUserID;
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

