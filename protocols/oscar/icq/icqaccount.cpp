/*
  icqaccount.cpp  -  ICQ Account Class

  Copyright (c) 2002 by Chris TenHarmsel <tenharmsel@staticmethod.net>
  Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

  *************************************************************************
  *                                                                       *
  * This program is free software; you can redistribute it and/or modify  *
  * it under the terms of the GNU General Public License as published by  *
  * the Free Software Foundation; either version 2 of the License, or     *
  * (at your option) any later version.                                   *
  *                                                                       *
  *************************************************************************

*/

#include "icqaccount.h"
#include "icqcontact.h"
#include "icqprotocol.h"
#include "oscarchangestatus.h"

#include <kdebug.h>
#include <klocale.h>
#include <kpopupmenu.h>


#include <oscarcontact.h>

ICQAccount::ICQAccount(KopeteProtocol *parent, QString accountID, const char *name)
	: OscarAccount(parent, accountID, name, true)
{
	//myself() has to be created in constructor!

	// FIXME: Bad initial setting but this does not change on
	// first connect (status is set differently there)
	// Fixing this also involves fixing OscarSocket
	mStatus = ICQ_STATUS_ONLINE;

	mWebAware = true;
	mHideIP = false;
	mInvisible = false;
	mMyself = new ICQContact(accountId(), "", this, 0L);

	QObject::connect(mAwayDialog, SIGNAL(goAway(const int, const QString&)),
		this, SLOT(slotAwayDialogReturned(const int, const QString&)));
}

void ICQAccount::loaded()
{
	// needs to be here because pluginData() does not work in constructor
	QString nickName = pluginData(protocol(), QString::fromLatin1("NickName"));
	if(!nickName.isNull())
		static_cast<ICQContact *>(mMyself)->setOwnDisplayName(nickName);

	reloadPluginData();
}

ICQAccount::~ICQAccount()
{
	kdDebug(14200) << k_funcinfo << "[" << accountId() << "] deleted" << endl;
}

KActionMenu* ICQAccount::actionMenu()
{
	// mActionMenu is managed by KopeteAccount.  It is deleted when
	// it is no longer shown, so we can (safely) just make a new one here.

	KActionMenu* mActionMenu = new KActionMenu(accountId(),
		"icq_protocol", this, "ICQAccount::mActionMenu");

	ICQProtocol *p = ICQProtocol::protocol();

	KAction* mActionOnline = new KAction(p->statusOnline.caption(),
		p->statusOnline.iconFor(this), 0,
		this, SLOT(slotGoOnline()), this, "ICQAccount::mActionOnline");

	KAction* mActionOffline = new KAction(p->statusOffline.caption(),
		p->statusOffline.iconFor(this), 0,
		this, SLOT(slotGoOffline()), this, "ICQAccount::mActionOffline");

	KAction* mActionAway		= new KAction(p->statusAway.caption(),
		p->statusAway.iconFor(this), 0,
		this, SLOT(slotGoAway()), this, "ICQAccount::mActionAway");

	KAction* mActionNA = new KAction(p->statusNA.caption(),
		p->statusNA.iconFor(this), 0,
		this, SLOT(slotGoNA()), this, "ICQAccount::mActionNA");

	KAction* mActionDND = new KAction(p->statusDND.caption(),
		p->statusDND.iconFor(this), 0,
		this, SLOT(slotGoDND()), this, "ICQAccount::mActionDND");

	KAction* mActionOCC = new KAction(p->statusOCC.caption(),
		p->statusOCC.iconFor(this), 0,
		this, SLOT(slotGoOCC()), this, "ICQAccount::mActionOCC");

	KAction* mActionFFC = new KAction(p->statusFFC.caption(),
		p->statusFFC.iconFor(this), 0,
		this, SLOT(slotGoFFC()), this, "ICQAccount::mActionFFC");

	KToggleAction* mActionInvisible = new KToggleAction(i18n("Invisible"),
		"icq_invisible", 0, this, SLOT(slotToggleInvisible()), this, "ICQAccount::mActionInvisible");
	mActionInvisible->setChecked(mInvisible);

	mActionOffline->setEnabled(isConnected());

	// FIXME: allow setting these on connect
	// OscarSocket needs to be fixed for that
/*
	mActionAway->setEnabled(isConnected());
	mActionNA->setEnabled(isConnected());
	mActionDND->setEnabled(isConnected());
	mActionOCC->setEnabled(isConnected());
	mActionFFC->setEnabled(isConnected());
	mActionInvisible->setEnabled(isConnected());
*/
	mActionMenu->popupMenu()->insertTitle(
		mMyself->onlineStatus().iconFor(mMyself),
		i18n("%2 <%1>").arg(accountId()).arg(mMyself->displayName()));

	mActionMenu->insert(mActionOnline); // always first
	mActionMenu->insert(mActionFFC);
	mActionMenu->insert(mActionAway);
	mActionMenu->insert(mActionNA);
	mActionMenu->insert(mActionDND);
	mActionMenu->insert(mActionOCC);
	mActionMenu->insert(mActionOffline);
	mActionMenu->popupMenu()->insertSeparator();
	mActionMenu->insert(mActionInvisible);

	return mActionMenu;
}

