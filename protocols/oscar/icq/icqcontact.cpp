/*
  icqontact.cpp  -  Oscar Protocol Plugin

  Copyright (c) 2003 by Stefan Gehn
  Copyright (c) 2003 by Olivier Goffart
  Kopete    (c) 2003 by the Kopete developers  <kopete-devel@kde.org>

  *************************************************************************
  *                                                                       *
  * This program is free software; you can redistribute it and/or modify  *
  * it under the terms of the GNU General Public License as published by  *
  * the Free Software Foundation; either version 2 of the License, or     *
  * (at your option) any later version.                                   *
  *                                                                       *
  *************************************************************************
  */

#include "icqcontact.h"

#include "icqprotocol.h"
#include "icqaccount.h"

#include <time.h>

#include <qapplication.h>
#include <qregexp.h>
#include <qstylesheet.h>
#include <qtimer.h>

#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kpopupmenu.h>
#include <kfiledialog.h>

#include "kopeteaway.h"
#include "kopetemessagemanager.h"
#include "kopetemessagemanagerfactory.h"
#include "kopetemetacontact.h"
#include "kopetestdaction.h"
#include "kopetecontactlist.h"
#include "kopetegroup.h"

#include "icqaccount.h"
#include "icquserinfo.h"
#include "aim.h"
#include "oscarsocket.icq.h"

ICQContact::ICQContact(const QString name, const QString displayName,
	ICQAccount *acc, KopeteMetaContact *parent)
	: OscarContact(name, displayName, acc, parent)
{
	mProtocol = static_cast<ICQProtocol *>(protocol());
	setOnlineStatus(mProtocol->statusOffline);
	infoDialog = 0L;
	userinfoRequestSequence=0;
	userinfoReplyCount = 0;
	generalInfo.uin=0;
	generalInfo.nickName="";
	generalInfo.firstName="";
	generalInfo.lastName="";
	generalInfo.eMail="";
	generalInfo.city="";
	generalInfo.state="";
	generalInfo.phoneNumber="";
	generalInfo.faxNumber="";
	generalInfo.street="";
	generalInfo.cellularNumber="";
	generalInfo.zip="";
	generalInfo.countryCode=0;
	generalInfo.timezoneCode=0;
	generalInfo.publishEmail=false;
	generalInfo.showOnWeb=false;

	workInfo.city="";
	workInfo.state="";
	workInfo.phone="";
	workInfo.fax="";
	workInfo.address="";
	workInfo.zip="";
	workInfo.countryCode=0;
	workInfo.company="";
	workInfo.department="";
	workInfo.position="";
	workInfo.occupation=0;
	workInfo.homepage="";

	// Buddy Changed
	QObject::connect(
		acc->getEngine(), SIGNAL(gotBuddyChange(const UserInfo &)),
		this, SLOT(slotContactChanged(const UserInfo &)));
	QObject::connect(
		acc->getEngine(), SIGNAL(gotIM(QString,QString,bool)),
		this, SLOT(slotIMReceived(QString,QString,bool)));
	QObject::connect(
		acc->getEngine(), SIGNAL(gotICQGeneralUserInfo(const int, const ICQGeneralUserInfo &)),
		this, SLOT(slotUpdGeneralInfo(const int, const ICQGeneralUserInfo &)));
	QObject::connect(
		acc->getEngine(), SIGNAL(gotICQWorkUserInfo(const int, const ICQWorkUserInfo &)),
		this, SLOT(slotUpdWorkInfo(const int, const ICQWorkUserInfo &)));
	QObject::connect(
		acc->getEngine(), SIGNAL(gotICQMoreUserInfo(const int, const ICQMoreUserInfo &)),
		this, SLOT(slotUpdMoreUserInfo(const int, const ICQMoreUserInfo &)));
	QObject::connect(
		acc->getEngine(), SIGNAL(gotICQAboutUserInfo(const int, const QString &)),
		this, SLOT(slotUpdAboutUserInfo(const int, const QString &)));
}

ICQContact::~ICQContact()
{
}

void ICQContact::slotContactChanged(const UserInfo &u)
{
	if (u.sn != contactname())
		return;

	if (u.icqextstatus & ICQ_STATUS_FFC)
		setStatus(OSCAR_FFC);
	else if (u.icqextstatus & ICQ_STATUS_DND)
		setStatus(OSCAR_DND);
	else if (u.icqextstatus & ICQ_STATUS_OCC)
		setStatus(OSCAR_OCC);
	else if (u.icqextstatus & ICQ_STATUS_NA)
		setStatus(OSCAR_NA);
	else if (u.icqextstatus & ICQ_STATUS_AWAY)
		setStatus(OSCAR_AWAY);
	else
		setStatus(OSCAR_ONLINE);

	slotUpdateBuddy();
}

void ICQContact::slotOffgoingBuddy(QString sender)
{
	if(sender != contactname())
		return;

//	mListContact->setStatus(mProtocol->getOnlineStatus(OSCAR_OFFLINE));
	setOnlineStatus(mProtocol->statusOffline);
	slotUpdateBuddy();
}

void ICQContact::slotIMReceived(QString message, QString sender, bool /*isAuto*/)
{
	// Check if we're the one who sent the message
	if(sender!=contactname())
		return;

	// Build a KopeteMessage and set the body as Rich Text
	KopeteContactPtrList tmpList;
	tmpList.append(account()->myself());
	KopeteMessage msg(
		this, tmpList, message,
		KopeteMessage::Inbound, KopeteMessage::PlainText);
	manager(true)->appendMessage(msg);
}

