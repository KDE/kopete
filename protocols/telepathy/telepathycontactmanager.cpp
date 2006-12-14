/*
 * telepathycontactmanager.cpp - Telepathy Contact Manager
 *
 * Copyright (c) 2006 by Michaël Larouche <larouche@kde.org>
 * 
 * Kopete    (c) 2002-2006 by the Kopete developers  <kopete-devel@kde.org>
 *
 *************************************************************************
 *                                                                       *
 * This program is free software; you can redistribute it and/or modify  *
 * it under the terms of the GNU General Public License as published by  *
 * the Free Software Foundation; either version 2 of the License, or     *
 * (at your option) any later version.                                   *
 *                                                                       *
 *************************************************************************
 */
#include "telepathycontactmanager.h"

// Qt includes
#include <QtCore/QPointer>

// KDE includes
#include <kdebug.h>
#include <klocale.h>

// QtTapioca includes
#include <QtTapioca/ContactList>
#include <QtTapioca/Contact>
#include <QtTapioca/ContactBase>

// Kopete includes
#include <kopetemetacontact.h>
#include <kopeteonlinestatus.h>
#include <kopetecontactlist.h>

// Local includes
#include "telepathyaccount.h"
#include "telepathycontact.h"
#include "telepathyprotocol.h"

using namespace QtTapioca;

class TelepathyContactManager::Private
{
public:
	Private()
	{}
	
	QPointer<TelepathyAccount> account;
	QPointer<ContactList> contactList;
};

TelepathyContactManager::TelepathyContactManager(TelepathyAccount *account)
 : QObject(account), d(new Private)
{
	d->account = account;
}

TelepathyContactManager::~TelepathyContactManager()
{
	delete d;
}

TelepathyAccount *TelepathyContactManager::account()
{
	Q_ASSERT_X( !d->account.isNull(), "TelepathyContactManager::account", "account is null" );

	return d->account;
}

QtTapioca::ContactList *TelepathyContactManager::contactList()
{
	Q_ASSERT_X( !d->contactList.isNull(), "TelepathyContactManager::contactList", "contactList is null" );
	return d->contactList;
}

QtTapioca::Contact *TelepathyContactManager::addContact(const QString &contactId)
{
	return const_cast<QtTapioca::Contact*>( contactList()->addContact(contactId) );
}

void TelepathyContactManager::removeContact(TelepathyContact *contact)
{
	kDebug(TELEPATHY_DEBUG_AREA) << k_funcinfo << endl;

	if( contact->internalContact() )
	{
		// First remove the contact from the contact list
		contactList()->removeContact( contact->internalContact() );
	}
	else
	{
		kDebug(TELEPATHY_DEBUG_AREA) << k_funcinfo << "WARNING: Internal contact in " << contact->contactId() << " is null. Removing in Kopete only." << endl;
	}

	// Then delete the contact from Kopete
	contact->deleteLater();
}

void TelepathyContactManager::setContactList(QtTapioca::ContactList *contactList)
{
	d->contactList = contactList;
}

void TelepathyContactManager::loadContacts()
{
	kDebug(TELEPATHY_DEBUG_AREA) << k_funcinfo << "Loading contact list into Kopete." << endl;

	QList<Contact*> contacts = contactList()->knownContacts();
	Contact *tempContact;
	foreach(tempContact, contacts)
	{
		QString contactId = tempContact->uri();

		// If the contact doesn't exist in Kopete, create it
		if( !account()->contacts()[contactId] )
		{
			createContact(tempContact);
		}
		// else, set the internal telepathy object in the existing contact.
		else
		{
			kDebug(TELEPATHY_DEBUG_AREA) << k_funcinfo << "Set internal information from Telepathy to " << contactId << endl;
			TelepathyContact *contact = static_cast<TelepathyContact*>( account()->contacts()[contactId] );
			contact->setInternalContact(tempContact);
		}
	}
}

void TelepathyContactManager::createContact(QtTapioca::Contact *telepathyContact)
{
	QString contactId = telepathyContact->uri();

	kDebug(TELEPATHY_DEBUG_AREA) << k_funcinfo << "Creating Telepathy contact \"" << contactId << "\" in Kopete." << endl;

	Kopete::MetaContact *metaContact = new Kopete::MetaContact();

	// Create the TelepathyContact
	TelepathyContact *newContact = new TelepathyContact( account(), contactId, metaContact );
	newContact->setInternalContact(telepathyContact);
	newContact->setMetaContact(metaContact);

	Kopete::ContactList::self()->addMetaContact( metaContact );
}
#include "telepathycontactmanager.moc"
