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

namespace Papillon 
{

class ContactList::Private
{
public:
	Private()
	{}
	
	QPointer<Client> client;
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

}

#include "contactlist.moc"