/** Called when we want to send a message */
void ICQContact::slotSendMsg(KopeteMessage& message, KopeteMessageManager *)
{
	if (message.plainBody().isEmpty()) // no text, do nothing
		return;

	// Check to see if we're even online
	if (!account()->isConnected())
	{
		KMessageBox::sorry(qApp->mainWidget(),
			i18n("<qt>You must be logged on to ICQ before you can "
				"send a message to a user.</qt>"),
			i18n("Not Signed On"));
		return;
	}

	// FIXME: We don't do HTML in ICQ
	// we might be able to do that in AIM and we might also convert
	// HTML to RTF for ICQ type-2 messages  [mETz]
	static_cast<ICQAccount*>(account())->getEngine()->sendIM(
		message.plainBody(), contactname(), false);

	// Show the message we just sent in the chat window
	manager()->appendMessage(message);
	manager()->messageSucceeded();
}

bool ICQContact::isReachable()
{
	return true;
}

// Returns a set of custom menu items for the context menu
KActionCollection *ICQContact::customContextMenuActions()
{
	actionCollection = new KActionCollection(this);
	return actionCollection;
}

void ICQContact::setStatus(const unsigned int newStatus)
{
	if(onlineStatus().internalStatus() == newStatus)
		return;

	switch(newStatus)
	{
		case OSCAR_FFC:
			setOnlineStatus(mProtocol->statusFFC);
			break;
		case OSCAR_OFFLINE:
			setOnlineStatus(mProtocol->statusOffline);
			break;
		case OSCAR_AWAY:
			setOnlineStatus(mProtocol->statusAway);
			break;
		case OSCAR_DND:
			setOnlineStatus(mProtocol->statusDND);
			break;
		case OSCAR_NA:
			setOnlineStatus(mProtocol->statusNA);
			break;
		case OSCAR_OCC:
			setOnlineStatus(mProtocol->statusOCC);
			break;
		case OSCAR_CONNECTING:
			setOnlineStatus(mProtocol->statusConnecting); // ONLY for myself() contact
			break;
		default: // emergency choose, also OSCAR_ONLINE
			setOnlineStatus(mProtocol->statusOnline);
	}

	kdDebug(14200) << k_funcinfo << "'" << displayName() << "' is now " <<
		onlineStatus().description() << endl;
}

void ICQContact::slotUserInfo()
{
	if (!infoDialog)
	{
		bool editable = (this == account()->myself());
		infoDialog = new ICQUserInfo(this, static_cast<ICQAccount*>(account()), editable);
		if(!infoDialog)
			return;
		connect(infoDialog, SIGNAL(closing()), this, SLOT(slotCloseUserInfoDialog()));
		infoDialog->show();
	}
	else
	{
		infoDialog->raise();
	}
}

void ICQContact::slotCloseUserInfoDialog()
{
	infoDialog->delayedDestruct();
	infoDialog = 0L;
}

void ICQContact::requestUserInfo()
{
	kdDebug(14200) << k_funcinfo << "called" << endl;
	userinfoReplyCount = 0;
	userinfoRequestSequence =
		account()->getEngine()->sendReqInfo(contactname().toULong());
}


void ICQContact::slotUpdGeneralInfo(const int seq, const ICQGeneralUserInfo &inf)
{
	// compare reply's sequence with the one we sent with our last request
	if(seq != userinfoRequestSequence)
		return;

	kdDebug(14200) << k_funcinfo << "called; seq=" << seq << ", last saved seq=" <<
		userinfoRequestSequence << endl;

	generalInfo = inf;

	userinfoReplyCount++; // number of packets that
	if (userinfoReplyCount >= 4)
		emit updatedUserInfo();
}

void ICQContact::slotUpdWorkInfo(const int seq, const ICQWorkUserInfo &inf)
{
	// compare reply's sequence with the one we sent with our last request
	if(seq != userinfoRequestSequence)
		return;

// 	kdDebug(14200) << k_funcinfo << "called; seq=" << seq << ", last saved seq=" <<
// 		userinfoRequestSequence << endl;

	workInfo = inf;

	userinfoReplyCount++; // number of packets that
	if (userinfoReplyCount >= 4)
		emit updatedUserInfo();
}

void ICQContact::slotUpdMoreUserInfo(const int seq, const ICQMoreUserInfo &inf)
{
	// compare reply's sequence with the one we sent with our last request
	if(seq != userinfoRequestSequence)
		return;

// 	kdDebug(14200) << k_funcinfo << "called; seq=" << seq << ", last saved seq=" <<
// 		userinfoRequestSequence << endl;

	moreInfo = inf;

	userinfoReplyCount++; // number of packets that
	if (userinfoReplyCount >= 4)
		emit updatedUserInfo();
}

void ICQContact::slotUpdAboutUserInfo(const int seq, const QString &inf)
{
	// compare reply's sequence with the one we sent with our last request
	if(seq != userinfoRequestSequence)
		return;

// 	kdDebug(14200) << k_funcinfo << "called; seq=" << seq << ", last saved seq=" <<
// 		userinfoRequestSequence << endl;

	aboutInfo = inf;

	userinfoReplyCount++; // number of packets that
	if (userinfoReplyCount >= 4)
		emit updatedUserInfo();
}

#include "icqcontact.moc"
// vim: set noet ts=4 sts=4 sw=4:
