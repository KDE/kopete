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

#include <kdeversion.h>
#include <kresolver.h>

#include <qobject.h>
#include <qregexp.h>
#include <qstring.h>

namespace KIRC
{

class Engine;

class Entity
	: public QObject
{
	Q_OBJECT

public:
	typedef enum Type
	{
		Unknown,
		Server,
		Channel,
		Service,
		User
	};

	Entity(KIRC::Engine *engine,const QString &name, const Type type = Unknown);

	QString name() const;
	QString host() const;

	KIRC::Entity::Type type() const;
	KIRC::Entity::Type guessType();
	static KIRC::Entity::Type guessType(const QString &name);

	// FIXME: Remove these is* functions ... They are duplicate with the ::guessType(const QString&)
	inline static bool isUser( const QString &s )
		{ return sm_userRegExp.exactMatch(s); };
	inline bool isChannel()
		{ return isChannel(m_name); };
	inline static bool isChannel( const QString &s )
		{ return sm_channelRegExp.exactMatch(s); };

	QString userNick() const;
	static QString userNick(const QString &s);

	QString userName() const;
	static QString userName(const QString &s);

	QString userHost() const;
	static QString userHost(const QString &s);




	inline KNetwork::KResolver::StatusCodes resolverStatus()
		{ return (KNetwork::KResolver::StatusCodes)getResolver()->status(); }

	KNetwork::KResolverResults resolve(bool *success = 0);
	void resolveAsync();
	KNetwork::KResolverResults resolverResults()
		{ return getResolver()->results(); }

signals:
	void resolverResults(KNetwork::KResolverResults);

private:
	KNetwork::KResolver *getResolver();

	static QString userInfo(const QString &s, int num_cap);

	static const QRegExp sm_userRegExp;
	static const QRegExp sm_userStrictRegExp;
	static const QRegExp sm_channelRegExp;

	KIRC::Entity::Type m_type;
	QString	m_name;

	// peer ip address if the entity is a User.
	QString m_address;
	KNetwork::KResolver *m_resolver;
};

}

#endif
