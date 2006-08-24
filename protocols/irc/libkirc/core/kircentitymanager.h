/*
    kircentitymanager.h - IRC Entity Manager

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

#ifndef KIRCENTITYMANAGER_H
#define KIRCENTITYMANAGER_H

#include <QObject>

class QByteArray;

namespace KIrc
{

class Entity;

/**
 * @author Michel Hermier <michel.hermier@wanadoo.fr>
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
	QList<Entity *> entities() const;
//	QList<Entity *> entitiesByHost(...) const;
//	QList<Entity *> entitiesByServer(...) const;
//	QList<Entity *> entitiesByType(...) const;

	Entity *entityByName(const QByteArray &name) const;

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

