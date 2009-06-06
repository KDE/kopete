/*
    kopeteblacklister.cpp - Kopete BlackLister

    Copyright (c) 2004      by Roie Kerstein         <sf_kersteinroie@bezeqint.net>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "kopeteblacklister.h"

#include "kopetecontact.h"

#include <ksharedconfig.h>
#include <kconfiggroup.h>
#include <kglobal.h>

#include <qstringlist.h>

namespace Kopete
{

class BlackLister::Private
{
public:
	QStringList blacklist;
	QString owner;
	QString protocol;
};


BlackLister::BlackLister(const QString &protocolId, const QString &accountId, QObject *parent)
 : QObject(parent), d( new Private )
{
	KConfigGroup config = KGlobal::config()->group("BlackLister");
	
	d->owner = accountId;
	d->protocol = protocolId;
	d->blacklist = config.readEntry( d->protocol + QString::fromLatin1("_") + d->owner, QStringList() );
}

BlackLister::~BlackLister()
{
	delete d;
}


bool BlackLister::isBlocked(const QString &contactId)
{
	return (d->blacklist.indexOf( contactId ) != -1 );
}

bool BlackLister::isBlocked(Contact *contact)
{
	return isBlocked(contact->contactId());
}

void BlackLister::addContact(const QString &contactId)
{
	if( !isBlocked(contactId) )
	{
		d->blacklist += contactId;
		saveToDisk();
		emit contactAdded( contactId );
	}
}

void BlackLister::addContact(Contact *contact)
{
	QString temp = contact->contactId();
	
	addContact( temp );
}

void BlackLister::removeContact(Contact *contact)
{
	QString temp = contact->contactId();
	
	removeContact( temp );
}

void BlackLister::saveToDisk()
{
	KConfigGroup config = KGlobal::config()->group("BlackLister");
	config.writeEntry( d->protocol + QString::fromLatin1("_") + d->owner, d->blacklist );
	config.sync();
}

void BlackLister::removeContact(const QString &contactId)
{
	if( isBlocked(contactId) )
	{
		d->blacklist.removeAll( contactId );
		saveToDisk();
		emit contactRemoved( contactId );
	}
}

}

#include "kopeteblacklister.moc"
