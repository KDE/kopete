
/*
    kircentity.h - IRC Client

    Copyright (c) 2004      by Michel Hermier <michel.hermier@wanadoo.fr>

    Kopete    (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>

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

	inline QString getNick() const
		{ return getNickFromPrefix(m_name); }

//	inline QString getUser() const
//		{ return getUserFromPrefix(m_name); }

//	inline QString getAddress()
//		{ return ; }

	inline static QString getNickFromPrefix(const QString &s)
		{ return s.section('!', 0, 0); }

	inline bool isChannel()
		{ return isChannel(m_name); };

	inline static bool isChannel( const QString &s )
		{ return channelRegExp.exactMatch(s); };

private:
	static const QRegExp userRegExp;
	static const QRegExp channelRegExp;

	QString	m_name;

	// peer ip address if the entity is a User.
	QString m_address;
};

#endif
