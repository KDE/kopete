/*
    kircentitymanager.h - IRC Entity Manager

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

#ifndef KIRCENTITYMANAGER_H
#define KIRCENTITYMANAGER_H

#include "kircentity.h"

class QByteArray;

namespace KIrc
{

class Entity;

/**
 * @author Michel Hermier <michel.hermier@gmail.com>
 */
class EntityManager
	: public QObject
{
	Q_OBJECT

	friend class Entity;

public:
	EntityManager(QObject *parent = 0);
	~EntityManager();

public:
	Entity::List entities() const;
//	Entity::List entitiesByHost(...) const;
//	Entity::List entitiesByServer(...) const;
//	Entity::List entitiesByType(...) const;

	Entity::Ptr entityFromName(const QByteArray &name) const;
//	Entity::Ptr entityFromName(const QByteArray &name, bool );

	Entity::List entitiesFromNames(const QList<QByteArray> &names);

	Entity::List entitiesFromNames(const QByteArray &names, char sep);

protected:
	void add(Entity *entity);
//	void add(const QList<Entity *> &entities);

	void remove(Entity *entity);
//	void remove(const QList<Entity *> &entities);

private:
	Q_DISABLE_COPY(EntityManager);

	class Private;
	Private * const d;
};

}

#endif

