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
#include "icqsendsmsdialog.h"
#include "oscarcontact.h"
#include "kopeteawayaction.h"

#include <kdebug.h>
#include <klocale.h>
#include <kpopupmenu.h>


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
	setMyself( new ICQContact(accountId(), QString::null, this, 0L) );
}

void ICQAccount::loaded()
{
	// needs to be here because pluginData() does not work in constructor
	QString nickName = pluginData(protocol(), QString::fromLatin1("NickName"));
	if(!nickName.isNull())
		static_cast<ICQContact *>(myself())->setOwnDisplayName(nickName);

	reloadPluginData();
}

ICQAccount::~ICQAccount()
{
}

KActionMenu* ICQAccount::actionMenu()
{
	// mActionMenu is managed by KopeteAccount.  It is deleted when
	// it is no longer shown, so we can (safely) just make a new one here.

	KActionMenu* mActionMenu = new KActionMenu(accountId(),
		myself()->onlineStatus().iconFor(this), this, "ICQAccount::mActionMenu");

	ICQProtocol *p = ICQProtocol::protocol();

	KAction* mActionOnline = new KAction(p->statusOnline.caption(),
		p->statusOnline.iconFor(this), 0,
		this, SLOT(slotGoOnline()), this, "ICQAccount::mActionOnline");

	KAction* mActionOffline = new KAction(p->statusOffline.caption(),
		p->statusOffline.iconFor(this), 0,
		this, SLOT(slotGoOffline()), this, "ICQAccount::mActionOffline");

	KopeteAwayAction* mActionAway = new KopeteAwayAction(p->statusAway.caption(),
		p->statusAway.iconFor(this), 0, 
		this, SLOT(slotGoAway( const QString & )), this, "ICQAccount::mActionAway" );
	
	KopeteAwayAction* mActionNA = new KopeteAwayAction(p->statusNA.caption(),
		p->statusNA.iconFor(this), 0, 
		this, SLOT(slotGoNA( const QString & )), this, "ICQAccount::mActionNA" );

	KopeteAwayAction* mActionDND = new KopeteAwayAction(p->statusDND.caption(),
		p->statusDND.iconFor(this), 0, 
		this, SLOT(slotGoDND( const QString & )), this, "ICQAccount::mActionDND" );

	KopeteAwayAction* mActionOCC = new KopeteAwayAction(p->statusOCC.caption(),
		p->statusOCC.iconFor(this), 0, 
		this, SLOT(slotGoOCC( const QString & )), this, "ICQAccount::mActionOCC" );

	KopeteAwayAction* mActionFFC = new KopeteAwayAction(p->statusFFC.caption(),
		p->statusFFC.iconFor(this), 0, 
		this, SLOT(slotGoFFC( const QString & )), this, "ICQAccount::mActionFCC" );

	KToggleAction* mActionInvisible = new KToggleAction(i18n("Invisible"),
		"icq_invisible", 0, this, SLOT(slotToggleInvisible()), this, "ICQAccount::mActionInvisible");
	mActionInvisible->setChecked(mInvisible);

	KToggleAction* mActionSendSMS = new KToggleAction(i18n("Send SMS..."),
		0, 0, this, SLOT(slotSendSMS()), this, "ICQAccount::mActionSendSMS");

	mActionOffline->setEnabled(isConnected());

	mActionMenu->popupMenu()->insertTitle(
		myself()->onlineStatus().iconFor(myself()),
		i18n("%2 <%1>").arg(accountId()).arg(myself()->displayName()));

	mActionMenu->insert(mActionOnline); // always first
	mActionMenu->insert(mActionFFC);
	mActionMenu->insert(mActionAway);
	mActionMenu->insert(mActionNA);
	mActionMenu->insert(mActionDND);
	mActionMenu->insert(mActionOCC);
	mActionMenu->insert(mActionOffline);
	mActionMenu->popupMenu()->insertSeparator();
	mActionMenu->insert(mActionInvisible);
	mActionMenu->popupMenu()->insertSeparator();
	mActionMenu->insert(mActionSendSMS);

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
		QString _password = password(passwordWasWrong(), 0L, 8);
		if (_password.isEmpty())
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
			engine()->doLogin(
				server,
				port.toInt(),
				screenName,
				_password,
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

void ICQAccount::slotGoAway( const QString &reason )
{
	kdDebug(14200) << k_funcinfo << "account='" << accountId() << "'" << endl;
	setStatus(ICQ_STATUS_SET_AWAY, reason);
}

void ICQAccount::slotGoNA( const QString &reason )
{
	kdDebug(14200) << k_funcinfo << "account='" << accountId() << "'" << endl;
	setStatus(ICQ_STATUS_SET_NA, reason);
}

void ICQAccount::slotGoOCC( const QString &reason )
{
	kdDebug(14200) << k_funcinfo << "account='" << accountId() << "'" << endl;
	setStatus(ICQ_STATUS_SET_OCC, reason);
}

void ICQAccount::slotGoFFC( const QString &reason )
{
	kdDebug(14200) << k_funcinfo << "account='" << accountId() << "'" << endl;
	setStatus(ICQ_STATUS_SET_FFC, reason);
}

void ICQAccount::slotGoDND( const QString &reason )
{
	kdDebug(14200) << k_funcinfo << "account='" << accountId() << "'" << endl;
	setStatus(ICQ_STATUS_SET_DND, reason);
}

void ICQAccount::slotToggleInvisible()
{
	kdDebug(14200) << k_funcinfo << "account='" << accountId() << "'" << endl;
	setInvisible(!mInvisible);
}

void ICQAccount::setAway(bool away, const QString &awayReason)
{
	kdDebug(14200) << k_funcinfo << "account='" << accountId() << "'" << endl;
	if(away)
		setStatus(ICQ_STATUS_SET_AWAY, awayReason);
	else
		setStatus(ICQ_STATUS_ONLINE);
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
		setAwayMessage(awayMessage);

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

void ICQAccount::slotSendSMS()
{
	kdDebug(14200) << k_funcinfo << endl;

	ICQSendSMSDialog *smsDialog = new ICQSendSMSDialog(this, 0L, 0L, "smsDialog");
	if(!smsDialog)
		return;
	smsDialog->exec();
	delete smsDialog;
}

void ICQAccount::reloadPluginData()
{
	kdDebug(14200) << k_funcinfo << "Called." << endl;
	bool oldwebaware=mWebAware;
	bool oldhideip=mHideIP;

//	setIgnoreUnknownContacts(pluginData(protocol(), "IgnoreUnknownContacts").toUInt() == 1);
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
	/*kdDebug(14200) << k_funcinfo <<
		"contactId='" << contactId <<
		"', displayName='" << displayName << endl;*/
	return new ICQContact(contactId, displayName, this, parentContact);
}

#include "icqaccount.moc"
// vim: set noet ts=4 sts=4 sw=4:
