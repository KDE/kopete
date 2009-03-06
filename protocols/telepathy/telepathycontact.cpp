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
    kDebug(TELEPATHY_DEBUG_AREA);

	d->internalContact = contact;
	
	setOnlineStatus(TelepathyProtocol::protocol()->telepathyStatusToKopete(static_cast<Telepathy::ConnectionPresenceType>(contact->presenceType())));

	QObject::connect(contact.data(),
		SIGNAL(aliasChanged (const QString &)),
		this,
		SLOT(onAliasChanged (const QString &)));
	QObject::connect(contact.data(),
		SIGNAL(avatarTokenChanged (const QString &)),
		this,
		SLOT(onAvatarTokenChanged (const QString &)));
	QObject::connect(contact.data(),
		SIGNAL(simplePresenceChanged (const QString &, uint, const QString &)),
		this,
		SLOT(onSimplePresenceChanged (const QString &, uint, const QString &)));
	QObject::connect(contact.data(),
		SIGNAL(subscriptionStateChanged (Telepathy::Client::Contact::PresenceState)),
		this,
		SLOT(onsubscriptionStateChanged (Telepathy::Client::Contact::PresenceState)));
	QObject::connect(contact.data(),
		SIGNAL(publishStateChanged (Telepathy::Client::Contact::PresenceState)),
		this,
		SLOT(onpublishStateChanged (Telepathy::Client::Contact::PresenceState)));
	QObject::connect(contact.data(),
		SIGNAL(blockStatusChanged (bool)),
		this,
		SLOT(onblockStatusChanged (bool)));
}

void TelepathyContact::onAliasChanged (const QString &avatar)
{
    kDebug(TELEPATHY_DEBUG_AREA) << avatar;
}

void TelepathyContact::onAvatarTokenChanged (const QString &avatarToken)
{
    kDebug(TELEPATHY_DEBUG_AREA) << avatarToken;
}

void TelepathyContact::onSimplePresenceChanged (const QString &status, uint type, const QString &presenceMessage)
{
    kDebug(TELEPATHY_DEBUG_AREA) << status << type << presenceMessage;
	setOnlineStatus(TelepathyProtocol::protocol()->telepathyStatusToKopete(static_cast<Telepathy::ConnectionPresenceType>(type)));
}

void TelepathyContact::onSubscriptionStateChanged (Telepathy::Client::Contact::PresenceState state)
{
    kDebug(TELEPATHY_DEBUG_AREA);
	
	Q_UNUSED(state);
}

void TelepathyContact::onPublishStateChanged (Telepathy::Client::Contact::PresenceState state)
{
    kDebug(TELEPATHY_DEBUG_AREA);

	Q_UNUSED(state);
}

void TelepathyContact::onBlockStatusChanged (bool blocked)
{
    kDebug(TELEPATHY_DEBUG_AREA) << blocked;
}


