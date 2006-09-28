/*
 * telepathyaccount.cpp - Telepathy Kopete Account.
 *
 * Copyright (c) 2006 by Michaël Larouche <michael.larouche@kdemail.net>
 *
 * Kopete    (c) 2002-2006 by the Kopete developers  <kopete-devel@kde.org>
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
#include "telepathyaccount.h"

// KDE includes
#include <kaction.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmenu.h>

// Kopete includes
#include "kopetemetacontact.h"
#include "kopeteonlinestatus.h"
#include "kopetecontactlist.h"

// Telepathy includes
#include "telepathyprotocol.h"
#include "telepathycontact.h"

TelepathyAccount::TelepathyAccount(TelepathyProtocol *protocol, const QString &accountId)
 : Kopete::Account(protocol, accountId.toLower())
{
	setMyself( new TelepathyContact(this, accountId, Kopete::ContactList::self()->myself()) );
}

TelepathyAccount::~TelepathyAccount()
{
	
}

KActionMenu *TelepathyAccount::actionMenu()
{
	KActionMenu *actionMenu = Kopete::Account::actionMenu();

	return actionMenu;
}

void TelepathyAccount::connect(const Kopete::OnlineStatus &initialStatus)
{
	
}

void TelepathyAccount::disconnect()
{
	
}

void TelepathyAccount::setOnlineStatus(const Kopete::OnlineStatus& status, const Kopete::StatusMessage &reason)
{
	
}

void TelepathyAccount::setStatusMessage(const Kopete::StatusMessage &statusMessage)
{
	
}

bool TelepathyAccount::createContact(const QString &contactId, Kopete::MetaContact *parentMetaContact)
{
	return false;
}

#include "telepathyaccount.moc"
