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
#include "common.h"

#include <kdebug.h>

#include <TelepathyQt4/Client/Contact>
#include <TelepathyQt4/Client/ContactManager>
#include <TelepathyQt4/Client/Account>
#include <TelepathyQt4/Client/PendingContacts>
#include <TelepathyQt4/Client/PendingReady>


TelepathyContactManager::TelepathyContactManager(TelepathyAccount *telepathyAccount, 
												 QSharedPointer<Telepathy::Client::Account> account)
{
	Q_UNUSED(telepathyAccount);
	m_account = account;
}

TelepathyContactManager::~TelepathyContactManager()
{
}
	
QSharedPointer<Telepathy::Client::Contact> TelepathyContactManager::addContact(const QString &contactId)
{
	Q_UNUSED(contactId);
	return QSharedPointer<Telepathy::Client::Contact>();
}

void TelepathyContactManager::removeContact(QSharedPointer<Telepathy::Client::Contact> contact)
{
	Q_UNUSED(contact);
}

void TelepathyContactManager::setContactList(QList<QSharedPointer<Telepathy::Client::Contact> > contactList)
{
	Q_UNUSED(contactList);
}

void TelepathyContactManager::loadContacts()
{
}

void TelepathyContactManager::fetchContactList()
{
	kDebug(TELEPATHY_DEBUG_AREA);
	if(!m_account || !m_account->haveConnection())
	{
		kDebug(TELEPATHY_DEBUG_AREA) << "Error: Could not find active connection or account";
	}
	
	m_contactManager = m_account->connection()->contactManager();

	m_connection = m_contactManager->connection();

	QObject::connect(m_connection->becomeReady(),
		SIGNAL(finished(Telepathy::Client::PendingOperation*)),
	    this,
		SLOT(onConnectionReady(Telepathy::Client::PendingOperation*))
	);
}

void TelepathyContactManager::onConnectionReady(Telepathy::Client::PendingOperation* operation)
{
    kDebug(TELEPATHY_DEBUG_AREA);
    
    if(TelepathyCommons::isOperationError(operation))
        return;

	QSet<uint> features = m_connection->requestedFeatures();
	features << Telepathy::Client::Connection::FeatureRoster;
	QObject::connect(m_connection->requestConnect(features),
		SIGNAL(finished(Telepathy::Client::PendingOperation*)),
		this,
		SLOT(onRequestConnect(Telepathy::Client::PendingOperation*)));
}

void TelepathyContactManager::onRequestConnect(Telepathy::Client::PendingOperation *operation)
{
    kDebug(TELEPATHY_DEBUG_AREA);
    
    if(TelepathyCommons::isOperationError(operation))
        return;

	QSet<QSharedPointer<Telepathy::Client::Contact> > contacts = m_contactManager->allKnownContacts();
	
	foreach(QSharedPointer<Telepathy::Client::Contact> contact, contacts)
	{
		kDebug(TELEPATHY_DEBUG_AREA) << contact->id() << contact->alias();
	}	
}

void TelepathyContactManager::onPendingContacts(Telepathy::Client::PendingOperation* operation)
{
    kDebug(TELEPATHY_DEBUG_AREA);
    
    if(TelepathyCommons::isOperationError(operation))
        return;
	
	Telepathy::Client::PendingContacts *pc = qobject_cast<Telepathy::Client::PendingContacts*>(operation);
	
	if(!pc)
	{
		kDebug(TELEPATHY_DEBUG_AREA) << "Error: Couldn't cast PendingContacts!";
		return;
	}
	
	kDebug(TELEPATHY_DEBUG_AREA) << "PendingContact handles count:" << pc->handles().size() << pc->invalidHandles().size() << pc->identifiers();
	
	QList<QSharedPointer<Telepathy::Client::Contact> > contacts = pc->contacts();
	
	foreach(QSharedPointer<Telepathy::Client::Contact> contact, contacts)
	{
		kDebug(TELEPATHY_DEBUG_AREA) << "Contact id:" << contact->id();
	}
	
	QList<QSharedPointer<Telepathy::Client::Contact> > contactsToUpgrade = pc->contacts();
	
	foreach(QSharedPointer<Telepathy::Client::Contact> contact, contactsToUpgrade)
	{
		kDebug(TELEPATHY_DEBUG_AREA) << "Contact id:" << contact->id();
	}

}














