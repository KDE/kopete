/*
    kircstdmessages.h - IRC Standard messages factory.

    Copyright (c) 2002      by Nick Betcher <nbetcher@kde.org>
    Copyright (c) 2003      by Jason Keirstead <jason@keirstead.org>
    Copyright (c) 2003-2006 by Michel Hermier <michel.hermier@wanadoo.fr>

    Kopete    (c) 2002-2006 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef KIRCSTDMESSAGES_H
#define KIRCSTDMESSAGES_H

#include "kircmessage.h"

#include <QFlags>
#include <QStringList>

namespace KIRC
{

class Socket;

/**
 * @author Nick Betcher <nbetcher@kde.org>
 * @author Michel Hermier <michel.hermier@wanadoo.fr>
 * @author Jason Keirstead <jason@keirstead.org>
 */
namespace StdMessages
{
	/* RFC2812: "<user> <mode> <unused> <realname>"
	 * mode is a numeric value (from a bit mask).
	 * 0x00 normal
	 * 0x04 request +w
	 * 0x08 request +i
	 */
	enum UserModes
	{
		Normal		= 0x00,
	//	W		= 0x04,
		Invisible	= 0x08
	};
	Q_DECLARE_FLAGS(UserMode, UserModes)

	KIRC::Message away(QString awayMessage = QString::null);

//	KIRC::Message invite();

	KIRC::Message ison(QStringList nickList);

	KIRC::Message join(QString name, QString key);

	KIRC::Message kick(QString user, QString channel, QString reason);

	KIRC::Message list();

	KIRC::Message mode(QString target, QString mode);

	KIRC::Message motd(QString server = QString::null);

	KIRC::Message nick(QString newNickname);

	KIRC::Message notice(QString target, QString message);

	KIRC::Message part(QString name, QString reason);

	KIRC::Message pass(QString password);

	KIRC::Message privmsg(QString contact, QString message);

	/**
	 * Send a quit message for the given reason.
	 * If now is set to true the connection is closed and no event message is sent.
	 * Therefore setting now to true should only be used while destroying the object.
	 */
	KIRC::Message quit(QString reason);

	KIRC::Message topic(QString channel, QString topic);

	KIRC::Message user(QString user, QString hostname, QString realName);

	KIRC::Message user(QString user, UserMode modes, QString realName);

	KIRC::Message who(QString mask/*, bool isOperator*/);

	KIRC::Message whois(QString target);

}

}

Q_DECLARE_OPERATORS_FOR_FLAGS(KIRC::StdMessages::UserMode)

#endif

