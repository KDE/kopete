/*
    kircstdmessages.h - IRC Standard messages factory.

    Copyright (c) 2002      by Nick Betcher <nbetcher@kde.org>
    Copyright (c) 2003      by Jason Keirstead <jason@keirstead.org>
    Copyright (c) 2003-2007 by Michel Hermier <michel.hermier@gmail.com>

    Kopete    (c) 2002-2007 by the Kopete developers <kopete-devel@kde.org>

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

namespace KIrc
{

/**
 * @author Nick Betcher <nbetcher@kde.org>
 * @author Michel Hermier <michel.hermier@gmail.com>
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

	KIRC_EXPORT KIrc::Message away(const QByteArray &awayMessage = QByteArray());

//	KIrc::Message invite();

	KIRC_EXPORT KIrc::Message ison(const QList<QByteArray> &nickList);

	KIRC_EXPORT KIrc::Message join(const QByteArray &name, const QByteArray &key = QByteArray());

	KIRC_EXPORT KIrc::Message kick(const QByteArray &user, const QByteArray &channel, const QByteArray &reason);

	KIRC_EXPORT KIrc::Message list();

	KIRC_EXPORT KIrc::Message mode(const QByteArray &target, const QByteArray &mode);

	KIRC_EXPORT KIrc::Message motd(const QByteArray &server = QByteArray());

	KIRC_EXPORT KIrc::Message nick(const QByteArray &newNickname);

	KIRC_EXPORT KIrc::Message notice(const QByteArray &target, const QByteArray &message);

	KIRC_EXPORT KIrc::Message part(const QByteArray &name, const QByteArray &reason);

	KIRC_EXPORT KIrc::Message pass(const QByteArray &password);

	KIRC_EXPORT KIrc::Message privmsg(const QByteArray &contact, const QByteArray &message);

	/**
	 * Send a quit message for the given reason.
	 * If now is set to true the connection is closed and no event message is sent.
	 * Therefore setting now to true should only be used while destroying the object.
	 */
	KIRC_EXPORT KIrc::Message quit(const QByteArray &reason);

	KIrc::Message topic(const QByteArray &channel, const QByteArray &topic);

	KIRC_EXPORT KIrc::Message user(const QByteArray &user, const QByteArray &hostname, const QByteArray &servername, const QByteArray &realName);

	KIRC_EXPORT KIrc::Message user(const QByteArray &user, UserMode modes, const QByteArray &realName);

	KIRC_EXPORT KIrc::Message who(const QByteArray &mask/*, bool isOperator*/);

	KIRC_EXPORT KIrc::Message whois(const QByteArray &target);

}

}

Q_DECLARE_OPERATORS_FOR_FLAGS(KIrc::StdMessages::UserMode)

#endif

