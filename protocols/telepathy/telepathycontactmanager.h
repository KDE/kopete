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

#ifndef TELEPATHYCONTACTMANAGER_H_
#define TELEPATHYCONTACTMANAGER_H_

#include <QObject>
#include <QSharedPointer>
#include <QList>

#include <TelepathyQt4/Contact>

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
    void onConnectionFeaturesReady(Tp::PendingOperation*);
    void onPresencePublicationRequested(const Tp::Contacts &);

private:
    void fetchContactList();
    void createContact(QSharedPointer<Tp::Contact> contact);

    class TelepathyContactManagerPrivate;
    TelepathyContactManagerPrivate *d;

    friend class TelepathyAccount;
};

#endif //TELEPATHYCONTACTMANAGER_H_
