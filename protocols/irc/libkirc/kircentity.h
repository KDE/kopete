/*
    kircentity.h - IRC Client

    Copyright (c) 2004      by Michel Hermier <michel.hermier@wanadoo.fr>

    Kopete    (c) 2004      by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef KIRCENTITY_H
#define KIRCENTITY_H

#include "kircconst.h"

#include <ksharedptr.h>

#include <qobject.h>
#include <qregexp.h>
#include <qstring.h>
#include <qvaluelist.h>

class QTextCodec;

namespace KIRC
{

class Engine;

class Entity
	: public QObject,
	  public KShared
{
	Q_OBJECT

public:
	static KIRC::EntityType guessType(const QString &name);

	Entity(const QString &name = QString::null, const KIRC::EntityType type = Unknown);
	virtual ~Entity();

	bool operator == (const Entity &);

	KIRC::EntityType type() const;
	KIRC::EntityType guessType();

	void setName(const QString &);
	QString name() const;

	QString host() const;

	void setCodec(QTextCodec *);
	QTextCodec *codec() const;

	// FIXME: Remove these is* functions ... They are duplicate with the ::guessType(const QString&)
	inline static bool isUser( const QString &s )
		{ return sm_userRegExp.exactMatch(s); };
	inline bool isChannel()
		{ return isChannel(m_name); };
	inline static bool isChannel( const QString &s )
		{ return sm_channelRegExp.exactMatch(s); };

signals:
	void destroyed(KIRC::Entity *self);

	void updated();

private:

	static QString userInfo(const QString &s, int num_cap);

	static const QRegExp sm_userRegExp;
	static const QRegExp sm_userStrictRegExp;
	static const QRegExp sm_channelRegExp;

	KIRC::EntityType m_type;

	QString	m_name;
	QString m_host;

	QTextCodec *m_codec;
};

class EntityPtr
	: public KSharedPtr<KIRC::Entity>
{
public:
	EntityPtr(KIRC::Entity *entity = 0)
		: KSharedPtr<KIRC::Entity>(entity)
	{ }

	EntityPtr(const KIRC::EntityPtr &entity)
		: KSharedPtr<KIRC::Entity>(entity)
	{ }
};

class EntityPtrList
	: public QValueList<EntityPtr>
{
public:
	EntityPtrList()
	{ }

	EntityPtrList(const EntityPtr &entity)
	{
		append(entity);
	}

	EntityPtrList(const QValueList<EntityPtr> &list)
		: QValueList<EntityPtr>(list)
	{ }
};

}

#endif
