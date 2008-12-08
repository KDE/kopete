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

#include <QtCore/QEvent>
#include <QtCore/QHash>

#include <kdebug.h>

using namespace KIrc;

class KIrc::ContextPrivate
{
public:
	ContextPrivate()
		: defaultCodec(0)
	{ }

	KIrc::EntityPtr entity;

	KIrc::EntityList entities;

	//Keeps Status information about the entities, relative to this context (e.g. Operators of a Channel)
	QHash<KIrc::EntityPtr, KIrc::EntityStatus> statusMap;

	QTextCodec *defaultCodec;
};

Context::Context(QObject *parent)
	: Handler(parent)
	, d_ptr(new ContextPrivate)
{
}

Context::~Context()
{
	delete d_ptr;
}
/*
QList<KIrc::Entity *> Context::anonymous() const
{

}
*/

KIrc::EntityPtr Context::owner() const
{
	Q_D( const Context );
	return d->entity;
}

void Context::setOwner(KIrc::EntityPtr entity)
{
	Q_D( Context );
	d->entity=entity;
}

KIrc::EntityList Context::entities() const
{
	Q_D(const Context);
	return d->entities;
}

KIrc::EntityPtr Context::entityFromName(const QByteArray &name)
{
	Q_D(Context);
	EntityPtr entity;

	QByteArray nick=name;

	if (nick.isEmpty())
		return entity;

	if (nick.contains( '!' ) ) //Its the extended format, containing hostname and stuff. only search for the nick
	  nick=name.left( nick.indexOf( '!' ) );

	//TODO: optimize this using Hash or something
	foreach( EntityPtr e, d->entities )
	{
		if ( e->name()==nick )
		{
			entity=e;
			break;
		}
	}

	if (!entity)
	{
		entity = new Entity(this);
		entity->setName(nick);
		entity->setStatus(KIrc::Online);
		add( entity );
	}

	return entity;
}

KIrc::EntityList Context::entitiesFromNames(const QList<QByteArray> &names)
{
	EntityList entities;
	EntityPtr entity;

	// This is slow and can easily be optimised with sorted lists
	foreach (const QByteArray &name, names)
	{
		entity = entityFromName(name);
		if (entity)
			entities.append(entity);
//		else
//			warn the user
	}

	return entities;
}

KIrc::EntityList Context::entitiesFromNames(const QByteArray &names, char sep)
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

void Context::postEvent(QEvent *event)
{
	// FIXME: use the Qt event system here
	emit ircEvent( event );
	delete event;
}

void Context::add(EntityPtr entity)
{
	Q_D(Context);
	if (!d->entities.contains(entity))
	{
		d->entities.append(entity);
		connect(entity.data(), SIGNAL(aboutToBeDestroyed( KIrc::Entity* )),
				this, SLOT(onEntityAboutToBeDestroyed(KIrc::Entity* )));
	}
}

void Context::remove(EntityPtr entity)
{
	Q_D(Context);

	d->entities.removeAll( entity );
	d->statusMap.remove( entity );
	disconnect(entity.data());
}

EntityStatus Context::statusOf(EntityPtr e) const
{
	Q_D(const Context);
	return e->status()|d->statusMap.value(e);
}

void Context::setStatus(EntityPtr entity, KIrc::EntityStatus s)
{
	Q_D(Context);
	d->statusMap[entity]=s;
}

void Context::addStatus(EntityPtr entity, KIrc::EntityStatus s)
{
	Q_D(Context);
	d->statusMap[entity]|=s;
}

#if 0
Status Context::SET()
{
}
#endif


void Context::onEntityAboutToBeDestroyed( KIrc::Entity* entity )
{
	remove( KIrc::EntityPtr( entity ) );
}

