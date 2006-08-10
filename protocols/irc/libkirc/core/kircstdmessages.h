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

namespace KIrc
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

	KIrc::Message away(QString awayMessage = QString::null);

//	KIrc::Message invite();

	KIrc::Message ison(QStringList nickList);

	KIrc::Message join(QString name, QString key);

	KIrc::Message kick(QString user, QString channel, QString reason);

	KIrc::Message list();

	KIrc::Message mode(QString target, QString mode);

	KIrc::Message motd(QString server = QString::null);

	KIrc::Message nick(QString newNickname);

	KIrc::Message notice(QString target, QString message);

	KIrc::Message part(QString name, QString reason);

	KIrc::Message pass(QString password);

	KIrc::Message privmsg(QString contact, QString message);

	/**
	 * Send a quit message for the given reason.
	 * If now is set to true the connection is closed and no event message is sent.
	 * Therefore setting now to true should only be used while destroying the object.
	 */
	KIrc::Message quit(QString reason);

	KIrc::Message topic(QString channel, QString topic);

	KIrc::Message user(QString user, QString hostname, QString realName);

	KIrc::Message user(QString user, UserMode modes, QString realName);

	KIrc::Message who(QString mask/*, bool isOperator*/);

	KIrc::Message whois(QString target);

}

}

Q_DECLARE_OPERATORS_FOR_FLAGS(KIrc::StdMessages::UserMode)

#endif

