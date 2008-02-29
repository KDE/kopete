/*
    kirccontext.h - IRC Context

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

#ifndef KIRCCONTEXT_H
#define KIRCCONTEXT_H

#include "kircentity.h"

class QByteArray;

namespace KIrc
{

class ContextPrivate;
class Event;

/**
 * @author Michel Hermier <michel.hermier@gmail.com>
 */
class KIRC_EXPORT Context
	: public QObject
//	, public KIrc::CommandHandlerInterface
//	, public KIrc::MessageHandlerInterface
{
	Q_OBJECT
	Q_DECLARE_PRIVATE(KIrc::Context)

//	Q_INTERFACES(Kirc::CommandHandlerInterface Kirc::MessageHandlerInterface)

public:
	explicit Context(QObject *parent = 0);
	~Context();

public:
	QTextCodec *defaultCodec() const;
	void setDefaultCodec(QTextCodec *codec);

//	Entity::List anonymous();

	Entity::List entities() const;
//	Entity::List entitiesByHost(...) const;
//	Entity::List entitiesByServer(...) const;
//	Entity::List entitiesByType(...) const;

	Entity::Ptr entityFromName(const QByteArray &name) const;
//	Entity::Ptr entityFromName(const QByteArray &name, bool );

	Entity::List entitiesFromNames(const QList<QByteArray> &names);

	Entity::List entitiesFromNames(const QByteArray &names, char sep);

public:
	void postEvent(KIrc::Event *event);

#if 0
protected:
	void add(Entity *entity);
//	void add(const QList<Entity *> &entities);

	void remove(Entity *entity);
//	void remove(const QList<Entity *> &entities);
#endif

public:
	/* This command allow to set and get values.
	 * Syntax: SET variable [new_value]
	 */
//	Status SET();

//	Status execute();

private:
	Q_DISABLE_COPY(Context)
};

}

#endif

