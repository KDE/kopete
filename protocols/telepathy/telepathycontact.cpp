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
#include "telepathychatsession.h"

#include <kaction.h>
#include <kdebug.h>

#include <kopetemetacontact.h>
#include <kopetechatsession.h>
#include <kopetechatsessionmanager.h>

#include "telepathyaccount.h"
#include "telepathyprotocol.h"

#include <TelepathyQt4/Contact>

#include <QPointer>
#include <QSharedPointer>

class TelepathyContact::TelepathyContactPrivate
{
public:
    TelepathyContactPrivate() {}

    QSharedPointer<Tp::Contact> internalContact;
    QPointer<Kopete::ChatSession> currentChatSession;
};

TelepathyContact::TelepathyContact(TelepathyAccount *account, const QString &contactId,
                                   Kopete::MetaContact *parent)
        : Kopete::Contact(account, contactId, parent), d(new TelepathyContactPrivate)
{
    kDebug(TELEPATHY_DEBUG_AREA);
    setOnlineStatus(TelepathyProtocol::protocol()->Offline);
}

TelepathyContact::~TelepathyContact()
{
    delete d;
}

bool TelepathyContact::isReachable()
{
    //kDebug(TELEPATHY_DEBUG_AREA);
    return account()->isConnected();
}

void TelepathyContact::serialize(QMap< QString, QString >& serializedData,
                                 QMap< QString, QString >& addressBookData)
{
    //kDebug(TELEPATHY_DEBUG_AREA);

    Q_UNUSED(serializedData);
    Q_UNUSED(addressBookData);
}

QList<KAction *> *TelepathyContact::customContextMenuActions()
{
    kDebug(TELEPATHY_DEBUG_AREA);
    return new QList<KAction*>();
}

QList<KAction *> *TelepathyContact::customContextMenuActions(Kopete::ChatSession *manager)
{
    kDebug(TELEPATHY_DEBUG_AREA);

    Q_UNUSED(manager);

    return new QList<KAction*>();
}

Kopete::ChatSession *TelepathyContact::manager(CanCreateFlags canCreate)
{
    kDebug(TELEPATHY_DEBUG_AREA);

    Q_UNUSED(canCreate);

    if (d->currentChatSession.isNull()) {
        kDebug(TELEPATHY_DEBUG_AREA);
        QList<Kopete::Contact*> others;
        others.append(this);

        // \brief: try to find existing chat session
        Kopete::ChatSession *existingSession =
                Kopete::ChatSessionManager::self()->findChatSession(account()->myself(),
                                                                    others,
                                                                    account()->protocol());
        if (existingSession) {
            kDebug(TELEPATHY_DEBUG_AREA) << "chat exist";
            d->currentChatSession = existingSession;
        // See API-Docs for libkopete. If no message manager exists, we must create one anyway.
        // } else if (canCreate == Kopete::Contact::CanCreate) {
        } else {
            kDebug(TELEPATHY_DEBUG_AREA) << "chat not exist";
            TelepathyChatSession *newSession = new TelepathyChatSession(account()->myself(),
                                                                        others,
                                                                        account()->protocol());
            newSession->createTextChannel(internalContact());
            d->currentChatSession = newSession;
        }
    }

    Q_ASSERT(d->currentChatSession != 0);
    return d->currentChatSession;
}

void TelepathyContact::setInternalContact(QSharedPointer<Tp::Contact> contact)
{
    kDebug(TELEPATHY_DEBUG_AREA);

    d->internalContact = contact;

    setOnlineStatus(TelepathyProtocol::protocol()->telepathyStatusToKopete(
            static_cast<Tp::ConnectionPresenceType>(contact->presenceType())));
    setNickName(contact->alias());
    setStatusMessage(contact->presenceMessage());

    connect(contact.data(),
            SIGNAL(aliasChanged(const QString &)),
            SLOT(onAliasChanged(const QString &)));
    connect(contact.data(),
            SIGNAL(avatarTokenChanged(const QString &)),
            SLOT(onAvatarTokenChanged(const QString &)));
    connect(contact.data(),
            SIGNAL(simplePresenceChanged(const QString &, uint, const QString &)),
            SLOT(onSimplePresenceChanged(const QString &, uint, const QString &)));
    connect(contact.data(),
            SIGNAL(subscriptionStateChanged(Tp::Contact::PresenceState)),
            SLOT(onSubscriptionStateChanged(Tp::Contact::PresenceState)));
    connect(contact.data(),
            SIGNAL(publishStateChanged(Tp::Contact::PresenceState)),
            SLOT(onPublishStateChanged(Tp::Contact::PresenceState)));
    connect(contact.data(),
            SIGNAL(blockStatusChanged(bool)),
            SLOT(onBlockStatusChanged(bool)));
}

void TelepathyContact::onAliasChanged(const QString &alias)
{
    kDebug(TELEPATHY_DEBUG_AREA) << alias;
    setNickName(alias);
}

void TelepathyContact::onAvatarTokenChanged(const QString &avatarToken)
{
    kDebug(TELEPATHY_DEBUG_AREA) << avatarToken;

    // TODO: Implement me!
}

void TelepathyContact::onSimplePresenceChanged(const QString &status, uint type, const QString &presenceMessage)
{
    kDebug(TELEPATHY_DEBUG_AREA) << status << type << presenceMessage;
    setOnlineStatus(TelepathyProtocol::protocol()->telepathyStatusToKopete(
            static_cast<Tp::ConnectionPresenceType>(type)));
    setStatusMessage(Kopete::StatusMessage(presenceMessage));
}

void TelepathyContact::onSubscriptionStateChanged(Tp::Contact::PresenceState state)
{
    kDebug(TELEPATHY_DEBUG_AREA);

    Q_UNUSED(state);

    // TODO: Implement me!

}

void TelepathyContact::onPublishStateChanged(Tp::Contact::PresenceState state)
{
    kDebug(TELEPATHY_DEBUG_AREA);

    Q_UNUSED(state);

    // TODO: Implement me!

}

void TelepathyContact::onBlockStatusChanged(bool blocked)
{
    kDebug(TELEPATHY_DEBUG_AREA) << blocked;

    // TODO: Implement me!

}

QSharedPointer<Tp::Contact> TelepathyContact::internalContact()
{
    return d->internalContact;
}


#include "telepathycontact.moc"

