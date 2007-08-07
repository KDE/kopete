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
#include "telepathyaddpendingcontactjob.h"

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
	kDebug(TELEPATHY_DEBUG_AREA) << k_funcinfo;

	if( contact->internalContact() )
	{
		// First remove the contact from the contact list
		contactList()->removeContact( contact->internalContact() );
	}
	else
	{
		kDebug(TELEPATHY_DEBUG_AREA) << k_funcinfo << "WARNING: Internal contact in " << contact->contactId() << " is null. Removing in Kopete only.";
	}

	// Then delete the contact from Kopete
	contact->deleteLater();
}

void TelepathyContactManager::setContactList(QtTapioca::ContactList *contactList)
{
	// Disconnect signals from previous instance.
	if( !d->contactList.isNull() )
	{
		d->contactList->disconnect();
	}

	d->contactList = contactList;
	
	// Connect signals/slot
	connect(d->contactList, SIGNAL(authorizationRequested(QtTapioca::Contact*)), this, SLOT(telepathyAuthorizationRequired(QtTapioca::Contact*)));
	connect(d->contactList, SIGNAL(subscriptionAccepted(QtTapioca::Contact *)), this, SLOT(telepathySubscriptionAccepted(QtTapioca::Contact*)));
}

void TelepathyContactManager::loadContacts()
{
	kDebug(TELEPATHY_DEBUG_AREA) << k_funcinfo << "Loading contact list into Kopete.";

	QList<Contact*> contacts = contactList()->knownContacts();
	if( contacts.isEmpty() )
	{
		kDebug(TELEPATHY_DEBUG_AREA) << k_funcinfo << "WARNING: Contact list from Telepathy is empty !";
	}

	Contact *tempContact;
	foreach(tempContact, contacts)
	{
		QString contactId = tempContact->uri();

		kDebug(TELEPATHY_DEBUG_AREA) << "Subscription Status(" << contactId << "): " << int(tempContact->subscriptionStatus());
		kDebug(TELEPATHY_DEBUG_AREA) << "Authorization Status(" << contactId << "): " << int(tempContact->authorizationStatus());

		if( tempContact->authorizationStatus() == QtTapioca::Contact::LocalPending )
		{
			kDebug(TELEPATHY_DEBUG_AREA) << k_funcinfo << "Found a local pending contact. Adding it";

			// Add the pending contact
			TelepathyAddPendingContactJob *addPendingContactJob = new TelepathyAddPendingContactJob( account() );
			addPendingContactJob->setPendingContact( tempContact );
			addPendingContactJob->start();
		}
		else
		{
			// If the contact doesn't exist in Kopete, create it
			if( !account()->contacts()[contactId] )
			{
				createContact(tempContact);
			}
			// else, set the internal telepathy object in the existing contact.
			else
			{
				kDebug(TELEPATHY_DEBUG_AREA) << k_funcinfo << "Set internal information from Telepathy to " << contactId;
				TelepathyContact *contact = static_cast<TelepathyContact*>( account()->contacts()[contactId] );
				contact->setInternalContact(tempContact);
			}
		}
	}
}

void TelepathyContactManager::telepathyAuthorizationRequired(QtTapioca::Contact *newContact)
{
	kDebug(TELEPATHY_DEBUG_AREA) << k_funcinfo << "A contact want authorization";
	
	QString contactUri = newContact->uri();

	// Add and/or authorize the pending contact
	TelepathyAddPendingContactJob *addPendingContactJob = new TelepathyAddPendingContactJob( account() );
	addPendingContactJob->setPendingContact( newContact );

	// Set to authorization only if the contact already exist in Kopete
	addPendingContactJob->setAuthorizeOnly( account()->contacts()[contactUri] ? true : false );

	addPendingContactJob->start();
}

void TelepathyContactManager::telepathySubscriptionAccepted(QtTapioca::Contact *contact)
{
	kDebug(TELEPATHY_DEBUG_AREA) << k_funcinfo << "Contact " << contact->uri() << " has accepted your subscription request.";
}

void TelepathyContactManager::createContact(QtTapioca::Contact *telepathyContact)
{
	QString contactId = telepathyContact->uri();

	kDebug(TELEPATHY_DEBUG_AREA) << k_funcinfo << "Creating Telepathy contact \"" << contactId << "\" in Kopete.";

	Kopete::MetaContact *metaContact = new Kopete::MetaContact();

	// Create the TelepathyContact
	TelepathyContact *newContact = new TelepathyContact( account(), contactId, metaContact );
	newContact->setInternalContact(telepathyContact);
	newContact->setMetaContact(metaContact);

	Kopete::ContactList::self()->addMetaContact( metaContact );
}

#include "telepathycontactmanager.moc"