void ICQAccount::connect()
{
	kdDebug(14200) << k_funcinfo << "accountId='" << accountId() << "'" << endl;
	setStatus(ICQ_STATUS_ONLINE, QString::null);
}

void ICQAccount::connect(const unsigned long status, const QString &awMessage)
{
	kdDebug(14200) << k_funcinfo << "accountId='" << accountId() <<
		"', status=" << status << ", awaymessage=" << awMessage << endl;

	// Get the screen name for this account
	QString screenName = accountId();
	QString server = pluginData(protocol(), "Server");
	QString port = pluginData(protocol(), "Port");

	if(server.isEmpty())
	{
		slotError(i18n("You have not specified a server address in the " \
			"account set up yet, please do so."), 0);
	}
	else if(port.isEmpty() || (port.toInt() < 1))
	{
		slotError(i18n("You have not specified a server port in the " \
			"account set up yet, please do so."), 0);
	}
	else if (screenName != i18n("(No Screen Name Set)") ) // FIXME: Is this needed at all?
	{
		QString password = getPassword();
		if (password.isEmpty())
		{
			slotError(i18n("Kopete is unable to attempt to signon to the " \
				"ICQ network because no password was specified in the " \
				"preferences."), 0);
		}
		else
		{
			kdDebug(14150) << k_funcinfo << accountId() <<
				": Logging in as " << screenName << endl;

			// Connect, need to normalize the name first
			mEngine->doLogin(
				server,
				port.toInt(),
				screenName,
				password,
				QString::null,
				status,
				awMessage);
		}
	}
	else
	{
		slotError(i18n("You have not specified your account name in the " \
			"account set up yet, please do so."), 0);
	}
}

void ICQAccount::slotGoOnline()
{
	if(
		myself()->onlineStatus().status() == KopeteOnlineStatus::Away ||
		myself()->onlineStatus().internalStatus() == OSCAR_FFC)
	{ // If we're away , set us available
		kdDebug(14150) << k_funcinfo << accountId() <<
			": Was AWAY or FFC, marking back" << endl;

		setAway(false, QString::null);
	}
	else if(myself()->onlineStatus().status() == KopeteOnlineStatus::Offline)
	{ // If we're offline, connect
		kdDebug(14150) << k_funcinfo << accountId() <<
			": Was OFFLINE, now connecting" << endl;

		ICQAccount::connect();
	}
	else
	{
		kdDebug(14150) << k_funcinfo << accountId() <<
			": Already ONLINE" << endl;
	}
}

void ICQAccount::slotGoNA()
{
	kdDebug(14200) << k_funcinfo << "Called" << endl;
	// Away could also be a different AWAY mode (like NA or OCC)
/*	if(
		(myself()->onlineStatus().status() == KopeteOnlineStatus::Online) ||
		(myself()->onlineStatus().status() == KopeteOnlineStatus::Away)
		)*/
	{
		mAwayDialog->show(OSCAR_NA);
	}
}

void ICQAccount::slotGoOCC()
{
	kdDebug(14200) << k_funcinfo << "Called" << endl;
	// Away could also be a different AWAY mode (like NA or OCC)
/*	if(
		(myself()->onlineStatus().status() == KopeteOnlineStatus::Online) ||
		(myself()->onlineStatus().status() == KopeteOnlineStatus::Away)
		)*/
	{
		mAwayDialog->show(OSCAR_OCC);
	}
}

void ICQAccount::slotGoFFC()
{
	kdDebug(14200) << k_funcinfo << "Called" << endl;
	// Away could also be a different AWAY mode (like NA or OCC)
/*	if(
		(myself()->onlineStatus().status() == KopeteOnlineStatus::Online) ||
		(myself()->onlineStatus().status() == KopeteOnlineStatus::Away)
		)*/
	{
		setStatus(ICQ_STATUS_SET_FFC);
	}
}

void ICQAccount::slotGoDND()
{
	kdDebug(14200) << k_funcinfo << "Called" << endl;
	// Away could also be a different AWAY mode (like NA or OCC)
/*	if(
		(myself()->onlineStatus().status() == KopeteOnlineStatus::Online) ||
		(myself()->onlineStatus().status() == KopeteOnlineStatus::Away)
		)*/
	{
		mAwayDialog->show(OSCAR_DND);
	}
}

void ICQAccount::slotToggleInvisible()
{
	kdDebug(14200) << k_funcinfo << "Called" << endl;
	setInvisible(!mInvisible);
}

