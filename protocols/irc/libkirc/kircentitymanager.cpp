/*
    kircentitymanager.cpp - IRC Entity Manager

    Copyright (c) 2005      by Michel Hermier <michel.hermier@wanadoo.fr>

    Kopete    (c) 2005      by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "kircentitymanager.moc"

#include "kircentity.h"

using namespace KIRC;

class EntityManager::Private
{
public:
	QList<Entity *> entities;
};

EntityManager::EntityManager(QObject *parent)
	: QObject(parent),
	  d(new Private)
{
}

EntityManager::~EntityManager()
{
	delete d;
}

QList<Entity *> EntityManager::entities() const
{
	return d->entities;
}

Entity *EntityManager::entityByName(const QByteArray &name) const
{
	if (name.isEmpty())
		return 0;

	Entity *entity = 0;

	#warning Do the searching code here.

	return entity;
}
/*
Entity *EntityManager::entityByName(const QByteArray &name, bool createIfNotFound)
{
	Entity *entity = entityByName(name);

	if (!entity)
	{
		entity = new Entity(name);
		add(entity);
	}

	return entity;
}
*/
EntityManager &EntityManager::add(Entity *entity)
{
	if (!d->entities.contains(entity))
	{
		d->entities.append(entity);
		connect(entity, SIGNAL(destroyed(KIRC::Entity *)),
			this, SLOT(remove(KIRC::Entity *)));
	}

	return *this;
}

EntityManager &EntityManager::remove(Entity *entity)
{
	d->entities.remove(entity);
//	disconnect(entity);

	return *this;
}

