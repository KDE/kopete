/*
    kopeteavatarmanager.cpp - Global avatar manager

    Copyright (c) 2007      by Michaël Larouche      <larouche@kde.org>

    Kopete    (c) 2002-2007 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/
#include "kopeteavatarmanager.h"

// Qt includes

// KDE includes
#include <kdebug.h>

namespace Kopete
{

//BEGIN AvatarManager
AvatarManager *AvatarManager::s_self = 0;

AvatarManager *AvatarManager::self()
{
	if( !s_self )
	{
		s_self = new AvatarManager;
	}
	return s_self;
}

class AvatarManager::Private
{
public:
};

AvatarManager::AvatarManager(QObject *parent)
 : QObject(parent), d(new Private)
{
}

AvatarManager::~AvatarManager()
{
	delete d;
}

QList<Kopete::AvatarManager::AvatarEntry> AvatarManager::query(Kopete::AvatarManager::AvatarCategory category)
{
	QList<Kopete::AvatarManager::AvatarEntry> result;

	return result;
}

void AvatarManager::add(Kopete::AvatarManager::AvatarEntry newEntry)
{
	Q_UNUSED(newEntry);
}

void AvatarManager::remove(Kopete::AvatarManager::AvatarEntry entryToRemove)
{
	Q_UNUSED(entryToRemove);
}
//END AvatarManager

//BEGIN AvatarQueryJob
class AvatarQueryJob::Private
{
public:
	Private()
	 : category(AvatarManager::All)
	{}

	AvatarManager::AvatarCategory category;
	QList<AvatarManager::AvatarEntry> avatarList;
};

AvatarQueryJob::AvatarQueryJob(QObject *parent)
 : KJob(parent), d(new Private)
{
}

AvatarQueryJob::~AvatarQueryJob()
{
	delete d;
}

void AvatarQueryJob::setQueryFilter(Kopete::AvatarManager::AvatarCategory category)
{
	d->category = category;
}

void AvatarQueryJob::start()
{
}

QList<Kopete::AvatarManager::AvatarEntry> AvatarQueryJob::avatarList() const
{
	return d->avatarList;
}

//END AvatarQueryJob

}

#include "kopeteavatarmanager.moc"
