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

#include "telepathycontactmanager.h"

TelepathyContactManager::TelepathyContactManager(TelepathyAccount *account)
{
}

TelepathyContactManager::~TelepathyContactManager()
{
}
	
QSharedPointer<Telepathy::Client::Contact> TelepathyContactManager::addContact(const QString &contactId)
{
	return QSharedPointer<Telepathy::Client::Contact>();
}

void TelepathyContactManager::removeContact(QSharedPointer<Telepathy::Client::Contact> contact)
{
}

void TelepathyContactManager::setContactList(QList<QSharedPointer<Telepathy::Client::Contact> > contactList)
{
}

void TelepathyContactManager::loadContacts()
{
}


