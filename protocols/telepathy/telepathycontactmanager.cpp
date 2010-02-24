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

    QList<QPointer<TelepathyContact> > contactList;
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

    foreach(QPointer<TelepathyContact> contact, d->contactList) {
        if (!contact)
            continue;

        Kopete::MetaContact *metaContact = contact->metaContact();
        Kopete::ContactList::self()->removeMetaContact(metaContact);
        contact->deleteLater();
    }

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

    QSet<Tp::ContactPtr> contacts = d->connection->contactManager()->allKnownContacts();

    QSet<Tp::Contact::Feature> features;
    features << Tp::Contact::FeatureAlias
             << Tp::Contact::FeatureAvatarToken
             << Tp::Contact::FeatureSimplePresence;

    QObject::connect(d->connection->contactManager()->upgradeContacts(contacts.toList(), features),
                     SIGNAL(finished(Tp::PendingOperation*)),
                     SLOT(onContactsUpgraded(Tp::PendingOperation*)));
}

void TelepathyContactManager::onContactsUpgraded(Tp::PendingOperation *op)
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

    d->contacts = pendingContacts->contacts();

    Tp::UIntList requestAvatarList;

    foreach(Tp::ContactPtr contact, pendingContacts->contacts()) {
        if ((contact->publishState() == Tp::Contact::PresenceStateYes) ||
            (contact->subscriptionState() == Tp::Contact::PresenceStateYes) ) {

            TelepathyContact *tpc = createContact(contact);

            if (tpc)
                if (contact->isAvatarTokenKnown() &&
                        (tpc->storedAvatarToken() != contact->avatarToken() ||
                        QFile::exists(tpc->storedAvatarPath()) == false)) {
                    requestAvatarList.append(contact->handle()[0]);
                }
        }

        connect(contact.data(),
                SIGNAL(subscriptionStateChanged(Tp::Contact::PresenceState)),
                SLOT(onContactSubscriptionStateChanged(Tp::Contact::PresenceState)));
        connect(contact.data(),
                SIGNAL(publishStateChanged(Tp::Contact::PresenceState)),
                SLOT(onContactPublishStateChanged(Tp::Contact::PresenceState)));
        connect(contact.data(),
                SIGNAL(blockStatusChanged(bool)),
                SLOT(onContactBlockStatusChanged(bool)));
    }

    if (!requestAvatarList.isEmpty()) {
        Tp::Client::ConnectionInterfaceAvatarsInterface *avatarIface =
                        d->connection->avatarsInterface();

        if (avatarIface)
            avatarIface->RequestAvatars(requestAvatarList);
    }
}

void TelepathyContactManager::onContactSubscriptionStateChanged(Tp::Contact::PresenceState state)
{
    kDebug();

    // TODO: Implement me!
}

void TelepathyContactManager::onContactPublishStateChanged(Tp::Contact::PresenceState state)
{
    kDebug();

    // TODO: Implement me!
}

void TelepathyContactManager::onContactBlockStatusChanged(bool blocked)
{
    kDebug();

    // Get the callee Tp::Contact.
    Tp::Contact *pContact = qobject_cast<Tp::Contact*>(sender());

    if (!pContact) {
        kWarning() << "Slot called by non-Tp::Contact object.";
        return;
    }

    Tp::ContactPtr contact = Tp::ContactPtr(pContact);

    // TODO: Implement me!
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

        Kopete::AddedInfoEvent::ShowActionOptions actions = Kopete::AddedInfoEvent::AuthorizeAction;
     //   actions |= Kopete::AddedInfoEvent::BlockAction;
        // FIXME: Add blocking support, then add the block action above.

        if (!kMetaContact || kMetaContact->isTemporary()) {
            actions |= Kopete::AddedInfoEvent::AddAction;
        }

        TelepathyAddedInfoEvent* event = new TelepathyAddedInfoEvent(contact, d->telepathyAccount);

        connect(event,
                SIGNAL(actionActivated(uint)),
                SLOT(onAddedInfoEventActionActivated(uint)));

        event->showActions(actions);
        event->sendEvent();
    }
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
        event->addContact();
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

    // Only create this contact if it isn't already in the list.
    foreach (Kopete::MetaContact *mc, Kopete::ContactList::self()->metaContacts()) {
        foreach (Kopete::Contact *c, mc->contacts()) {
            // FIXME: Comparing the string ids is WRONG! One has to compare handles, one way or
            // another.
            if((c->account() == d->telepathyAccount) &&
               (c->contactId() == contact->id()))
            {
                // Contact is already in the contact list. Check if it has a internalContact, and
                // if it doesn't (or if the current one is invalid), add one
                // (this is the case if it is one that was deserialized from contactlist.xml file),
                // or we are reconnecting.
                kDebug() << "Contact is already in list. Don't add it.";

                TelepathyContact *tpc = qobject_cast<TelepathyContact*>(c);
                if (!c) {
                    kDebug() << "Contact is not of type TelepathyContact.";
                    return NULL;
                }

                if (tpc->internalContact().isNull() ||
                    !tpc->internalContact()->manager()->connection()->isValid()) {
                    tpc->setInternalContact(contact);
                }

                return tpc;
            }
        }
    }

    Kopete::MetaContact *metaContact = new Kopete::MetaContact();
    QPointer<TelepathyContact> newContact = new TelepathyContact(d->telepathyAccount, contact->id(), metaContact);
    newContact->setInternalContact(contact);
    newContact->setMetaContact(metaContact);

    Kopete::ContactList::self()->addMetaContact(metaContact);

    d->contactList.push_back(newContact);

    return newContact;
}


#include "telepathycontactmanager.moc"

