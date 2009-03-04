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

#include "telepathyaccount.h"

#include <TelepathyQt4/Client/Contact>

#include <QObject>
#include <QSharedPointer>
#include <QList>
 
class TelepathyContactManager : public QObject
{
public:
	TelepathyContactManager(TelepathyAccount *account);
	~TelepathyContactManager();
	
	QSharedPointer<Telepathy::Client::Contact> addContact(const QString &contactId);
	void removeContact(QSharedPointer<Telepathy::Client::Contact> contact);
	void setContactList(QList<QSharedPointer<Telepathy::Client::Contact> > contactList);
	void loadContacts(); 
};

#endif //TELEPATHYCONTACTMANAGER_H_
