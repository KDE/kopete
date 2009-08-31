/*
 * This file is part of Kopete
 *
 * Copyright (C) 2009 Collabora Ltd. <info@collabora.co.uk>
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

#include <KopeteTelepathy/telepathycontact.h>

#include <KopeteTelepathy/telepathyaccount.h>
#include <KopeteTelepathy/telepathychatsession.h>
#include <KopeteTelepathy/telepathyprotocolinternal.h>

#include <KAction>
#include <KDebug>

#include <kopetechatsession.h>
#include <kopetechatsessionmanager.h>
#include <kopetemetacontact.h>

#include <TelepathyQt4/Contact>

#include <QtCore/QPointer>

class TelepathyContact::TelepathyContactPrivate
{
public:
    TelepathyContactPrivate() {}

    Tp::ContactPtr internalContact;
    QPointer<Kopete::ChatSession> currentChatSession;
};

TelepathyContact::TelepathyContact(TelepathyAccount *account, const QString &contactId,
                                   Kopete::MetaContact *parent)
        : Kopete::Contact(account, contactId, parent), d(new TelepathyContactPrivate)
{
    kDebug();
    setOnlineStatus(TelepathyProtocolInternal::protocolInternal()->Offline);
}

TelepathyContact::~TelepathyContact()
{
    delete d;
}

bool TelepathyContact::isReachable()
{
    bool ret = account()->isConnected() && (!internalContact().isNull());
    kDebug() << ret;

    return account()->isConnected() && (!internalContact().isNull());
}

void TelepathyContact::serialize(QMap< QString, QString >& serializedData,
                                 QMap< QString, QString >& addressBookData)
{
    //kDebug();

    Q_UNUSED(serializedData);
    Q_UNUSED(addressBookData);
}

QList<KAction *> *TelepathyContact::customContextMenuActions()
{
    kDebug();
    return new QList<KAction*>();
}

QList<KAction *> *TelepathyContact::customContextMenuActions(Kopete::ChatSession *manager)
{
    kDebug();

    Q_UNUSED(manager);

    return new QList<KAction*>();
}

Kopete::ChatSession *TelepathyContact::manager(CanCreateFlags canCreate)
{
    kDebug();

    Kopete::ContactPtrList members;
    members << this;

    return manager(members, canCreate);
}

Kopete::ChatSession *TelepathyContact::manager(Kopete::ContactPtrList members, CanCreateFlags canCreate)
{
    kDebug();

    if (d->currentChatSession.isNull()) {
        kDebug();

        // Try to find existing chat session
        Kopete::ChatSession *existingSession =
                Kopete::ChatSessionManager::self()->findChatSession(account()->myself(),
                                                                    members,
                                                                    account()->protocol());
        if (existingSession) {
            kDebug() << "chat exist";
            d->currentChatSession = existingSession;
        } else if (canCreate == Kopete::Contact::CanCreate) {
            kDebug() << "chat not exist";
            TelepathyChatSession *newSession = new TelepathyChatSession(account()->myself(),
                                                                        members,
                                                                        account()->protocol());
            newSession->createTextChannel(internalContact());
            d->currentChatSession = newSession;
        }
    }

    return d->currentChatSession;
}

void TelepathyContact::setInternalContact(Tp::ContactPtr contact)
{
    kDebug();

    d->internalContact = contact;

    setOnlineStatus(TelepathyProtocolInternal::protocolInternal()->telepathyStatusToKopete(
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
    kDebug() << alias;
    setNickName(alias);
}

void TelepathyContact::onAvatarTokenChanged(const QString &avatarToken)
{
    kDebug() << avatarToken;

    // TODO: Implement me!
}

void TelepathyContact::onSimplePresenceChanged(const QString &status, uint type, const QString &presenceMessage)
{
    kDebug() << status << type << presenceMessage;
    setOnlineStatus(TelepathyProtocolInternal::protocolInternal()->telepathyStatusToKopete(
            static_cast<Tp::ConnectionPresenceType>(type)));
    setStatusMessage(Kopete::StatusMessage(presenceMessage));
}

void TelepathyContact::onSubscriptionStateChanged(Tp::Contact::PresenceState state)
{
    kDebug();

    Q_UNUSED(state);

    // TODO: Implement me!

}

void TelepathyContact::onPublishStateChanged(Tp::Contact::PresenceState state)
{
    kDebug();

    Q_UNUSED(state);

    // TODO: Implement me!

}

void TelepathyContact::onBlockStatusChanged(bool blocked)
{
    kDebug() << blocked;

    // TODO: Implement me!

}

Tp::ContactPtr TelepathyContact::internalContact()
{
    return d->internalContact;
}

void TelepathyContact::deleteContact()
{
    kDebug();

    TelepathyAccount *tAccount = qobject_cast<TelepathyAccount*>(account());

    if (!tAccount) {
        kWarning() << "Account is not a TelepathyAccount.";
        return;
    }

    if (d->internalContact) {
        tAccount->deleteContact(d->internalContact);
    }
}


#include "telepathycontact.moc"

