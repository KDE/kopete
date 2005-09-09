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

#include "kircentity.h"

namespace KIRC
{

class EntityManagerPrivate;

/**
 * @author Michel Hermier <michel.hermier@wanadoo.fr>
 */
class EntityManager
{
	Q_OBJECT

public:
	EntityManager(QObject *parent = 0);

public:
	QList<KIRC::Entity *> entities() const;
//	QList<KIRC::Entity *> entitiesByHost(...) const;
//	QList<KIRC::Entity *> entitiesByServer(...) const;
//	QList<KIRC::Entity *> entitiesByType(...) const;

	KIRC::Entity *entityByName(const QByteArray &name) const;
	KIRC::Entity *entityByName(const QByteArray &name, bool createIfNotFound = false);

public slots:
	EntityManager &add(KIRC::Entity *entity);
	EntityManager &remove(KIRC::Entity *entity);

private:
	EntityManagerPrivate *d;
};

}

#endif

