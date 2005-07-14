/*
    kircconst.h - The KIRC constants & enums.

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

#ifndef KIRCCONST_H
#define KIRCCONST_H

#include <ksharedptr.h>

#include <qregexp.h>
#include <qstring.h>
#include <qvaluelist.h>

class QTextCodec;

/**
 * @author Michel Hermier <michel.hermier@wanadoo.fr>
 */
namespace KIRC
{
	enum ConnectionState
	{
		Idle,
		Connecting,
		Authentifying,
		Connected,
		Closing
	};

	enum MessageType
	{
		ErrorMessage = -1,

		PrivateMessage,
		InfoMessage,

		MessageOfTheDayMessage,
		MessageOfTheDayCondensedMessage
	};

	typedef enum EntityType
	{
		// From lower to higher importance
		Unknown = 0,
		User,
		Service,
		Channel,
		Server
	};

	typedef struct EntityStatus
	{
		KIRC::EntityType type	: 4;
		bool online		: 1;

		bool mode_a		: 1;
		bool mode_i		: 1;
		bool mode_o		: 1;
		bool mode_r		: 1;
		bool mode_s		: 1;
		bool mode_v		: 1;
		bool mode_w		: 1;
		bool mode_O		: 1;

		EntityStatus()
		{
			type = KIRC::Unknown;
			online = false;

			mode_a = false;
			mode_i = false;
			mode_o = false;
			mode_r = false;
			mode_s = false;
			mode_v = false;
			mode_w = false;
			mode_O = false;
		}

		bool operator < (const KIRC::EntityStatus &o) const
		{
			return	(type < o.type) ||
				(online < o.online) ||
				(mode_a < o.mode_a) ||
				(mode_i < o.mode_i) ||
				(mode_o < o.mode_o) ||
				(mode_r < o.mode_r) ||
				(mode_s < o.mode_s) ||
				(mode_v < o.mode_v) ||
				(mode_w < o.mode_w) ||
				(mode_O < o.mode_O);
		}
	};

	// Static regular expressions
	extern const QRegExp sm_RemoveLinefeeds;

	// Static strings
	extern const QString AWAY;
	extern const QString ERROR;
	extern const QString INVITE;
	extern const QString ISON;
	extern const QString JOIN;
	extern const QString KICK;
	extern const QString LIST;
	extern const QString MODE;
	extern const QString MOTD;
	extern const QString NICK;
	extern const QString NOTICE;
	extern const QString PART;
	extern const QString PASS;
	extern const QString PING;
	extern const QString PONG;
	extern const QString PRIVMSG;
	extern const QString QUIT;
	extern const QString SQUIT;
	extern const QString TOPIC;
	extern const QString USER;
	extern const QString WHO;
	extern const QString WHOIS;
	extern const QString WHOWAS;

	// Static codecs
	extern QTextCodec *UTF8;

	// Some typdefs
	class Entity;
	typedef KSharedPtr<KIRC::Entity> EntityPtr;
	typedef QValueList<KIRC::EntityPtr> EntityPtrList;
}

#endif

