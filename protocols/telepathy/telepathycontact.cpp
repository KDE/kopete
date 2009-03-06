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

#include "telepathycontact.h"

#include <kaction.h>
#include <kdebug.h>

#include <kopetemetacontact.h>
#include <kopetechatsession.h>

#include "telepathyaccount.h"
#include "telepathyprotocol.h"

#include <TelepathyQt4/Client/Contact>

#include <QPointer>
#include <QSharedPointer>

class TelepathyContact::TelepathyContactPrivate
{
public:
	QSharedPointer<Telepathy::Client::Contact> internalContact;
	QPointer<Kopete::ChatSession> currentChatSession;
};

TelepathyContact::TelepathyContact(TelepathyAccount *account, const QString &contactId, Kopete::MetaContact *parent)
    : Kopete::Contact(account, contactId, parent), d(new TelepathyContactPrivate)
{
    kDebug(TELEPATHY_DEBUG_AREA);
	setOnlineStatus(TelepathyProtocol::protocol()->Offline);
}

TelepathyContact::~TelepathyContact()
{
    kDebug(TELEPATHY_DEBUG_AREA);
	delete d;
}

bool TelepathyContact::isReachable()
{
    kDebug(TELEPATHY_DEBUG_AREA);
    return account()->isConnected();
}

void TelepathyContact::serialize(QMap< QString, QString >& serializedData, QMap< QString, QString >& addressBookData)
{
    kDebug(TELEPATHY_DEBUG_AREA);
	
	Q_UNUSED(serializedData);
	Q_UNUSED(addressBookData);
}

QList<KAction *> *TelepathyContact::customContextMenuActions()
{
    kDebug(TELEPATHY_DEBUG_AREA);
    return new QList<KAction*>();
}

QList<KAction *> *TelepathyContact::customContextMenuActions( Kopete::ChatSession *manager )
{
    kDebug(TELEPATHY_DEBUG_AREA);
	
	Q_UNUSED(manager);

    return new QList<KAction*>();
}

Kopete::ChatSession *TelepathyContact::manager( CanCreateFlags canCreate )
{
    kDebug(TELEPATHY_DEBUG_AREA);
	
	Q_UNUSED(canCreate);
	
    return 0;
}

void TelepathyContact::setInternalContact(QSharedPointer<Telepathy::Client::Contact> contact)
{
	d->internalContact = contact;
}



