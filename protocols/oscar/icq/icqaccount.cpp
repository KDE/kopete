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
#include "kopetepassword.h"

#include <kdebug.h>
#include <klocale.h>
#include <kpopupmenu.h>


ICQAccount::ICQAccount(Kopete::Protocol *parent, QString accountID, const char *name)
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
	// mActionMenu is managed by Kopete::Account.  It is deleted when
	// it is no longer shown, so we can (safely) just make a new one here.

	KActionMenu* mActionMenu = Kopete::Account::actionMenu();

	KToggleAction* mActionInvisible = new KToggleAction(i18n("Invisible"),
		"icq_invisible", 0, this, SLOT(slotToggleInvisible()), this, "ICQAccount::mActionInvisible");
	mActionInvisible->setChecked(mInvisible);

	KToggleAction* mActionSendSMS = new KToggleAction(i18n("Send SMS..."),
		0, 0, this, SLOT(slotSendSMS()), this, "ICQAccount::mActionSendSMS");

	mActionMenu->popupMenu()->insertSeparator();
	mActionMenu->insert(mActionInvisible);
	mActionMenu->popupMenu()->insertSeparator();
	mActionMenu->insert(mActionSendSMS);

	return mActionMenu;
}


// ----------------------


void ICQAccount::connectWithPassword( const QString & )
{
	Kopete::OnlineStatus status = initialStatus();
	kdDebug(14153) << k_funcinfo << "accountId='" << accountId() << "'" <<
		" initialStatus=" << (int)status.status() << endl;

	if (status.status() == Kopete::OnlineStatus::Away)
		setStatus(ICQ_STATUS_SET_AWAY);
	else
		setStatus(ICQ_STATUS_ONLINE);
}


void ICQAccount::slotGoOnline()
{
	kdDebug(14153) << k_funcinfo << "account='" << accountId() << "'" << endl;
	setStatus(ICQ_STATUS_ONLINE, QString::null);
}

void ICQAccount::slotGoAway(const QString &reason)
{
	kdDebug(14153) << k_funcinfo << "account='" << accountId() << "'" << endl;
	setStatus(ICQ_STATUS_SET_AWAY, reason);
}

void ICQAccount::slotGoNA(const QString &reason)
{
	kdDebug(14153) << k_funcinfo << "account='" << accountId() << "'" << endl;
	setStatus(ICQ_STATUS_SET_NA, reason);
}

void ICQAccount::slotGoOCC(const QString &reason)
{
	kdDebug(14153) << k_funcinfo << "account='" << accountId() << "'" << endl;
	setStatus(ICQ_STATUS_SET_OCC, reason);
}

void ICQAccount::slotGoFFC(const QString &reason)
{
	kdDebug(14153) << k_funcinfo << "account='" << accountId() << "'" << endl;
	setStatus(ICQ_STATUS_SET_FFC, reason);
}

void ICQAccount::slotGoDND(const QString &reason)
{
	kdDebug(14153) << k_funcinfo << "account='" << accountId() << "'" << endl;
	setStatus(ICQ_STATUS_SET_DND, reason);
}

void ICQAccount::slotToggleInvisible()
{
	kdDebug(14153) << k_funcinfo << "account='" << accountId() << "'" << endl;
	setInvisible(!mInvisible);
}


void ICQAccount::setAway(bool away, const QString &awayReason)
{
	kdDebug(14153) << k_funcinfo << "account='" << accountId() << "'" << endl;
	if(away)
		setStatus(ICQ_STATUS_SET_AWAY, awayReason);
	else
		setStatus(ICQ_STATUS_ONLINE);
}


unsigned long ICQAccount::fullStatus(unsigned long plainStatus)
{
	unsigned long sendStatus = plainStatus;

	if(mInvisible)
	{
		kdDebug(14153) << k_funcinfo << "ORing with invisible flag" << endl;
		sendStatus |= ICQ_STATUS_SET_INVIS;
	}

	if(!mHideIP)
	{
		kdDebug(14153) << k_funcinfo << "ORing with show ip flag" << endl;
		sendStatus |= ICQ_STATUS_SHOWIP;
	}

	if(mWebAware)
	{
		kdDebug(14153) << k_funcinfo << "ORing with web aware flag" << endl;
		sendStatus |= ICQ_STATUS_WEBAWARE;
	}
	return sendStatus;
}


void ICQAccount::setStatus(unsigned long status,
	const QString &awayMessage)
{
	kdDebug(14153) << k_funcinfo <<
		"new status=" << status <<
		", old status=" << mStatus << endl;

	mStatus = status;
	setAwayMessage(awayMessage);

	unsigned long outgoingStatus = fullStatus(status);
	if (isConnected())
	{
		/*kdDebug(14153) << k_funcinfo <<
			"calling sendICQStatus(), outgoingStatus = " <<
			outgoingStatus << endl;*/
		engine()->sendICQStatus(outgoingStatus);
	}
	else
	{
		kdDebug(14153) << k_funcinfo << "LOGGING IN; accountId='" <<
			accountId() << "', status=" << outgoingStatus <<
			", awaymessage=" << awayMessage << endl;

		QString server = pluginData(protocol(), "Server");
		if(server.isEmpty())
			server = ICQ_SERVER;
		QString port = pluginData(protocol(), "Port");
		if(port.isEmpty() || (port.toInt() < 1))
			port = ICQ_PORT;

		QString pass = password().cachedValue();
		if (pass.isEmpty())
		{
			new ConnectTask( this, status, awayMessage );
		}
		else
		{
			// Connect, need to normalize the name first
			engine()->doLogin(
				server,
				port.toInt(),
				accountId(),
				pass,
				QString::null,
				outgoingStatus,
				awayMessage);
		}
	}
}

void ICQAccount::setInvisible(bool invis)
{
	if (invis == mInvisible)
		return;

	kdDebug(14153) << k_funcinfo <<
		"changing invisible setting to " << invis << endl;

	mInvisible = invis;

	if(isConnected())
		setStatus(mStatus); // also sends the new invis flag
}

void ICQAccount::slotSendSMS()
{
	//kdDebug(14153) << k_funcinfo << endl;
	ICQSendSMSDialog *smsDialog = new ICQSendSMSDialog(this, 0L, 0L, "smsDialog");
	smsDialog->exec();
	delete smsDialog;
}

void ICQAccount::reloadPluginData()
{
	kdDebug(14153) << k_funcinfo << "Called." << endl;
	bool oldwebaware = mWebAware;
	bool oldhideip = mHideIP;

//	setIgnoreUnknownContacts(pluginData(protocol(), "IgnoreUnknownContacts").toUInt() == 1);
	mWebAware=(pluginData(protocol(), "WebAware").toUInt() == 1);
	mHideIP=(pluginData(protocol(), "HideIP").toUInt() == 1);

	if(isConnected() && (oldhideip != mHideIP || oldwebaware != mWebAware))
	{
		kdDebug(14153) << k_funcinfo <<
			"sending status to reflect HideIP and WebAware settings" << endl;
		setStatus(mStatus, QString::null);
	}
}

OscarContact *ICQAccount::createNewContact(const QString &contactId,
	const QString &displayName, Kopete::MetaContact *parentContact,
	bool isOnSSI)
{
	/*kdDebug(14153) << k_funcinfo <<
		"contactId='" << contactId <<
		"', displayName='" << displayName << endl;*/

	ICQContact* contact = new ICQContact(contactId, displayName, this, parentContact);
	contact->setServerSide( isOnSSI );
	return contact;
}

#include "icqaccount.moc"
// vim: set noet ts=4 sts=4 sw=4:
