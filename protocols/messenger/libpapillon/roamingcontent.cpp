/*
	roamingcontent.cpp: Windows Roaming content processing

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
#include "Papillon/RoamingContent"

// Papillon includes
#include "Papillon/Client"
namespace Papillon
{

Class RoamingContent::Private
{
public:
	Private()
	{}
	
	~Private()
	{
	}

	QPointer<Client> client;
}

RoamingContent::RoamingContent(Client *client)
 : QObject(client), d(new Private)
{
	d->client = client;
}

RoamingContent::~RoamingContent()
{
	delete d;
}

Client *RoamingContent::client()
{
	return d->client;
}

}

#include "addressbook.moc"
