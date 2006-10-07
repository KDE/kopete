/*
 * messengeraccount.cpp - Windows Live Messenger Kopete Account.
 *
 * Copyright (c) 2006 by Michaël Larouche <larouche@kde.org>
 * 
 * Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>
 *
 *************************************************************************
 *                                                                       *
 * This program is free software; you can redistribute it and/or modify  *
 * it under the terms of the GNU General Public License as published by  *
 * the Free Software Foundation; either version 2 of the License, or     *
 * (at your option) any later version.                                   *
 *                                                                       *
 *************************************************************************
 */
#include "messengeraccount.h"

// KDE includes
#include <kaction.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmenu.h>

// Kopete includes
#include "kopetemetacontact.h"
#include "kopeteonlinestatus.h"
#include "kopetecontactlist.h"

// Messenger includes
#include "messengerprotocol.h"
#include "messengercontact.h"

MessengerAccount::MessengerAccount(MessengerProtocol *protocol, const QString &accountId)
 : Kopete::PasswordedAccount(protocol, accountId.toLower(), false)
{
	setMyself( new MessengerContact(this, accountId, Kopete::ContactList::self()->myself()) );
}

MessengerAccount::~MessengerAccount()
{
	
}

KActionMenu *MessengerAccount::actionMenu()
{
	KActionMenu *messengerActionMenu = Kopete::Account::actionMenu();

	return messengerActionMenu;
}

void MessengerAccount::connectWithPassword(const QString &password)
{
	
}

void MessengerAccount::disconnect()
{
	
}

void MessengerAccount::setOnlineStatus(const Kopete::OnlineStatus& status, const Kopete::StatusMessage &reason)
{
	
}

void MessengerAccount::setStatusMessage(const Kopete::StatusMessage &statusMessage)
{
	
}

bool MessengerAccount::createContact(const QString &contactId, Kopete::MetaContact *parentMetaContact)
{
	return false;
}

#include "messengeraccount.moc"
