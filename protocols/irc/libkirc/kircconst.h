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

#include <QRegExp>

class QTextCodec;

/**
 * @author Michel Hermier <michel.hermier@wanadoo.fr>
 */
namespace KIRC
{
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
	extern const QByteArray AWAY;
	extern const QByteArray ERROR;
	extern const QByteArray INVITE;
	extern const QByteArray ISON;
	extern const QByteArray JOIN;
	extern const QByteArray KICK;
	extern const QByteArray LIST;
	extern const QByteArray MODE;
	extern const QByteArray MOTD;
	extern const QByteArray NICK;
	extern const QByteArray NOTICE;
	extern const QByteArray PART;
	extern const QByteArray PASS;
	extern const QByteArray PING;
	extern const QByteArray PONG;
	extern const QByteArray PRIVMSG;
	extern const QByteArray QUIT;
	extern const QByteArray SQUIT;
	extern const QByteArray TOPIC;
	extern const QByteArray USER;
	extern const QByteArray WHO;
	extern const QByteArray WHOIS;
	extern const QByteArray WHOWAS;

	// Static codecs
	extern QTextCodec *UTF8;
}

#endif

