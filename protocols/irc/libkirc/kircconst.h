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

#include <QList>
#include <QRegExp>

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

		JoinMessage,
		PartMessage,

		PrivateMessage,
		InfoMessage,
		NoticeMessage,

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
/*
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
*/
	// Static regular expressions
	extern const QRegExp sm_RemoveLinefeeds;

	// Static strings
	extern const char *AWAY;
	extern const char *ERROR;
	extern const char *INVITE;
	extern const char *ISON;
	extern const char *JOIN;
	extern const char *KICK;
	extern const char *LIST;
	extern const char *MODE;
	extern const char *MOTD;
	extern const char *NICK;
	extern const char *NOTICE;
	extern const char *PART;
	extern const char *PASS;
	extern const char *PING;
	extern const char *PONG;
	extern const char *PRIVMSG;
	extern const char *QUIT;
	extern const char *SQUIT;
	extern const char *TOPIC;
	extern const char *USER;
	extern const char *WHO;
	extern const char *WHOIS;
	extern const char *WHOWAS;

	// Static codecs
	extern QTextCodec *UTF8;

	// Some typdefs
	class Entity;
	typedef KSharedPtr<KIRC::Entity> EntityPtr;
	typedef QList<KIRC::EntityPtr> EntityPtrList;
}

#endif

