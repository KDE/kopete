
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

#include <qapplication.h>
#include <qwidget.h>
#include <qtimer.h>

#include <kaction.h>
#include <kdebug.h>
#include <klocale.h>
#include <kpopupmenu.h>
#include <kmessagebox.h>

#include "kopetecontact.h"
#include "kopetegroup.h"
#include "kopeteaway.h"
#include "kopetemetacontact.h"
#include "kopetecontactlist.h"
#include "kopetestdaction.h"

#include "oscaraccount.h"
#include "oscarsocket.h"
#include "oscarsocket.icq.h"
#include "oscarcontact.h"

#include "icqchangestatus.h"
//#include "icquserinfo.h"

#include <klineeditdlg.h>
//	OscarAccount(KopeteProtocol *parent, QString accountID, const char *name=0L, bool isICQ=false);

ICQAccount::ICQAccount(KopeteProtocol *parent, QString accountID, const char *name)
	: OscarAccount(parent, accountID, name, true)
{
	// Init the myself contact
	mMyself = new ICQContact( accountID, accountID, this , 0L );

	mAwayDialog = new ICQChangeStatus(getEngine());
}
ICQAccount::~ICQAccount()
{
//	kdDebug(14200) << k_funcinfo << "[" << accountId() << "] deleted" << endl;
	delete mAwayDialog;
}


KActionMenu* ICQAccount::actionMenu()
{
	KActionMenu* mActionMenu=new KActionMenu(accountId(), "icq_online", this, "ICQAccount::mActionMenu");

	KAction* mActionGoOnline	= new KAction(i18n("Online"), static_cast<ICQProtocol*>(protocol())->statusOnline.iconFor( this ) , 0, this, SLOT(slotGoOnline()), this, "mActionGoOnline");
	KAction* mActionGoOffline	= new KAction(i18n("Offline"), static_cast<ICQProtocol*>(protocol())->statusOffline.iconFor( this ), 0, this, SLOT(slotGoOffline()), this, "mActionGoOffline");
	KAction* mActionGoAway		= new KAction(i18n("Away"),static_cast<ICQProtocol*>(protocol())->statusAway.iconFor( this ), 0, this, SLOT(slotGoAway()), this, "mActionGoAway");
	KAction* mActionGoNA			= new KAction(i18n("Not Available"), static_cast<ICQProtocol*>(protocol())->statusNA.iconFor( this ), 0, this, SLOT(slotGoNA()), this, "mActionGoNA");
	KAction* mActionGoDND		= new KAction(i18n("Do Not Disturb"), static_cast<ICQProtocol*>(protocol())->statusDND.iconFor( this ), 0, this, SLOT(slotGoDND()), this, "mActionGoDND");
	KAction* mActionGoOccupied	= new KAction(i18n("Occupied"), static_cast<ICQProtocol*>(protocol())->statusOCC.iconFor( this ), 0, this, SLOT(slotGoOCC()), this, "mActionGoOccupied");
	KAction* mActionGoFFC		= new KAction(i18n("Free For Chat"), static_cast<ICQProtocol*>(protocol())->statusFFC.iconFor( this ), 0, this, SLOT(slotGoFFC()), this, "mActionGoFFC");
//	mActionEditInfo = 0L; // TODO: can't send/retrieve info yet so no menuitem
	KAction* mActionFastAddContact = new KAction(i18n("Fast add a Contact"), "", 0, this, SLOT(slotFastAddContact()), this, "actionFastAddContact" );

	mActionMenu->insert(mActionGoOnline); // always first
	mActionMenu->insert(mActionGoFFC);
	mActionMenu->insert(mActionGoAway);
	mActionMenu->insert(mActionGoNA);
	mActionMenu->insert(mActionGoDND);
	mActionMenu->insert(mActionGoOccupied);
	mActionMenu->insert(mActionGoOffline);
	mActionMenu->popupMenu()->insertSeparator();
	mActionMenu->insert(mActionFastAddContact);

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
		mEngine->sendStatus(ICQ_STATUS_NA);
}

void ICQAccount::slotGoOCC()
{
	kdDebug(14200) << k_funcinfo << "Called" << endl;
	// Away could also be a different AWAY mode (like NA or OCC)
	if(
		(myself()->onlineStatus().status() == KopeteOnlineStatus::Online) ||
		(myself()->onlineStatus().status() == KopeteOnlineStatus::Away)
		)
		mEngine->sendStatus(ICQ_STATUS_OCC);
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
		mEngine->sendStatus(ICQ_STATUS_AWAY);
	else
		mEngine->sendStatus(ICQ_STATUS_ONLINE);
}

OscarContact *ICQAccount::createNewContact(
	const QString &contactId,
	const QString &displayName,
	KopeteMetaContact *parentContact)
{
	return new ICQContact(contactId,displayName,this,parentContact);
}

#include "icqaccount.moc"
// vim: set noet ts=4 sts=4 sw=4:
