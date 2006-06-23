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

#ifndef KIRC_ENTITYMANAGER_H
#define KIRC_ENTITYMANAGER_H

#include <QObject>

class QByteArray;

namespace KIRC
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
	QList<KIRC::Entity *> entities() const;
//	QList<KIRC::Entity *> entitiesByHost(...) const;
//	QList<KIRC::Entity *> entitiesByServer(...) const;
//	QList<KIRC::Entity *> entitiesByType(...) const;

	KIRC::Entity *entityByName(const QByteArray &name) const;

protected:
	void add(KIRC::Entity *entity);
//	void add(const QList<KIRC::Entity *> &entities);

	void remove(KIRC::Entity *entity);
//	void remove(const QList<KIRC::Entity *> &entities);

private:
	Q_DISABLE_COPY(EntityManager);

	class Private;
	Private * const d;
};

}

#endif

