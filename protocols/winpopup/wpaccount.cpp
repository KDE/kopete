/***************************************************************************
                          wpaccount.cpp  -  WP Plugin
                             -------------------
    begin                : Fri Apr 26 2002
    copyright            : (C) 2002 by Gav Wood
    email                : gav@indigoarchive.net

    Based on code from   : (C) 2002 by Duncan Mac-Vicar Prett
    email                : duncan@kde.org
 ***************************************************************************

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// QT Includes
#include <qregexp.h>

// KDE Includes
#include <kdebug.h>
#include <kpopupmenu.h>
#include <klocale.h>
#include <kconfig.h>

// Kopete Includes

// Local Includes
#include "wpaccount.h"

class KPopupMenu;

WPAccount::WPAccount(WPProtocol *parent, const QString &accountID, const char *name)
	: Kopete::Account(parent, accountID, name)
{
//	kdDebug(14170) <<  "WPAccount::WPAccount()" << endl;

	mProtocol = WPProtocol::protocol();

	// we need this before initActions
	Kopete::MetaContact *myself = Kopete::ContactList::self()->myself();
	setMyself( new WPContact(this, accountID, myself->displayName(), myself) );

//	if (excludeConnect()) connect(Kopete::OnlineStatus::Online); // ??
}

// Destructor
WPAccount::~WPAccount()
{
}

const QStringList WPAccount::getGroups()
{
	return mProtocol->getGroups();
}

const QStringList WPAccount::getHosts(const QString &Group)
{
	return mProtocol->getHosts(Group);
}

bool WPAccount::checkHost(const QString &Name)
{
//	kdDebug() << "WPAccount::checkHost: " << Name << endl;
	if (Name.upper() == QString::fromLatin1("LOCALHOST")) {
		// Assume localhost is always there, but it will not appear in the samba output.
		// Should never happen as localhost is now forbidden as contact, just for safety. GF
		return true;
	} else {
		return mProtocol->checkHost(Name);
	}
}

bool WPAccount::createContact(const QString &contactId, Kopete::MetaContact *parentContact )
{
//	kdDebug(14170) << "[WPAccount::createContact] contactId: " << contactId << endl;

	if (!contacts()[contactId]) {
		WPContact *newContact = new WPContact(this, contactId, parentContact->displayName(), parentContact);
		return newContact != 0;
	} else {
		kdDebug(14170) << "[WPAccount::addContact] Contact already exists" << endl;
	}

	return false;
}

void WPAccount::slotGotNewMessage(const QString &Body, const QDateTime &Arrival, const QString &From)
{
//	kdDebug(14170) <<  "WPAccount::slotGotNewMessage(" << Body << ", " << Arrival.toString() << ", " << From << ")" << endl;

	// Ignore messages from own host or IPs.
	// IPs can not be matched to an account anyway.
	// This should happen rarely but they make kopete crash.
	// The reason for this seems to be in ChatSessionManager? GF
	QRegExp ip("\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}");

//	kdDebug(14170) << "ip.search: " << From << " match: " << ip.search(From) << endl;

	if (From == accountId() || ip.exactMatch(From)) {
		kdDebug(14170) << "Ignoring message from own host/account or IP." << endl;
		return;
	}

	if (isConnected()) {
		if (!isAway()) {
			if(!contacts()[From]) {
				addContact(From, From, 0, Kopete::Account::DontChangeKABC);
			}
			static_cast<WPContact *>(contacts()[From])->slotNewMessage(Body, Arrival);
		}
		else {
			if (!theAwayMessage.isEmpty()) mProtocol->sendMessage(theAwayMessage, From);
		}
	 } else {
		// What to do with offline received messages?
		kdDebug(14170) << "That's strange - we got a message while offline! Ignoring." << endl;
	}
}

void WPAccount::connect(const Kopete::OnlineStatus &)
{
//	kdDebug(14170) <<  "WPAccount::Connect()" << endl;
	myself()->setOnlineStatus(mProtocol->WPOnline);
}

void WPAccount::disconnect()
{
//	kdDebug(14170) <<  "WPAccount::Disconnect()" << endl;
	myself()->setOnlineStatus(mProtocol->WPOffline);
}

/* I commented this code because deleting myself may have *dangerous* side effect, for example, for the status tracking.
void WPAccount::updateAccountId()
{
	delete myself();
	theInterface->setHostName(accountId());
	myself() = new WPContact(this, accountId(), accountId(), 0);
}*/

void WPAccount::setAway(bool status, const QString &awayMessage)
{
//	kdDebug(14170) <<  "WPAccount::setAway()" << endl;

	theAwayMessage = awayMessage;

//	if(!isConnected())
//		theInterface->goOnline();
	myself()->setOnlineStatus(status ? mProtocol->WPAway : mProtocol->WPOnline);
}

KActionMenu* WPAccount::actionMenu()
{
	kdDebug(14170) <<  "WPAccount::actionMenu()" << endl;

	/// How to remove an action from Kopete::Account::actionMenu()? GF

	KActionMenu *theActionMenu = new KActionMenu(accountId() , myself()->onlineStatus().iconFor(this), this);
	theActionMenu->popupMenu()->insertTitle(myself()->onlineStatus().iconFor(this), i18n("WinPopup (%1)").arg(accountId()));

	if (mProtocol)
	{
		KAction *goOnline = new KAction("Online", QIconSet(mProtocol->WPOnline.iconFor(this)), 0,
										 this, SLOT(connect()), theActionMenu, "actionGoAvailable");
		goOnline->setEnabled(isConnected() && isAway());
		theActionMenu->insert(goOnline);

		KAction *goAway = new KAction("Away", QIconSet(mProtocol->WPAway.iconFor(this)), 0,
									  this, SLOT(goAway()), theActionMenu, "actionGoAway");
		goAway->setEnabled(isConnected() && !isAway());
		theActionMenu->insert(goAway);

		/// One can not really go offline manually - appears online as long as samba server is running. GF

		theActionMenu->popupMenu()->insertSeparator();
		theActionMenu->insert(new KAction(i18n("Properties"),  0,
							  this, SLOT(editAccount()), theActionMenu, "actionAccountProperties"));

	}

	return theActionMenu;
}

void WPAccount::slotSendMessage(const QString &Body, const QString &Destination)
{
	kdDebug(14170) << "WPAccount::slotSendMessage(" << Body << ", " << Destination << ")" << endl;

	if (myself()->onlineStatus().status() == Kopete::OnlineStatus::Away) myself()->setOnlineStatus(mProtocol->WPOnline);
	mProtocol->sendMessage(Body, Destination);
}

void WPAccount::setOnlineStatus(const Kopete::OnlineStatus &status, const QString &reason)
{
	if (myself()->onlineStatus().status() == Kopete::OnlineStatus::Offline && status.status() == Kopete::OnlineStatus::Online)
		connect( status );
	else if (myself()->onlineStatus().status() != Kopete::OnlineStatus::Offline && status.status() == Kopete::OnlineStatus::Offline)
		disconnect();
	else if (myself()->onlineStatus().status() != Kopete::OnlineStatus::Offline && status.status() == Kopete::OnlineStatus::Away)
		setAway( true, reason );
}

#include "wpaccount.moc"

// vim: set noet ts=4 sts=4 sw=4:
// kate: tab-width 4; indent-width 4; replace-trailing-space-save on;