void ICQAccount::setAway(bool away, const QString &awayReason)
{
	kdDebug(14200) << k_funcinfo << " " << accountId() << endl;
	if(away)
	{
/*		if((myself()->onlineStatus().status() == KopeteOnlineStatus::Online) ||
			(myself()->onlineStatus().status() == KopeteOnlineStatus::Away))*/
		{
			setStatus(ICQ_STATUS_SET_AWAY, awayReason);
		}
	}
	else
	{
/*		if(myself()->onlineStatus().status() == KopeteOnlineStatus::Away ||
			myself()->onlineStatus().internalStatus() == OSCAR_FFC)*/
		{
			setStatus(ICQ_STATUS_ONLINE);
		}
	}
}

const unsigned long ICQAccount::fullStatus(const unsigned long plainStatus)
{
	unsigned long sendStatus = plainStatus;

	if(mInvisible)
	{
		kdDebug(14200) << k_funcinfo << "ORing with invisible flag" << endl;
		sendStatus |= ICQ_STATUS_SET_INVIS;
	}

	if(!mHideIP)
	{
		kdDebug(14200) << k_funcinfo << "ORing with show ip flag" << endl;
		sendStatus |= ICQ_STATUS_SHOWIP;
	}

	if(mWebAware)
	{
		kdDebug(14200) << k_funcinfo << "ORing with web aware flag" << endl;
		sendStatus |= ICQ_STATUS_WEBAWARE;
	}
	return sendStatus;
}

void ICQAccount::setStatus(const unsigned long status,
	const QString &awayMessage)
{
	kdDebug(14200) << k_funcinfo <<
		"new status=" << status <<
		", old status=" << mStatus << endl;

	mStatus = status;
	if(!awayMessage.isNull())
		mAwayMessage = awayMessage;

	unsigned long outgoingStatus = fullStatus(status);
	if (isConnected())
	{
		kdDebug(14200) << k_funcinfo << "calling sendICQStatus(), outgoingStatus=" << outgoingStatus << endl;
		engine()->sendICQStatus(outgoingStatus);
	}
	else
	{
		kdDebug(14200) << k_funcinfo << "calling connect(), outgoingStatus=" << outgoingStatus << endl;
		ICQAccount::connect(fullStatus(status), awayMessage);
	}
}

void ICQAccount::setInvisible(bool invis)
{
	if (invis == mInvisible)
		return;

	kdDebug(14200) << k_funcinfo << "changing invisible setting to " << invis << endl;

	mInvisible = invis;

	if(isConnected())
	{
/*		if(mInvisible)
			engine()->sendChangeVisibility(3);
		else
			engine()->sendChangeVisibility(4);
*/
		setStatus(mStatus); // also sends the new invis flag
	}
}


void ICQAccount::slotAwayDialogReturned(const int awaytype, const QString &message)
{
	kdDebug(14200) << k_funcinfo << "awaytype=" << awaytype << endl;

	switch(awaytype)
	{
		case OSCAR_AWAY:
//			kdDebug(14200) << k_funcinfo << "calling setStatus for AWAY" << endl;
			setStatus(ICQ_STATUS_SET_AWAY, message);
			break;
		case OSCAR_DND:
//			kdDebug(14200) << k_funcinfo << "calling setStatus for DND" << endl;
			setStatus(ICQ_STATUS_SET_DND, message);
			break;
		case OSCAR_NA:
//			kdDebug(14200) << k_funcinfo << "calling setStatus for NA" << endl;
			setStatus(ICQ_STATUS_SET_NA, message);
			break;
		case OSCAR_OCC:
//			kdDebug(14200) << k_funcinfo << "calling setStatus for OCC" << endl;
			setStatus(ICQ_STATUS_SET_OCC, message);
			break;
	}
}

void ICQAccount::reloadPluginData()
{
	kdDebug(14200) << k_funcinfo << "Called." << endl;
	bool oldwebaware=mWebAware;
	bool oldhideip=mHideIP;

	mWebAware=(pluginData(protocol(), "WebAware").toUInt() == 1);
	mHideIP=(pluginData(protocol(), "HideIP").toUInt() == 1);

	if(isConnected() && (oldhideip != mHideIP || oldwebaware != mWebAware))
	{
		kdDebug(14200) << k_funcinfo <<
			"sending status to reflect HideIP and WebAware settings" << endl;
		setStatus(mStatus, QString::null);
	}
}

OscarContact *ICQAccount::createNewContact(
	const QString &contactId,
	const QString &displayName,
	KopeteMetaContact *parentContact)
{
	kdDebug(14200) << k_funcinfo << "contactId='" << contactId << "', displayName='" <<
		displayName << /*"', ptr parentContact=" << parentContact <<*/ endl;
	return new ICQContact(contactId,displayName,this,parentContact);
}

#include "icqaccount.moc"
// vim: set noet ts=4 sts=4 sw=4:
