/*
   contactlist.cpp - Windows Live Messenger Contact List

   Copyright (c) 2006 by Michaël Larouche <larouche@kde.org>

   *************************************************************************
   *                                                                       *
   * This library is free software; you can redistribute it and/or         *
   * modify it under the terms of the GNU Lesser General Public            *
   * License as published by the Free Software Foundation; either          *
   * version 2 of the License, or (at your option) any later version.      *
   *                                                                       *
   *************************************************************************
 */
#include "Papillon/ContactList"

// Qt includes
#include <QtCore/QPointer>

// Papillon includes
#include "Papillon/Client"
#include "Papillon/Contact"

// Papillon private includes
#include "fetchcontactlistjob.h"

namespace Papillon 
{

class ContactList::Private
{
public:
	Private()
	{}
	
	~Private()
	{
	}

	QPointer<Client> client;
	// Contacts are deleted by QObject subclassing
	QList<Contact*> contacts;
};

ContactList::ContactList(Client *client)
 : QObject(client), d(new Private)
{
	d->client = client;
}


ContactList::~ContactList()
{
	delete d;
}

Client *ContactList::client()
{
	return d->client;
}

QList<Papillon::Contact*> ContactList::contacts() const
{
	QList<Contact*> result;
	
	foreach(Contact *contact, d->contacts)
	{
		if( contact->lists() & ContactListEnums::ForwardList )
		{
			result << contact;
		}
	}
	return result;
}

QList<Papillon::Contact*> ContactList::allowList() const
{
	QList<Contact*> result;

	foreach(Contact *contact, d->contacts)
	{
		if( contact->lists() & ContactListEnums::AllowList )
		{
			result << contact;
		}
	}

	return result;
}

QList<Papillon::Contact*> ContactList::blockList() const
{
	QList<Contact*> result;
	
	foreach(Contact *contact, d->contacts)
	{
		if( contact->lists() & ContactListEnums::BlockList )
		{
			result << contact;
		}
	}

	return result;
}

QList<Papillon::Contact*> ContactList::reverseList() const
{
	QList<Contact*> result;

	foreach(Contact *contact, d->contacts)
	{
		if( contact->lists() & ContactListEnums::ReverseList )
		{
			result << contact;
		}
	}

	return result;
}

QList<Papillon::Contact*> ContactList::pendingList() const
{
	QList<Contact*> result;

	foreach(Contact *contact, d->contacts)
	{
		if( contact->lists() & ContactListEnums::PendingList )
		{
			result << contact;
		}
	}

	return result;
}

Contact* ContactList::contact(const QString &contactId)
{
	Contact *foundContact = 0, *contact = 0;
	
	foreach(contact, d->contacts)
	{
		if( contact->contactId() == contactId || contact->passportId() == contactId )
		{
			foundContact = contact;
			break;
		}
	}

	return foundContact;
}

void ContactList::load()
{
	FetchContactListJob *fetchJob = new FetchContactListJob(this);
	// FIXME: Temp
	connect(fetchJob, SIGNAL(finished(Papillon::FetchContactListJob*)), this, SIGNAL(contactListLoaded()));
	fetchJob->execute();
}

Contact* ContactList::createContact(const QString &contactId)
{
	if( !contact(contactId) )
	{
		Contact *newContact = new Contact(this);
		newContact->setContactId(contactId);

		d->contacts << newContact;
	}

	return contact(contactId);
}

}

#include "contactlist.moc"
