
/*
  icqaccount.cpp  -  ICQ Account Class

  Copyright (c) 2003 by Stefan Gehn
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

#include "icqaccount.h"
#include "icqcontact.h"
#include "icqprotocol.h"
#include "icqchangestatus.h"

#include <kdebug.h>
#include <klocale.h>
#include <kpopupmenu.h>

#include "kopetestdaction.h"

#include <oscarcontact.h>

ICQAccount::ICQAccount(KopeteProtocol *parent, QString accountID, const char *name)
	: OscarAccount(parent, accountID, name, true)
{
	//myself has to be created in constructor
	mMyself = new ICQContact(accountId(), accountId(), this, 0L);
	mAwayDialog = new ICQChangeStatus(getEngine());
}

void ICQAccount::loaded()
{
	// needs to be here because pluginData() does not work in constructor
	mMyself->rename(pluginData(protocol(), "NickName"));
}

ICQAccount::~ICQAccount()
{
//	kdDebug(14200) << k_funcinfo << "[" << accountId() << "] deleted" << endl;
	delete mAwayDialog;
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

	KAction* mActionOccupied = new KAction(p->statusOCC.caption(),
		p->statusOCC.iconFor(this), 0,
		this, SLOT(slotGoOCC()), this, "ICQAccount::mActionOccupied");

	KAction* mActionFFC = new KAction(p->statusFFC.caption(),
		p->statusFFC.iconFor(this), 0,
		this, SLOT(slotGoFFC()), this, "ICQAccount::mActionFFC");

	mActionMenu->popupMenu()->insertTitle(
		mMyself->onlineStatus().iconFor(mMyself),
		i18n("%2 <%1>").arg(accountId()).arg(mMyself->displayName()));

	mActionMenu->insert(mActionOnline); // always first
	mActionMenu->insert(mActionFFC);
	mActionMenu->insert(mActionAway);
	mActionMenu->insert(mActionNA);
	mActionMenu->insert(mActionDND);
	mActionMenu->insert(mActionOccupied);
	mActionMenu->insert(mActionOffline);
	mActionMenu->popupMenu()->insertSeparator();
	mActionMenu->insert(
		KopeteStdAction::contactInfo(myself(), SLOT(slotUserInfo()),
			mActionMenu, "ICQAccount::mActionEditInfo"));

	return mActionMenu;
}

void ICQAccount::slotGoNA()
{
	kdDebug(14200) << k_funcinfo << "Called" << endl;
	// Away could also be a different AWAY mode (like NA or OCC)
	if(
		(myself()->onlineStatus().status() == KopeteOnlineStatus::Online) ||
		(myself()->onlineStatus().status() == KopeteOnlineStatus::Away)
		)
	{
		mAwayDialog->show(OSCAR_NA);
	}
}

void ICQAccount::slotGoOCC()
{
	kdDebug(14200) << k_funcinfo << "Called" << endl;
	// Away could also be a different AWAY mode (like NA or OCC)
	if(
		(myself()->onlineStatus().status() == KopeteOnlineStatus::Online) ||
		(myself()->onlineStatus().status() == KopeteOnlineStatus::Away)
		)
	{
		mAwayDialog->show(OSCAR_OCC);
	}
}

void ICQAccount::slotGoFFC()
{
	kdDebug(14200) << k_funcinfo << "Called" << endl;
	// Away could also be a different AWAY mode (like NA or OCC)
	if(
		(myself()->onlineStatus().status() == KopeteOnlineStatus::Online) ||
		(myself()->onlineStatus().status() == KopeteOnlineStatus::Away)
		)
		mEngine->sendStatus(ICQ_STATUS_FFC);
}

void ICQAccount::slotGoDND()
{
	kdDebug(14200) << k_funcinfo << "Called" << endl;
	// Away could also be a different AWAY mode (like NA or OCC)
	if(
		(myself()->onlineStatus().status() == KopeteOnlineStatus::Online) ||
		(myself()->onlineStatus().status() == KopeteOnlineStatus::Away)
		)
		mEngine->sendStatus(ICQ_STATUS_DND);
}

void ICQAccount::setAway(bool away, const QString &awayReason)
{
	kdDebug(14200) << k_funcinfo << " " << accountId() << endl;
// TODO: Make use of away message as well
	if(away)
	{
		if((myself()->onlineStatus().status() == KopeteOnlineStatus::Online) ||
			(myself()->onlineStatus().status() == KopeteOnlineStatus::Away))
		{
			mEngine->sendStatus(ICQ_STATUS_AWAY);
		}
	}
	else
	{
		if(myself()->onlineStatus().status() == KopeteOnlineStatus::Away)
			mEngine->sendStatus(ICQ_STATUS_ONLINE);
	}
}

OscarContact *ICQAccount::createNewContact(
	const QString &contactId,
	const QString &displayName,
	KopeteMetaContact *parentContact)
{
	kdDebug(14200) << k_funcinfo << "contactId='" << contactId << "', displayName='" <<
		displayName << "', ptr parentContact=" << parentContact << endl;
	return new ICQContact(contactId,displayName,this,parentContact);
}

#include "icqaccount.moc"
// vim: set noet ts=4 sts=4 sw=4:
