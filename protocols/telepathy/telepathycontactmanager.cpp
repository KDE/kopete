/*
 * This file is part of Kopete
 *
 * Copyright (C) 2009-2010 Collabora Ltd. <info@collabora.co.uk>
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

#include <telepathycontactmanager.h>

#include <telepathyaccount.h>
#include <telepathyaddedinfoevent.h>
#include <telepathycontact.h>
#include <telepathyprotocolinternal.h>

#include <KDebug>

#include <kopetecontactlist.h>
#include <kopetemetacontact.h>
#include <kopetegroup.h>

#include <TelepathyQt4/Connection>
#include <TelepathyQt4/ContactManager>
#include <TelepathyQt4/Account>
#include <TelepathyQt4/Contact>
#include <TelepathyQt4/PendingContacts>
#include <TelepathyQt4/PendingReady>

#include <QPointer>

class TelepathyContactManager::TelepathyContactManagerPrivate
{
public:
    TelepathyAccount *telepathyAccount;
    Tp::AccountPtr account;
    Tp::ConnectionPtr connection;

    QList<Tp::ContactPtr> contacts;
};

TelepathyContactManager::TelepathyContactManager(TelepathyAccount *telepathyAccount)
        : d(new TelepathyContactManagerPrivate)
{
    kDebug();

    d->telepathyAccount = telepathyAccount;
    d->account = d->telepathyAccount->m_account;

    QObject::connect(Kopete::ContactList::self(),
                     SIGNAL(groupAdded(Kopete::Group *)),
                     SLOT(onKGroupAdded(Kopete::Group *)));

    QObject::connect(Kopete::ContactList::self(),
                     SIGNAL(groupRemoved(Kopete::Group *)),
                     SLOT(onKGroupRemoved(Kopete::Group *)));
}

TelepathyContactManager::~TelepathyContactManager()
{
    kDebug();

    delete d;
}

QSharedPointer<Tp::Contact> TelepathyContactManager::addContact(const QString &contactId)
{
    kDebug();

    Q_UNUSED(contactId);
    return QSharedPointer<Tp::Contact>();
}

void TelepathyContactManager::removeContact(TelepathyContact *contact)
{
    kDebug();

    if (contact->internalContact()) {

    }
    contact->deleteLater();
}

void TelepathyContactManager::setContactList(QList<QSharedPointer<Tp::Contact> > contactList)
{
    kDebug();

    Q_UNUSED(contactList);
}

void TelepathyContactManager::loadContacts()
{
    kDebug();
}

void TelepathyContactManager::fetchContactList()
{
    kDebug();

    if (!d->account || !d->account->haveConnection()) {
        kWarning() << "Error: Could not find active connection or account";
        return;
    }

    d->connection = d->account->connection();

    Tp::Features features;
    features << Tp::Connection::FeatureCore
             << Tp::Connection::FeatureRoster
             << Tp::Connection::FeatureRosterGroups;

    connect(d->connection->becomeReady(features),
            SIGNAL(finished(Tp::PendingOperation*)),
            SLOT(onConnectionReady(Tp::PendingOperation*)));
}

void TelepathyContactManager::onConnectionReady(Tp::PendingOperation *operation)
{
    kDebug();

    if (operation->isError()) {
        kWarning() << "Readying connection failed:"
                   << operation->errorName()
                   << operation->errorMessage();
        return;
    }

    QObject::connect(d->connection->contactManager(),
                     SIGNAL(presencePublicationRequested(const Tp::Contacts &)),
                     SLOT(onPresencePublicationRequested(const Tp::Contacts &)));

    QObject::connect(d->connection->contactManager(),
                     SIGNAL(groupAdded(const QString &)),
                     SLOT(onTpGroupAdded(const QString &)));

    QObject::connect(d->connection->contactManager(),
                     SIGNAL(groupRemoved(const QString &)),
                     SLOT(onTpGroupRemoved(const QString &)));

    QSet<QString> ids;

    foreach (Tp::ContactPtr contact, d->connection->contactManager()->allKnownContacts())
        ids.insert(contact->id());

    foreach (Kopete::Contact *contact, d->telepathyAccount->contacts().values())
        ids.insert(contact->contactId());

    QSet<Tp::Contact::Feature> features;
    features << Tp::Contact::FeatureAlias
             << Tp::Contact::FeatureAvatarToken
             << Tp::Contact::FeatureSimplePresence;

    QObject::connect(
            d->connection->contactManager()->contactsForIdentifiers(ids.toList(), features),
            SIGNAL(finished(Tp::PendingOperation*)),
            SLOT(onContactsFetched(Tp::PendingOperation*)));
}

void TelepathyContactManager::onContactsFetched(Tp::PendingOperation *op)
{
    kDebug();

    if (op->isError()) {
        kWarning() << "Fetching contacts failed:" << op->errorName() << op->errorMessage();
        return;
    }

    Tp::PendingContacts *pendingContacts = qobject_cast<Tp::PendingContacts*>(op);

    if (!pendingContacts) {
        kWarning() << "Slot called with incorrect type.";
        return;
    }

    d->contacts = pendingContacts->contacts();

    Tp::UIntList requestAvatarList;

    for (int i = 0; i < d->contacts.length(); i++) {
        Tp::ContactPtr fetchedContact = d->contacts[i];
        QString requestId = pendingContacts->validIdentifiers()[i];
        Kopete::Contact *kopeteContact = d->telepathyAccount->contacts().value(requestId, NULL);
        TelepathyContact *tpContact =
            kopeteContact ? qobject_cast<TelepathyContact *>(kopeteContact) : NULL;

        if (tpContact && tpContact->internalContact() != fetchedContact)
            tpContact->setInternalContact(fetchedContact);

        if ((fetchedContact->publishState() == Tp::Contact::PresenceStateYes) ||
            (fetchedContact->subscriptionState() != Tp::Contact::PresenceStateNo)) {

            if (!tpContact)
                tpContact = createContact(fetchedContact);

            if (fetchedContact->publishState() == Tp::Contact::PresenceStateAsk)
                askPresenceAuthorization(tpContact->metaContact(), fetchedContact);
        } else if (fetchedContact->publishState() == Tp::Contact::PresenceStateAsk) {
            if (tpContact)
                askPresenceAuthorization(tpContact->metaContact(), fetchedContact);
            else
                askPresenceAuthorization(NULL, fetchedContact);
        }

        if (tpContact && fetchedContact->isAvatarTokenKnown() &&
                (tpContact->storedAvatarToken() != fetchedContact->avatarToken() ||
                     !QFile::exists(tpContact->storedAvatarPath()))) {
            requestAvatarList.append(fetchedContact->handle()[0]);
        }
    }

    foreach (QString invalidId, pendingContacts->invalidIdentifiers().keys()) {
        Kopete::Contact *kopeteContact = d->telepathyAccount->contacts().value(invalidId, NULL);

        if (kopeteContact) {
            Kopete::MetaContact *mc = kopeteContact->metaContact();

            kDebug() << "Deleting contact with invalid ID" << invalidId;
            delete kopeteContact;

            if (mc->contacts().isEmpty())
                Kopete::ContactList::self()->removeMetaContact(mc);
        }
    }

    if (!requestAvatarList.isEmpty()) {
        Tp::Client::ConnectionInterfaceAvatarsInterface *avatarIface =
                        d->connection->avatarsInterface();

        if (avatarIface)
            avatarIface->RequestAvatars(requestAvatarList);
    }
}

void TelepathyContactManager::onPresencePublicationRequested(const Tp::Contacts &contacts)
{
    kDebug();

    QSet<Tp::Contact::Feature> features;
    features << Tp::Contact::FeatureAlias
             << Tp::Contact::FeatureAvatarToken
             << Tp::Contact::FeatureSimplePresence;

    QObject::connect(d->connection->contactManager()->upgradeContacts(contacts.toList(), features),
                     SIGNAL(finished(Tp::PendingOperation*)),
                     SLOT(onRequestingContactsUpgraded(Tp::PendingOperation*)));
}

void TelepathyContactManager::onRequestingContactsUpgraded(Tp::PendingOperation *op)
{
    kDebug();

    if (op->isError()) {
        kWarning() << "Upgrading contacts failed:" << op->errorName() << op->errorMessage();
        return;
    }

    Tp::PendingContacts *pendingContacts = qobject_cast<Tp::PendingContacts*>(op);

    if (!pendingContacts) {
        kWarning() << "Slot called with incorrect type.";
        return;
    }

    foreach (Tp::ContactPtr contact, pendingContacts->contacts()) {
        Kopete::Contact *kContact = 0;
        Kopete::MetaContact *kMetaContact = 0;

        // Check if the telepathy contact is already in the list
        foreach (Kopete::MetaContact *mc, Kopete::ContactList::self()->metaContacts()) {
            foreach (Kopete::Contact *c, mc->contacts()) {
                // FIXME: Comparing string ids is WRONG!
                if ((c->account() == d->telepathyAccount) &&
                        (c->contactId() == contact->id())) {

                    // Contact is already in the list.
                    kContact = c;
                    kMetaContact = mc;
                    break;
                }
            }

            if (kContact) {
                break;
            }
        }

        askPresenceAuthorization(kMetaContact, contact);
    }
}

void TelepathyContactManager::askPresenceAuthorization(Kopete::MetaContact *mc, Tp::ContactPtr contact) {
    Kopete::AddedInfoEvent::ShowActionOptions actions = Kopete::AddedInfoEvent::AuthorizeAction;
    actions |= Kopete::AddedInfoEvent::BlockAction;

    if (!mc || mc->isTemporary()) {
        actions |= Kopete::AddedInfoEvent::AddAction;
    }

    TelepathyAddedInfoEvent* event = new TelepathyAddedInfoEvent(contact, d->telepathyAccount);

    connect(event,
            SIGNAL(actionActivated(uint)),
            SLOT(onAddedInfoEventActionActivated(uint)));

    event->showActions(actions);
    event->sendEvent();
}

void TelepathyContactManager::onAddedInfoEventActionActivated(uint actionId)
{
    kDebug();

    TelepathyAddedInfoEvent *event = qobject_cast<TelepathyAddedInfoEvent*>(sender());

    if (!event) {
        kWarning() << "Method not called by a TelepathyAddedInfoEvent. Aborting.";
        return;
    }

    if (actionId == Kopete::AddedInfoEvent::AuthorizeAction) {
        // Authorize the contact to view our presence.
        QList<Tp::ContactPtr> contacts;
        contacts << event->contact();
        d->connection->contactManager()->authorizePresencePublication(contacts);
        // FIXME: Handle the completion of the above Tp::PendingOperation
    } else if (actionId == Kopete::AddedInfoEvent::AddContactAction) {
        // Add the contact
        Kopete::MetaContact *parentContact = event->addContact();
        d->telepathyAccount->addNewContact(event->contactId());

        if (!parentContact)
            return;

        QStringList groupNames;
        foreach (Kopete::Group *group, parentContact->groups())
            groupNames += group->displayName();

        // TODO: Add to those groups too
    } else if (actionId == Kopete::AddedInfoEvent::BlockAction) {
        event->contact()->removePresencePublication();
    } else {
        kWarning() << "Unknown button pressed.";
    }
}

void TelepathyContactManager::onTpGroupAdded(const QString &group)
{
    kDebug() << "New tp group added:" << group;

    Kopete::Group *kgroup = Kopete::ContactList::self()->findGroup(group);

    if (!kgroup) {
        kgroup = new Kopete::Group(group);
        Kopete::ContactList::self()->addGroup(kgroup);
    }
}

void TelepathyContactManager::onTpGroupRemoved(const QString &group)
{
    kDebug() << "tp Group removed:" << group;

    Kopete::Group *kgroup = Kopete::ContactList::self()->findGroup(group);

    if (kgroup)
        Kopete::ContactList::self()->removeGroup(kgroup);
}

void TelepathyContactManager::onKGroupAdded(Kopete::Group *group)
{
    kDebug() << "Kopete group added:" << group;

    if (d->connection->isReady()) {
        Tp::ContactManager *cm = d->connection->contactManager();
        QStringList tpGroupList = cm->allKnownGroups();

        if (!tpGroupList.contains(group->displayName())) {
            kDebug() << "Adding new group to Tp:" << group->displayName();
            cm->addGroup(group->displayName());
        }
    }
}

void TelepathyContactManager::onKGroupRemoved(Kopete::Group *group)
{
    kDebug() << "Kopete group removed:" << group;

    if (d->connection->isReady()) {
        Tp::ContactManager *cm = d->connection->contactManager();
        QStringList tpGroupList = cm->allKnownGroups();

        if (tpGroupList.contains(group->displayName())) {
            kDebug() << "Removing new group to Tp:" << group->displayName();
            cm->removeGroup(group->displayName());
        }
    }
}

TelepathyContact * TelepathyContactManager::createContact(QSharedPointer<Tp::Contact> contact)
{
    kDebug() << contact->id() << contact->alias();
    kDebug() << "Subscription status:" << contact->subscriptionState();
    kDebug() << "Publish status:" << contact->publishState();

    Kopete::MetaContact *metaContact = new Kopete::MetaContact();
    QPointer<TelepathyContact> newContact = new TelepathyContact(d->telepathyAccount, contact->id(), metaContact);
    newContact->setInternalContact(contact);
    newContact->setMetaContact(metaContact);

    Kopete::ContactList::self()->addMetaContact(metaContact);

    return newContact;
}


#include "telepathycontactmanager.moc"

