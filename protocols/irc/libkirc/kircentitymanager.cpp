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

#include "kircentitymanager.h"

#include "kircentity.h"

using namespace KIRC;

class EntityManagerPrivate
{
public:
	QList<Entity *> entities;
};

EntityManager::EntityManager(QObject *parent)
	: QObject(parent),
	  d(new EntityManagerPrivate)
{
}

EnitytManager::~EntityManager()
{
	delete d;
}

QList<Entity *> EntityManager::entities() const
{
	return d->entities;
}

Entity *Engine::entityByName(const QString &name) const
{
	Entity *entity = 0;

	#warning Do the searching code here.

	return entity;
}

Entity *Engine::entityByName(const QString &name, bool createIfNotFound)
{
	Entity *entity = (const) entityByName(name);

	if (!entity)
	{
		entity = new Entity(name);
		add(entity);
	}

	return entity;
}

EntityManager &EntityManager::add(Entity *entity)
{
	if (d->entities.contains(entity))
		return;

	d->entities.append(entity);
	connect(entity, SIGNAL(destroyed(KIRC::Entity *)),
		this, SLOT(remove(KIRC::Entity *)));

	return *this;
}

EntityManager &EntityManager::remove(Entity *entity)
{
	d->entities.remove(entity);
//	disconnect(entity);

	return *this;
}

#include "kircentitymanager.moc"

