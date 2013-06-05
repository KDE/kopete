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

// Local Includes
#include "wpaccount.h"

// QT Includes
#include <qregexp.h>
#include <QIcon>

// KDE Includes
#include <kdebug.h>
#include <kmenu.h>
#include <klocale.h>
#include <kconfig.h>
#include <kactionmenu.h>
#include <kicon.h>

// Kopete Includes


class KMenu;

WPAccount::WPAccount(WPProtocol *parent, const QString &accountID)
	: Kopete::Account(parent, accountID)
{
//	kDebug(14170) <<  "WPAccount::WPAccount()";

	mProtocol = WPProtocol::protocol();

	// we need this before initActions
	Kopete::MetaContact *myself = Kopete::ContactList::self()->myself();
	setMyself( new WPContact(this, accountID, QString(), myself) );

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
//	kDebug() << "WPAccount::checkHost: " << Name;
	if (Name.toUpper() == QString::fromLatin1("LOCALHOST")) {
		// Assume localhost is always there, but it will not appear in the samba output.
		// Should never happen as localhost is now forbidden as contact, just for safety. GF
		return true;
	} else {
		return mProtocol->checkHost(Name);
	}
}

bool WPAccount::createContact(const QString &contactId, Kopete::MetaContact *parentContact )
{
//	kDebug(14170) << "[WPAccount::createContact] contactId: " << contactId;

	if (!contacts().value(contactId)) {
		WPContact *newContact = new WPContact(this, contactId, parentContact->displayName(), parentContact);
		return newContact != 0;
	} else {
		kDebug(14170) << "[WPAccount::addContact] Contact already exists";
	}

	return false;
}

void WPAccount::slotGotNewMessage(const QString &Body, const QDateTime &Arrival, const QString &From)
{
//	kDebug(14170) <<  "WPAccount::slotGotNewMessage(" << Body << ", " << Arrival.toString() << ", " << From << ")";

	// Ignore messages from own host or IPs.
	// IPs cannot be matched to an account anyway.
	// This should happen rarely but they make kopete crash.
	// The reason for this seems to be in ChatSessionManager? GF
	QRegExp ip("\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}");

//	kDebug(14170) << "ip.search: " << From << " match: " << ip.search(From);

	if (From == accountId() || ip.exactMatch(From)) {
		kDebug(14170) << "Ignoring message from own host/account or IP.";
		return;
	}

	if (isConnected()) {
		if (!isAway()) {
			if(!contacts().value(From)) {
				addContact(From, From, 0, Kopete::Account::DontChangeKABC);
			}
			static_cast<WPContact *>(contacts().value(From))->slotNewMessage(Body, Arrival);
		}
		else {
			if (!theAwayMessage.isEmpty()) mProtocol->sendMessage(theAwayMessage, From);
		}
	 } else {
		// What to do with offline received messages?
		kDebug(14170) << "That's strange - we got a message while offline! Ignoring.";
	}
}

void WPAccount::connect(const Kopete::OnlineStatus &)
{
//	kDebug(14170) <<  "WPAccount::Connect()";
	myself()->setOnlineStatus(mProtocol->WPOnline);
}

void WPAccount::disconnect()
{
//	kDebug(14170) <<  "WPAccount::Disconnect()";
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
//	kDebug(14170) <<  "WPAccount::setAway()";

	theAwayMessage = awayMessage;

//	if(!isConnected())
//		theInterface->goOnline();
	myself()->setOnlineStatus(status ? mProtocol->WPAway : mProtocol->WPOnline);
	myself()->setStatusMessage( Kopete::StatusMessage(theAwayMessage) );
}

void WPAccount::fillActionMenu( KActionMenu *actionMenu )
{
	kDebug(14170);

	/// How to remove an action from Kopete::Account::actionMenu()? GF

	actionMenu->setIcon( myself()->onlineStatus().iconFor(this) );
	actionMenu->menu()->addTitle( QIcon(myself()->onlineStatus().iconFor(this)), i18n("WinPopup (%1)", accountId()));

	if (mProtocol)
	{
		KAction *goOnline = new KAction( KIcon(QIcon(mProtocol->WPOnline.iconFor(this))), i18n("Online"), this );
		//, "actionGoAvailable" );
		QObject::connect( goOnline, SIGNAL(triggered(bool)), this, SLOT(connect()) );
		goOnline->setEnabled(isConnected() && isAway());
		actionMenu->addAction(goOnline);

		KAction *goAway = new KAction( KIcon(QIcon(mProtocol->WPAway.iconFor(this))), i18n("Away"), this );
                //, "actionGoAway" );
		QObject::connect( goAway, SIGNAL(triggered(bool)), this, SLOT(goAway()) );
		goAway->setEnabled(isConnected() && !isAway());
		actionMenu->addAction(goAway);

		/// One cannot really go offline manually - appears online as long as samba server is running. GF

		actionMenu->addSeparator();
		KAction *properties = new KAction( i18n("Properties"), this );
                // "actionAccountProperties" );
		QObject::connect( properties, SIGNAL(triggered(bool)), this, SLOT(editAccount()) );
		actionMenu->addAction( properties );
	}
}

bool WPAccount::hasCustomStatusMenu() const
{
	return true;
}

void WPAccount::slotSendMessage(const QString &Body, const QString &Destination)
{
	kDebug(14170) << "WPAccount::slotSendMessage(" << Body << ", " << Destination << ")";

	if (myself()->onlineStatus().status() == Kopete::OnlineStatus::Away ||
		myself()->onlineStatus().status() == Kopete::OnlineStatus::Busy)
		myself()->setOnlineStatus(mProtocol->WPOnline);
	mProtocol->sendMessage(Body, Destination);
}

void WPAccount::setOnlineStatus(const Kopete::OnlineStatus &status, const Kopete::StatusMessage &reason, const OnlineStatusOptions& /*options*/)
{
	if (myself()->onlineStatus().status() == Kopete::OnlineStatus::Offline && status.status() == Kopete::OnlineStatus::Online)
		connect( status );
	else if (myself()->onlineStatus().status() != Kopete::OnlineStatus::Offline && status.status() == Kopete::OnlineStatus::Offline)
		disconnect();
	else if (myself()->onlineStatus().status() != Kopete::OnlineStatus::Offline && status.status() == Kopete::OnlineStatus::Away)
		setAway( true, reason.message() );
}

void WPAccount::setStatusMessage(const Kopete::StatusMessage &statusMessage)
{
	if(myself()->onlineStatus().status() == Kopete::OnlineStatus::Online)
		setAway( false, statusMessage.message() );
	else if(myself()->onlineStatus().status() == Kopete::OnlineStatus::Away ||
		myself()->onlineStatus().status() == Kopete::OnlineStatus::Busy)
		setAway( true, statusMessage.message() );
}
#include "wpaccount.moc"

// vim: set noet ts=4 sts=4 sw=4:
// kate: tab-width 4; indent-width 4; replace-trailing-space-save on;
