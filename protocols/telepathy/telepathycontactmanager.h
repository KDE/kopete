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

#include <TelepathyQt4/Client/Contact>

class TelepathyAccount;
class TelepathyContact;
 
class TelepathyContactManager : public QObject
{
	Q_OBJECT
public:
	TelepathyContactManager(TelepathyAccount *telepathyAccount);
	~TelepathyContactManager();
	
	QSharedPointer<Telepathy::Client::Contact> addContact(const QString &contactId);
	void removeContact(TelepathyContact *contact);
	void setContactList(QList<QSharedPointer<Telepathy::Client::Contact> > contactList);
	void loadContacts();
	
private slots: 
	void onConnectionReady(Telepathy::Client::PendingOperation*);
	void onConnectionFeaturesReady(Telepathy::Client::PendingOperation*);
	void onPresencePublicationRequested(const Telepathy::Client::Contacts &);
	
private:
	void fetchContactList();
	void createContact(QSharedPointer<Telepathy::Client::Contact> contact);
	
	class TelepathyContactManagerPrivate;
	TelepathyContactManagerPrivate *d;
	
	friend class TelepathyAccount;
};

#endif //TELEPATHYCONTACTMANAGER_H_
