/*
 * This file is part of Kopete
 *
 * Copyright (C) 2009 Collabora Ltd. <http://www.collabora.co.uk/>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "telepathyaccount.h"
#include "telepathyprotocol.h"
#include "telepathycontact.h"

#include <kopetemetacontact.h>
#include <kopetecontactlist.h>

#include <kdebug.h>

TelepathyAccount::TelepathyAccount(TelepathyProtocol *protocol, const QString &accountId)
    : Kopete::Account(protocol, accountId)
{
    kDebug(TELEPATHY_DEBUG_AREA);

    setMyself( new TelepathyContact(this, accountId, Kopete::ContactList::self()->myself()) );
}

TelepathyAccount::~TelepathyAccount()
{
    kDebug(TELEPATHY_DEBUG_AREA);
}

void TelepathyAccount::connect (const Kopete::OnlineStatus &initialStatus)
{
    kDebug(TELEPATHY_DEBUG_AREA);
}

void TelepathyAccount::disconnect ()
{
    kDebug(TELEPATHY_DEBUG_AREA);
}

void TelepathyAccount::setOnlineStatus (const Kopete::OnlineStatus &status, const Kopete::StatusMessage &reason, const OnlineStatusOptions& options)
{
    kDebug(TELEPATHY_DEBUG_AREA);
}

void TelepathyAccount::setStatusMessage (const Kopete::StatusMessage &statusMessage)
{
    kDebug(TELEPATHY_DEBUG_AREA);
}

bool TelepathyAccount::createContact( const QString &contactId, Kopete::MetaContact *parentContact )
{
    kDebug(TELEPATHY_DEBUG_AREA);
    return false;
}

