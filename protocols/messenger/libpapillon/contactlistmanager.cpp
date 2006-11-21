/*
   contactlistmanager.cpp - Windows Live Messenger Contact List

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
#include "Papillon/ContactListManager"

// Qt includes
#include <QtXml/QDomDocument>

namespace Papillon 
{

class ContactListManager::Private
{
public:
	Private()
	{}
};

ContactListManager::ContactListManager(QObject *parent)
 : QObject(parent), d(new Private)
{
}


ContactListManager::~ContactListManager()
{
	delete d;
}


}

#include "contactlistmanager.moc"
