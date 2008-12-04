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

#include "kirchandler.h"

namespace KIrc
{

class Entity;

class ContextPrivate;

/**
 * @author Michel Hermier <michel.hermier@gmail.com>
 */
class KIRC_EXPORT Context
	: public KIrc::Handler
{
	Q_OBJECT
	Q_DECLARE_PRIVATE(KIrc::Context)

//	Q_INTERFACES(KIrc::CommandHandlerInterface KIrc::MessageHandlerInterface)

private:
	Q_DISABLE_COPY(Context)

public:
	explicit Context(QObject *parent = 0);
	~Context();

public:
	QTextCodec *defaultCodec() const;
	void setDefaultCodec(QTextCodec *codec);

//	Entity::List anonymous();

	KIrc::EntityList entities() const;
//	Entity::List entitiesByHost(...) const;
//	Entity::List entitiesByServer(...) const;
//	Entity::List entitiesByType(...) const;

	KIrc::EntityPtr entityFromName(const QByteArray &name);

	KIrc::EntityList entitiesFromNames(const QList<QByteArray> &names);

	KIrc::EntityList entitiesFromNames(const QByteArray &names, char sep = ',');

	KIrc::EntityStatus statusOf(EntityPtr entity) const;
	void setStatus(EntityPtr entity, KIrc::EntityStatus s);
	void addStatus(EntityPtr entity, KIrc::EntityStatus s);
public Q_SLOTS:
	void add(EntityPtr entity);
	void remove(EntityPtr entity);

public:
	void postEvent(QEvent *event);

Q_SIGNALS:
	void ircEvent(QEvent *event);

public:
	/* This command allow to set and get values.
	 * Syntax: SET variable [new_value]
	 */
//	Status SET();

//	Status execute();
private Q_SLOTS:
	void onEntityAboutToBeDestroyed(KIrc::Entity* entity);

protected:
	ContextPrivate * const d_ptr;
};

}

#endif

