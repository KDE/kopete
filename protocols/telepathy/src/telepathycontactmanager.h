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

#ifndef TELEPATHYCONTACTMANAGER_H_
#define TELEPATHYCONTACTMANAGER_H_

#include <QObject>
#include <QSharedPointer>
#include <QList>

#include <TelepathyQt4/Contact>

namespace Kopete {
    class Group;
    class MetaContact;
}

class TelepathyAccount;
class TelepathyContact;

class TelepathyContactManager : public QObject
{
    Q_OBJECT
public:
    TelepathyContactManager(TelepathyAccount *telepathyAccount);
    virtual ~TelepathyContactManager();

    QSharedPointer<Tp::Contact> addContact(const QString &contactId);
    void removeContact(TelepathyContact *contact);
    void setContactList(QList<QSharedPointer<Tp::Contact> > contactList);
    void loadContacts();

private slots:
    void onConnectionReady(Tp::PendingOperation*);
    void onContactsFetched(Tp::PendingOperation *op);
    void onPresencePublicationRequested(const Tp::Contacts &);
    void onRequestingContactsUpgraded(Tp::PendingOperation *op);
    void onAddedInfoEventActionActivated(uint);
    void onTpGroupAdded(const QString &);
    void onTpGroupRemoved(const QString &);
    void onKGroupAdded(Kopete::Group *);
    void onKGroupRemoved(Kopete::Group *);

private:
    void fetchContactList();
    void askPresenceAuthorization(Kopete::MetaContact *mc, Tp::ContactPtr contact);
    TelepathyContact *createContact(QSharedPointer<Tp::Contact> contact);

    class TelepathyContactManagerPrivate;
    TelepathyContactManagerPrivate *d;

    friend class TelepathyAccount;
};


#endif //TELEPATHYCONTACTMANAGER_H_

