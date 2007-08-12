/*
	addressbook.cpp: addressbook related class

    Copyright (c) 2007		by Zhang Panyong	        <pyzhang@gmail.com>
    Kopete    (c) 2002-2005 by the Kopete developers	<kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

// Papillon includes
#include "Papillon/Client"

namespace Papillon 
{
Class AddressBook::Private
{
public:
	Private()
	{}
	
	~Private()
	{
	}

	QPointer<Client> client;
}

AddressBook::AddressBook(Client *client)
 : QObject(client), d(new Private)
{
	d->client = client;
}


AddressBook::~AddressBook()
{
	delete d;
}

Client *AddressBook::client()
{
	return d->client;
}

void AddressBook::load()
{
	FetchAddressBookJob *fetchJob = new FetchAddressBookJob(this);
	// FIXME:
	connect(fetchJob, SIGNAL(finished(Papillon::FetchAddressBookJob*)), this, SIGNAL(AddressbookLoaded()));
	fetchJob->execute();
}

}
#include "addressbook.moc"
