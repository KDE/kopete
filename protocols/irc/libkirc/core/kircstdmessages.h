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

	KIrc::Message away(const QString &awayMessage = QString::null);

//	KIrc::Message invite();

	KIrc::Message ison(const QStringList &nickList);

	KIrc::Message join(const QString &name, const QString &key = QString());

	KIrc::Message kick(const QString &user, const QString &channel, const QString &reason);

	KIrc::Message list();

	KIrc::Message mode(const QString &target, const QString &mode);

	KIrc::Message motd(const QString &server = QString::null);

	KIrc::Message nick(const QString &newNickname);

	KIrc::Message notice(const QString &target, const QString &message);

	KIrc::Message part(const QString &name, const QString &reason);

	KIrc::Message pass(const QString &password);

	KIrc::Message privmsg(const QString &contact, const QString &message);

	/**
	 * Send a quit message for the given reason.
	 * If now is set to true the connection is closed and no event message is sent.
	 * Therefore setting now to true should only be used while destroying the object.
	 */
	KIrc::Message quit(const QString &reason);

	KIrc::Message topic(const QString &channel, const QString &topic);

	KIrc::Message user(const QString &user, const QString &hostname, const QString &servername, const QString &realName);

	KIrc::Message user(const QString &user, UserMode modes, const QString &realName);

	KIrc::Message who(const QString &mask/*, bool isOperator*/);

	KIrc::Message whois(const QString &target);

}

}

Q_DECLARE_OPERATORS_FOR_FLAGS(KIrc::StdMessages::UserMode)

#endif

