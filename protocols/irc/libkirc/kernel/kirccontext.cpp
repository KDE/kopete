/*
    kirccontext.cpp - IRC Context

    Copyright (c) 2005-2007 by Michel Hermier <michel.hermier@gmail.com>

    Kopete    (c) 2005-2007 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "kirccontext.moc"

#include "kircentity.h"

using namespace KIrc;

class KIrc::ContextPrivate
{
public:
	ContextPrivate()
		: defaultCodec(0)
	{ }

	QList<Entity *> entities;

	QTextCodec *defaultCodec;
};

Context::Context(QObject *parent)
	: QObject(parent)
//	, d(new Private)
{
}

Context::~Context()
{
//	delete d;
}

Entity::List Context::entities() const
{
	Q_D(const Context);

	Entity::List entities;

	foreach(Entity *entity, d->entities)
		entities.append(Entity::Ptr(entity));

	return entities;
}

Entity::Ptr Context::entityFromName(const QByteArray &name) const
{
	Entity::Ptr entity;

	if (name.isEmpty())
		return entity;

#ifdef __GNUC__
	#warning Do the searching code here.
#endif

	return entity;
}
/*
Entity::Ptr Context::entityFromName(const QByteArray &name, bool createIfNotFound)
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

Entity::List Context::entitiesFromNames(const QList<QByteArray> &names)
{
	Entity::List entities;

	foreach (const QByteArray &name, names)
		entities.append(entityFromName(name));

	return entities;
}

Entity::List Context::entitiesFromNames(const QByteArray &names, char sep)
{
	return entitiesFromNames(names.split(sep));
}

QTextCodec *Context::defaultCodec() const
{
	Q_D(const Context);

	return d->defaultCodec;
}

void Context::setDefaultCodec(QTextCodec *defaultCodec)
{
	Q_D(Context);

	d->defaultCodec = defaultCodec;
}

void Context::postEvent(Event *event)
{
#warning CODE ME
//	delete event;
}
#if 0
void Context::add(Entity *entity)
{
	if (!d->entities.contains(entity))
	{
		d->entities.append(entity);
		connect(entity, SIGNAL(destroyed(KIrc::Entity *)),
			this, SLOT(remove(KIrc::Entity *)));
	}
}

void Context::remove(Entity *entity)
{
	d->entities.removeAll(entity);
//	disconnect(entity);
}
#endif

#if 0
Status Context::SET()
{
}
#endif
