/*
    kircentity.cpp - IRC Client

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

#include "kircengine.h"
#include "kircentity.h"

#include <kdebug.h>

using namespace KIRC;
using namespace KNetwork;

/**
 * Match a possible user definition:
 * nick!user@host
 * where user and host are optionnal.
 * NOTE: If changes are done to the regexp string, update also the sm_userStrictRegExp regexp string.
 */
const QRegExp Entity::sm_userRegExp(QString::fromLatin1("^([^\\s,:!@]+)(?:(?:!([^\\s,:!@]+))?(?:@([^\\s,!@]+)))?$"));

/**
 * Regexp to match strictly the complete user definition:
 * nick!user@host
 * NOTE: If changes are done to the regexp string, update also the sm_userRegExp regexp string.
 */
const QRegExp Entity::sm_userStrictRegExp(QString::fromLatin1("^([^\\s,:!@]+)!([^\\s,:!@]+)@([^\\s,:!@]+)$"));

const QRegExp Entity::sm_channelRegExp( QString::fromLatin1("^[#!+&][^\\s,]+$") );

Entity::Entity(const QString &, const Type type)
	: QObject(0, "KIRC::Entity"),
	  m_type(type)
{
//	rename(name, type);
}

Entity::~Entity()
{
	emit destroyed(this);
}

QString Entity::name() const
{
	return m_name;
}

QString Entity::host() const
{
	switch(m_type)
	{
//	case Unknown:
	case Server:
		return m_name;
//	case Channel:
	case Service:
	case User:
		return userHost();
	default:
		kdDebug(14121) << k_funcinfo << "No host defined for type:" << m_type;
		return QString::null;
	}
}

KIRC::Entity::Type Entity::type() const
{
	return m_type;
}

KIRC::Entity::Type Entity::guessType()
{
	m_type = guessType(m_name);
	return m_type;
}

// FIXME: Implement me
KIRC::Entity::Type Entity::guessType(const QString &)
{
	return Unknown;
}

QString Entity::userNick() const
{
	return userNick(m_name);
}

QString Entity::userNick(const QString &s)
{
	return userInfo(s, 1);
}

QString Entity::userName() const
{
	return userName(m_name);
}

QString Entity::userName(const QString &s)
{
	return userInfo(s, 2);
}

QString Entity::userHost() const
{
	return userHost(m_name);
}

QString Entity::userHost(const QString &s)
{
	return userInfo(s, 3);
}

QString Entity::userInfo(const QString &s, int num)
{
	QRegExp userRegExp(sm_userRegExp);
	userRegExp.search(s);
	return userRegExp.cap(num);
}

#include "kircentity.moc"

