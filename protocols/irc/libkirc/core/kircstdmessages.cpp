/*
    kircstdmessages.cpp - IRC Standard messages factory.

    Copyright (c) 2002      by Nick Betcher <nbetcher@kde.org>
    Copyright (c) 2003      by Jason Keirstead <jason@keirstead.org>
    Copyright (c) 2003-2006 by Michel Hermier <michel.hermier@gmail.com>

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

#ifdef HAVE_CONFIG
#include "config.h"
#endif

#include "kircstdmessages.h"

#include "kircmessage.h"
#include "kircsocket.h"

#include <kdebug.h>
#include <klocale.h>
#include <kuser.h>

#include <QDateTime>
#include <qfileinfo.h>
#include <qregexp.h>
/*
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#include <netinet/in.h>
#include <arpa/inet.h>
*/
using namespace KIrc;
#warning make usage of KUser (done!) and make more useful using some default strings (todo!)

Message StdMessages::away(const QByteArray &awayMessage)
{
	Message msg;
	msg.setCommand(AWAY);
	if (!awayMessage.isNull())
		msg.setSuffix(awayMessage);
	return msg;
}

Message StdMessages::ison(const QList<QByteArray> &nickList)
{
	Message msg;
	#warning FIXME bogus length check
/*
	if (!nickList.isEmpty())
	{
		QByteArray statement = ISON;
		for (QByteArrayList::ConstIterator it = nickList.begin(); it != nickList.end(); ++it)
		{
			if ((statement.length()+(*it).length())>509) // 512(max buf)-2("\r\n")-1(<space separator>)
			{
				writeMessage(statement);
				statement = QByteArray::fromLatin1("ISON ") +  (*it);
			}
			else
				statement.append(QChar(' ') + (*it));
		}
		writeMessage(statement);
	}
*/
	return msg;
}

Message StdMessages::join(const QByteArray &name, const QByteArray &key)
{
	Message msg;
	msg.setCommand(JOIN);
	msg.appendArg(name);
	if (!key.isEmpty())
		msg.appendArg(key);
	return msg;
}

Message StdMessages::kick(const QByteArray &user, const QByteArray &channel, const QByteArray &reason)
{
	Message msg;
	msg.setCommand(KICK);
	msg.appendArg(channel);
	msg.appendArg(user);
	msg.setSuffix(reason);
	return msg;
}

Message StdMessages::list()
{
	Message msg;
	msg.setCommand(LIST);
	return msg;
}

Message StdMessages::mode(const QByteArray &target, const QByteArray &mode)
{
	Message msg;
	msg.setCommand(MODE);
	msg.appendArg(target);
	msg.appendArg(mode);
	return msg;
}

Message StdMessages::motd(const QByteArray &server)
{
	Message msg;
	msg.setCommand(MOTD);
	if (!server.isNull())
		msg.appendArg(server);
	return msg;
}

KIrc::Message StdMessages::nick(const QByteArray &newNickName)
{
//	if (newNickName.isEmpty()) newNickName = KUser().loginName();

	Message msg;
	msg.setCommand(NICK);
	msg.appendArg(newNickName);
	return msg;
}

KIrc::Message StdMessages::notice(const QByteArray &target, const QByteArray &content)
{
	Message msg;
	msg.setCommand(NOTICE);
	msg.setArgs(target);
	msg.setSuffix(content/*, target->codec()*/);
	return msg;
}

/* This will part a channel with 'reason' as the reason for parting
 */
KIrc::Message StdMessages::part(const QByteArray &channel, const QByteArray &reason)
{
	Message msg;
	msg.setCommand(PART);
	msg.setArgs(channel);
	msg.setSuffix(reason/*, channel->codec()*/);
	return msg;
}

KIrc::Message StdMessages::pass(const QByteArray &password)
{
	Message msg;
	msg.setCommand(PASS);
	msg.setArgs(password);
	return msg;
}

KIrc::Message StdMessages::privmsg(const QByteArray &contact, const QByteArray &content)
{
	Message msg;
	msg.setCommand(PRIVMSG);
	msg.setArgs(contact);
	msg.setSuffix(content/*, contact->codec*/);
	return msg;
}

KIrc::Message StdMessages::quit(const QByteArray &reason)
{
	Message msg;
	msg.setCommand(QUIT);
	msg.setSuffix(reason);
	return msg;
}

Message StdMessages::topic(const QByteArray &channel, const QByteArray &topic)
{
	Message msg;
	msg.setCommand(TOPIC);
	msg.setArgs(channel);
	msg.setSuffix(topic/*, channel->codec*/);
	return msg;
}

/* RFC1459: "<username> <hostname> <servername> <realname>"
 * The USER command is used at the beginning of connection to specify
 * the username, hostname and realname of a new user.
 * hostname is usualy set to "127.0.0.1"
 */
Message StdMessages::user(const QByteArray &user, const QByteArray &hostName, const QByteArray &serverName, const QByteArray &realName)
{
//	if (user.isEmpty())     user     = KUser().loginName();
//	if (realName.isEmpty()) realName = KUser().fullName();

	Message msg;
	msg.setCommand(USER);
	msg.appendArg(user);
	msg.appendArg(hostName);
	msg.appendArg(serverName);
	msg.setSuffix(realName);
	return msg;
}

Message StdMessages::user(const QByteArray &user, UserMode mode, const QByteArray &realName)
{
//      if (user.isEmpty())     user     = KUser().loginName();
//      if (realName.isEmpty()) realName = KUser().fullName();

	Message msg;
	msg.setCommand(USER);
	msg.appendArg(user);
	msg.appendArg(QByteArray::number(mode));
	msg.appendArg(QChar('*')); // empty byte array instead ...
	msg.setSuffix(realName);
	return msg;
}

Message StdMessages::whois(const QByteArray &user)
{
	Message msg;
	msg.setCommand(WHOIS);
	msg.setArgs(user);
	return msg;
}

