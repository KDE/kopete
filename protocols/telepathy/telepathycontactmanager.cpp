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

#include "telepathyprotocol.h"
#include "telepathycontactmanager.h"
#include "telepathyaccount.h"
#include "telepathycontact.h"
#include "common.h"

#include <kdebug.h>

#include <kopetemetacontact.h>
#include <kopetecontactlist.h>

#include <TelepathyQt4/Connection>
#include <TelepathyQt4/ContactManager>
#include <TelepathyQt4/Account>
#include <TelepathyQt4/PendingContacts>
#include <TelepathyQt4/PendingReady>

class TelepathyContactManager::TelepathyContactManagerPrivate
{
public:
    TelepathyAccount *telepathyAccount;
    Tp::AccountPtr account;
    Tp::ConnectionPtr connection;

    QList<TelepathyContact*> contactList;
};

TelepathyContactManager::TelepathyContactManager(TelepathyAccount *telepathyAccount)
        : d(new TelepathyContactManagerPrivate)
{
    kDebug(TELEPATHY_DEBUG_AREA);

    d->telepathyAccount = telepathyAccount;
    d->account = d->telepathyAccount->m_account;
}

TelepathyContactManager::~TelepathyContactManager()
{
    kDebug(TELEPATHY_DEBUG_AREA);

    foreach(TelepathyContact *contact, d->contactList) {
        Kopete::MetaContact *metaContact = contact->metaContact();
        Kopete::ContactList::self()->removeMetaContact(metaContact);
        delete contact;
    }

    delete d;
}

QSharedPointer<Tp::Contact> TelepathyContactManager::addContact(const QString &contactId)
{
    kDebug(TELEPATHY_DEBUG_AREA);

    Q_UNUSED(contactId);
    return QSharedPointer<Tp::Contact>();
}

void TelepathyContactManager::removeContact(TelepathyContact *contact)
{
    kDebug(TELEPATHY_DEBUG_AREA);

    if (contact->internalContact()) {

    }
    contact->deleteLater();
}

void TelepathyContactManager::setContactList(QList<QSharedPointer<Tp::Contact> > contactList)
{
    kDebug(TELEPATHY_DEBUG_AREA);

    Q_UNUSED(contactList);
}

void TelepathyContactManager::loadContacts()
{
    kDebug(TELEPATHY_DEBUG_AREA);
}

void TelepathyContactManager::fetchContactList()
{
    kDebug(TELEPATHY_DEBUG_AREA);

    if (!d->account || !d->account->haveConnection()) {
        kWarning(TELEPATHY_DEBUG_AREA) << "Error: Could not find active connection or account";
        return;
    }

    d->connection = d->account->connection();

    Tp::Features features;
    features << Tp::Connection::FeatureCore
             << Tp::Connection::FeatureRoster;

    connect(d->connection->becomeReady(features),
            SIGNAL(finished(Tp::PendingOperation*)),
            SLOT(onConnectionReady(Tp::PendingOperation*)));
}

void TelepathyContactManager::onConnectionReady(Tp::PendingOperation *operation)
{
    kDebug(TELEPATHY_DEBUG_AREA);

    if (TelepathyCommons::isOperationError(operation))
        return;

    QObject::connect(d->connection->contactManager(),
                     SIGNAL(presencePublicationRequested(const Tp::Contacts &)),
                     SLOT(onPresencePublicationRequested(const Tp::Contacts &)));

    QSet<QSharedPointer<Tp::Contact> > contacts = d->connection->contactManager()->allKnownContacts();

    foreach(QSharedPointer<Tp::Contact> contact, contacts) {
        if ((contact->publishState() == Tp::Contact::PresenceStateYes) ||
            (contact->subscriptionState() == Tp::Contact::PresenceStateYes) ) {
            createContact(contact);
        }
    }
}

void TelepathyContactManager::onPresencePublicationRequested(const Tp::Contacts &contacts)
{
    kDebug(TELEPATHY_DEBUG_AREA);

    foreach(QSharedPointer<Tp::Contact> contact, contacts) {
        createContact(contact);
    }
}

void TelepathyContactManager::createContact(QSharedPointer<Tp::Contact> contact)
{
    kDebug(TELEPATHY_DEBUG_AREA) << contact->id() << contact->alias();
    kDebug(TELEPATHY_DEBUG_AREA) << "Subscription status:" << contact->subscriptionState();
    kDebug(TELEPATHY_DEBUG_AREA) << "Publish status:" << contact->publishState();

    // Only create this contact if it isn't already in the list.
    foreach (Kopete::MetaContact *mc, Kopete::ContactList::self()->metaContacts()) {
        foreach (Kopete::Contact *c, mc->contacts()) {
            if((c->account() == d->telepathyAccount) &&
               (c->contactId() == contact->id()))
            {
                // Contact is already in the contact list. Check if it has a internalContact, and
                // if it doesn't, add one (this is the case if it is one that was deserialized
                // from contactlist.xml file).
                kDebug(TELEPATHY_DEBUG_AREA) << "Contact is already in list. Don't add it.";

                TelepathyContact *tpc = qobject_cast<TelepathyContact*>(c);
                if (!c) {
                    kDebug(TELEPATHY_DEBUG_AREA) << "Contact is not of type TelepathyContact.";
                    return;
                }

                if (tpc->internalContact().isNull()) {
                    tpc->setInternalContact(contact);
                }

                return;
            }
        }
    }

    Kopete::MetaContact *metaContact = new Kopete::MetaContact();
    TelepathyContact *newContact = new TelepathyContact(d->telepathyAccount, contact->id(), metaContact);
    newContact->setInternalContact(contact);
    newContact->setMetaContact(metaContact);

    Kopete::ContactList::self()->addMetaContact(metaContact);

    d->contactList.push_back(newContact);
}

