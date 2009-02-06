/*
 * telepathyaccount.cpp
 *
 * Copyright (c) 2009 by Dariusz Mikulski <dariusz.mikulski@gmail.com>
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

// Local includes
#include "telepathyprotocol.h"
#include "telepathycontact.h"
#include "telepathycontactmanager.h"
#include "telepathychatsession.h"
#include "common.h"

// Kopete includes
#include <kopetemetacontact.h>
#include <kopeteonlinestatus.h>
#include <kopetecontactlist.h>
#include <kopetechatsessionmanager.h>
#include <kopeteuiglobal.h>
#include <avatardialog.h>

#include <QtTapioca/TextChannel>

class TelepathyAccount::Private
{
public:
    void initTelepathyAccount();
};

TelepathyAccount::TelepathyAccount(TelepathyProtocol *protocol, const QString &accountId)
    : Kopete::Account(protocol, accountId.toLower()), d(new Private)
{
    Telepathy::registerTypes();
    kDebug(TELEPATHY_DEBUG_AREA);
    setMyself( new TelepathyContact(this, accountId, Kopete::ContactList::self()->myself()) );
}

TelepathyAccount::~TelepathyAccount()
{
    kDebug(TELEPATHY_DEBUG_AREA);
    delete d;
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

bool TelepathyAccount::createContact (const QString &contactId, Kopete::MetaContact *parentContact)
{
    kDebug(TELEPATHY_DEBUG_AREA);
    return false;
}

QtTapioca::TextChannel *TelepathyAccount::createTextChannel(QtTapioca::Contact *internalContact)
{
    kDebug(TELEPATHY_DEBUG_AREA);
    return NULL;
}

QString TelepathyAccount::connectionManager()
{
    kDebug(TELEPATHY_DEBUG_AREA);
    return QString();
}

QString TelepathyAccount::connectionProtocol()
{
    kDebug(TELEPATHY_DEBUG_AREA);
    return QString();
}

bool TelepathyAccount::readConfig()
{
    kDebug(TELEPATHY_DEBUG_AREA);
    return false;
}

Telepathy::Client::ProtocolParameterList TelepathyAccount::allConnectionParameters()
{
    kDebug(TELEPATHY_DEBUG_AREA);
    return Telepathy::Client::ProtocolParameterList();
}


