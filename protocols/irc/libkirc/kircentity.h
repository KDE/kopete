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

#include <qobject.h>
#include <qregexp.h>
#include <qstring.h>

#include <kdeversion.h>
#if KDE_IS_VERSION( 3, 2, 90 )
#include <qt-addon/qresolver.h>
#else
#define QResolverResults int
#endif
#include <kdemacros.h>

class KIRCEntity
	: public QObject
{
	Q_OBJECT

public:
/*
	enum Type
	{
		Unknown,
		Server,
		Channel,
		User
	};
*/

	KIRCEntity( const QString &name )
		: m_name(name)
		{  }

	inline QString name() const
		{ return m_name; }

	inline QString userNick() const
		{ return userNick(m_name); }
	static QString userNick(const QString &s)
		{ return userInfo(s, 1); }

	inline QString userName() const
		{ return userName(m_name); }
	inline static QString userName(const QString &s)
		{ return userInfo(s, 2); }

	inline QString userHost() const
		{ return userHost(m_name); }
	inline static QString userHost(const QString &s)
		{ return userInfo(s, 3); }

	inline static bool isUser( const QString &s )
		{ return sm_userRegExp.exactMatch(s); };

	inline bool isChannel()
		{ return isChannel(m_name); };

	inline static bool isChannel( const QString &s )
		{ return sm_channelRegExp.exactMatch(s); };

#if KDE_IS_VERSION( 3, 2, 90 )
	inline QResolver::StatusCodes resolverStatus()
		{ return (QResolver::StatusCodes)getResolver()->status(); }

	QResolverResults resolve(bool *success = 0);
	void resolveAsync();
	QResolverResults resolverResults()
		{ return getResolver()->results(); }
#endif
signals:
	void resolverResults(QResolverResults);

protected:
	static QString userInfo(const QString &s, int num_cap);
#if KDE_IS_VERSION( 3, 2, 90 )
	QResolver *getResolver();
#endif
private:
	static const QRegExp sm_userRegExp;
	static const QRegExp sm_channelRegExp;

	QString	m_name;

	// peer ip address if the entity is a User.
	QString m_address;
#if KDE_IS_VERSION( 3, 2, 90 )
	QResolver *m_resolver;
#endif

};

#endif
