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
#include <qcursor.h>
#include <qprocess.h>
#include <qfile.h>
#include <qregexp.h>

// KDE Includes
#include <kaction.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kgenericfactory.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kpopupmenu.h>
#include <kpushbutton.h>
#include <ksimpleconfig.h>
#include <kstandarddirs.h>
#include <kstdguiitem.h>

// Local Includes
#include "wpaccount.h"
#include "wpdebug.h"
#include "wpcontact.h"
#include "wpprotocol.h"
#include "wpaddcontact.h"
#include "wppreferences.h"

// Kopete Includes
#include "kopetemetacontact.h"

class KPopupMenu;

WPAccount::WPAccount(WPProtocol *parent, const QString &accountID, const char *name) : KopeteAccount(parent, accountID, name)
{
	DEBUG(WPDMETHOD, "WPAccount::WPAccount()");

	// setup internals
	theInterface = 0;
	QString theHostName = accountID;

	// Create the interface...
	theInterface = parent->createInterface(theHostName);

	// we need this before initActions
	theMyself = new WPContact(this, theHostName, theHostName, 0);

	if(autoLogin()) connect();

	QObject::connect(theInterface, SIGNAL(newMessage(const QString &, const QDateTime &, const QString &)), this, SLOT(slotGotNewMessage(const QString &, const QDateTime &, const QString &)));
	QObject::connect(theInterface, SIGNAL(accountIdChanged()), this, SLOT(updateAccountId()));
}

// Destructor
WPAccount::~WPAccount()
{
	DEBUG(WPDMETHOD, "WPAccount::~WPAccount()");

	static_cast<WPProtocol *>(protocol())->destroyInterface(theInterface);
}

KopeteContact *WPAccount::myself() const
{
	return theMyself;
}

const QStringList WPAccount::getGroups()
{
	kdDebug(14180) << "[WPAccount::getGroups] this: " << this << ", theInterface: " << theInterface << endl;
	return theInterface->getGroups();
}

const QStringList WPAccount::getHosts(const QString &Group)
{
	kdDebug(14180) << "[WPAccount::getHosts] this: " << this << ", theInterface: " << theInterface << endl;
	return theInterface->getHosts(Group);
}

bool WPAccount::checkHost(const QString &Name)
{
	return theInterface->checkHost(Name);
}

bool WPAccount::addContactToMetaContact(const QString &contactId, const QString &displayName, KopeteMetaContact *parentContact )
{
	kdDebug(14180) << "[WPAccount::addContactToMetaContact] contactId: " << contactId << endl;

	if(!contacts()[contactId])
	{
		WPContact *newContact = new WPContact(this, contactId, displayName, parentContact);
		return newContact != 0;
	}
	else
		kdDebug(14180) << "[WPAccount::addContact] Contact already exists" << endl;

	return false;
}

void WPAccount::slotGotNewMessage(const QString &Body, const QDateTime &Arrival, const QString &From)
{
	DEBUG(WPDMETHOD, "WPAccount::slotGotNewMessage(" << Body << ", " << Arrival.toString() << ", " << From << ")");

	if(isConnected())
		if(!isAway())
		{	if(!contacts()[From]) addContact(From, From, 0, QString::null, true);
			static_cast<WPContact *>(contacts()[From])->slotNewMessage(Body, Arrival);
		}
		else
			theInterface->slotSendMessage(theAwayMessage, From);
	else
		DEBUG(WPDINFO, "WinPopup: That's strange - we got a message while offline! Ignoring.");
}

void WPAccount::connect()
{
	DEBUG(WPDMETHOD, "WPAccount::Connect()");

	theInterface->goOnline();
	myself()->setOnlineStatus(static_cast<WPProtocol *>(protocol())->WPOnline);
}

void WPAccount::disconnect()
{
	DEBUG(WPDMETHOD, "WPAccount::Disconnect()");

	theInterface->goOffline();
	myself()->setOnlineStatus(static_cast<WPProtocol *>(protocol())->WPOffline);
}

void WPAccount::updateAccountId()
{
	delete theMyself;
	theInterface->setHostName(accountId());
	theMyself = new WPContact(this, accountId(), accountId(), 0);
}

void WPAccount::setAway(bool status, const QString &awayMessage)
{
	DEBUG(WPDMETHOD, "WPAccount::setAway()");

	theAwayMessage = awayMessage.isNull() ? QString::fromLatin1("I'm away at the moment.") : awayMessage;

	if(!isConnected())
		theInterface->goOnline();
	myself()->setOnlineStatus(status ? static_cast<WPProtocol *>(protocol())->WPAway : static_cast<WPProtocol *>(protocol())->WPOnline);
}

KActionMenu* WPAccount::actionMenu()
{
	DEBUG(WPDMETHOD, "WPAccount::actionMenu()");

	WPProtocol *theProtocol = dynamic_cast<WPProtocol *>(protocol());
	KActionMenu *theActionMenu = new KActionMenu("WinPopup", this);
	theActionMenu->popupMenu()->insertTitle(theMyself->icon(), "WinPopup (" + accountId() + ")");

	theActionMenu->insert(new KAction("Online", QIconSet(theProtocol->WPOnline.iconFor(this)), 0, this, SLOT(connect()), theActionMenu, "actionGoAvailable"));
	theActionMenu->insert(new KAction("Away", QIconSet(theProtocol->WPAway.iconFor(this)), 0, this, SLOT(goAway()), theActionMenu, "actionGoAway"));
	theActionMenu->insert(new KAction("Offline", QIconSet(theProtocol->WPOffline.iconFor(this)), 0, this, SLOT(disconnect()), theActionMenu, "actionGoOffline"));

	return theActionMenu;
}

void WPAccount::slotSendMessage(const QString &Body, const QString &Destination)
{
	DEBUG(WPDMETHOD, "WPAccount::slotSendMessage(" << Body << ", " << Destination << ")");

	theInterface->sendMessage(Body, Destination);
}

#include "wpaccount.moc"

// vim: set noet ts=4 sts=4 sw=4:

