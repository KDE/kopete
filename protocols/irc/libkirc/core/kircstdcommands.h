/*
    kircclientcommands.h - IRC Client Commands

    Copyright (c) 2002      by Nick Betcher <nbetcher@kde.org>
    Copyright (c) 2003      by Jason Keirstead <jason@keirstead.org>
    Copyright (c) 2003-2005 by Michel Hermier <michel.hermier@wanadoo.fr>

    Kopete    (c) 2002-2005 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef KIRCSTDCOMMANDS_H
#define KIRCSTDCOMMANDS_H

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
namespace StdCommands
{
	/* RFC2812: "<user> <mode> <unused> <realname>"
	 * mode is a numeric value (from a bit mask).
	 * 0x00 normal
	 * 0x04 request +w
	 * 0x08 request +i
	 */
	typedef enum
	{
		Normal		= 0x00,
	//	W		= 0x04,
		Invisible	= 0x08
	} Mode;
	Q_DECLARE_FLAGS(Modes, Mode)

	void away(KIRC::Socket *socket, QString awayMessage = QString::null);
//	void invite(KIRC::Socket *socket);
	void ison(KIRC::Socket *socket, QStringList nickList);
	void join(KIRC::Socket *socket, QString name, QString key);
	void kick(KIRC::Socket *socket, QString user, QString channel, QString reason);
	void list(KIRC::Socket *socket);
	void mode(KIRC::Socket *socket, QString target, QString mode);
	void motd(KIRC::Socket *socket, QString server = QString::null);
	void nick(KIRC::Socket *socket, QString newNickname);
	void notice(KIRC::Socket *socket, QString target, QString message);
	void part(KIRC::Socket *socket, QString name, QString reason);
	void pass(KIRC::Socket *socket, QString password);
	void privmsg(KIRC::Socket *socket, QString contact, QString message);

	/**
	 * Send a quit message for the given reason.
	 * If now is set to true the connection is closed and no event message is sent.
	 * Therefore setting now to true should only be used while destroying the object.
	 */
	void quit(KIRC::Socket *socket, QString reason);

	void topic(KIRC::Socket *socket, QString channel, QString topic);
	void user(KIRC::Socket *socket, QString newUsername, QString hostname, QString newRealname);
	void user(KIRC::Socket *socket, QString newUsername, Modes modes, QString newRealname);
	void whois(KIRC::Socket *socket, QString user);


	/* CTCP commands */
	void CtcpRequestCommand(KIRC::Socket *socket, QString contact, QString command);
	void CtcpRequest_action(KIRC::Socket *socket, QString contact, QString message);
//	void CtcpRequest_dcc(KIRC::Socket *socket, QString, QString, unsigned int port, KIRC::Transfer::Type type);
	void CtcpRequest_ping(KIRC::Socket *socket, QString target);
	void CtcpRequest_version(KIRC::Socket *socket, QString target);
}

}

Q_DECLARE_OPERATORS_FOR_FLAGS(KIRC::StdCommands::Modes)

#endif

