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
#include "aim.h"
#include "oscarsocket.h"
#include "oscarsocket.icq.h"

ICQContact::ICQContact(const QString name, const QString displayName,
	ICQAccount *acc, KopeteMetaContact *parent)
	: OscarContact(name, displayName, acc, parent)
{
	mProtocol = static_cast<ICQProtocol *>(protocol());
	setOnlineStatus(mProtocol->statusOffline);

	// Buddy Changed
	QObject::connect(
		acc->getEngine(), SIGNAL(gotBuddyChange(UserInfo)),
		this, SLOT(slotContactChanged(UserInfo)));
	QObject::connect(
		acc->getEngine(), SIGNAL(gotIM(QString,QString,bool)),
		this, SLOT(slotIMReceived(QString,QString,bool)));
}

ICQContact::~ICQContact()
{
}

void ICQContact::slotContactChanged(UserInfo u)
{
	if (u.sn != contactname())
		return;

//	kdDebug(14200) << k_funcinfo << "Setting status for '" << displayName() << "'" << endl;

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

	//FIXME:
	/*mListContact->setEvil(u.evil);
	mListContact->setIdleTime(u.idletime);
	mListContact->setSignOnTime(u.onlinesince);*/

	setIdleTime(u.idletime); // update OscarContact idletime var
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

#include "icqcontact.moc"
// vim: set noet ts=4 sts=4 sw=4:
