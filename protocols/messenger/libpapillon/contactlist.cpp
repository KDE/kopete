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
#include <QtXml/QDomDocument>

namespace Papillon 
{

class ContactList::Private
{
public:
	Private()
	{}
};

ContactList::ContactList(QObject *parent)
 : QObject(parent), d(new Private)
{
}


ContactList::~ContactList()
{
	delete d;
}


}

#include "contactlist.moc"
