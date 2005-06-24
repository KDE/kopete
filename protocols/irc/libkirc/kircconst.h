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

#include <qregexp.h>
#include <qstring.h>

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
		Unknown			=     0,
		Server			= 1<< 0,
		Channel			= 1<< 1,
		Service			= 1<< 2,
		User			= 1<< 3,
		// Mask			= Server|Channel|Service|User,

		Online			= 1<< 4,

		// RFC-2811 Channel Modes

		// RFC-2812 User Modes
		Away			= 1<<16, // a
		Invisible		= 1<<17, // i
		Operator		= 1<<18, // o
		LocalOperator		= 1<<19, // O
		Restricted		= 1<<20, // r
		RecieveServerNotice	= 1<<21, // s
		ReceiveWallOps		= 1<<22, // w

		// Common User Modes
		Voiced			= 1<<24, // v
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
}

#endif

