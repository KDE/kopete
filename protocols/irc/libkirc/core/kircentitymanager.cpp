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

using namespace KIrc;

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

Entity::List EntityManager::entities() const
{
	Entity::List entities;

	foreach(Entity *entity, d->entities)
		entities.append(Entity::Ptr(entity));

	return entities;
}

Entity::Ptr EntityManager::entityFromName(const QByteArray &name) const
{
	Entity::Ptr entity;

	if (name.isEmpty())
		return entity;

	#warning Do the searching code here.

	return entity;
}
/*
Entity::Ptr EntityManager::entityFromName(const QByteArray &name, bool createIfNotFound)
{
	Entity *entity = entityFromName(name);

	if (!entity)
	{
		entity = new Entity(name);
		add(entity);
	}

	return entity;
}
*/

Entity::List EntityManager::entitiesFromNames(const QList<QByteArray> &names)
{
	Entity::List entities;

	foreach (const QByteArray &name, names)
		entities.append(entityFromName(name));

	return entities;
}

Entity::List EntityManager::entitiesFromNames(const QByteArray &names, char sep)
{
	return entitiesFromNames(names.split(sep));
}

void EntityManager::add(Entity *entity)
{
	if (!d->entities.contains(entity))
	{
		d->entities.append(entity);
		connect(entity, SIGNAL(destroyed(KIrc::Entity *)),
			this, SLOT(remove(KIrc::Entity *)));
	}
}

void EntityManager::remove(Entity *entity)
{
	d->entities.removeAll(entity);
//	disconnect(entity);
}

