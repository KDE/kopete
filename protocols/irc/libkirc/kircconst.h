/*
    kirc.h - Constants and enums of the KIRC namespace.

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

#ifndef KIRC_H
#define KIRC_H

#include <qregexp.h>
#include <qstring.h>

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

	// Static regular expressions
	static const QRegExp sm_RemoveLinefeeds;

	// Static strings
	static const QString AWAY;
	static const QString ERROR;
	static const QString INVITE;
	static const QString ISON;
	static const QString JOIN;
	static const QString KICK;
	static const QString LIST;
	static const QString MODE;
	static const QString MOTD;
	static const QString NICK;
	static const QString NOTICE;
	static const QString PART;
	static const QString PASS;
	static const QString PING;
	static const QString PONG;
	static const QString PRIVMSG;
	static const QString QUIT;
	static const QString SQUIT;
	static const QString TOPIC;
	static const QString USER;
	static const QString WHO;
	static const QString WHOIS;
	static const QString WHOWAS;
}

#endif
