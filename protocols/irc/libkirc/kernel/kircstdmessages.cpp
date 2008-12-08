/*
    kircstdmessages.cpp - IRC Standard messages factory.

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

#include "kircstdmessages.h"

#include "kircconst.h"
#include "kircmessage.h"
#include "kircsocket.h"

#include <klocale.h>
#include <kuser.h>

#include <QList>
/*
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
*/
using namespace KIrc;
#ifdef __GNUC__
#warning make usage of KUser (done!) and make more useful using some default strings (todo!)
#endif


static const QByteArray AWAY("AWAY");
static const QByteArray ERROR("ERROR");
static const QByteArray INVITE("INVITE");
static const QByteArray ISON("ISON");
static const QByteArray JOIN("JOIN");
static const QByteArray KICK("KICK");
static const QByteArray LIST("LIST");
static const QByteArray MODE("MODE");
static const QByteArray MOTD("MOTD");
static const QByteArray NICK("NICK");
static const QByteArray NOTICE("NOTICE");
static const QByteArray PART("PART");
static const QByteArray PASS("PASS");
static const QByteArray PING("PING");
static const QByteArray PONG("PONG");
static const QByteArray PRIVMSG("PRIVMSG");
static const QByteArray QUIT("QUIT");
static const QByteArray SQUIT("SQUIT");
static const QByteArray TOPIC("TOPIC");
static const QByteArray USER("USER");
static const QByteArray WHO("WHO");
static const QByteArray WHOIS("WHOIS");
static const QByteArray WHOWAS("WHOWAS");

Message StdMessages::away(const QByteArray &awayMessage)
{
	Message msg;
	msg << AWAY;
	if (!awayMessage.isNull())
		msg.setSuffix(awayMessage);
	return msg;
}

Message StdMessages::ison(const QList<QByteArray> &nickList)
{
	Message msg;
#ifdef __GNUC__
	#warning FIXME bogus length check
#endif
/*
	if (!nickList.isEmpty())
	{
		QByteArray statement = ISON;
		for (QList<ByteArray>::ConstIterator it = nickList.begin(); it != nickList.end(); ++it)
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
	msg << JOIN << name;
	if (!key.isEmpty())
		msg<<key;
	return msg;
}

Message StdMessages::kick(const QByteArray &user, const QByteArray &channel, const QByteArray &reason)
{
	Message msg;
	msg << KICK << channel << user;
	msg.setSuffix(reason);
	return msg;
}

Message StdMessages::list()
{
	Message msg;
	msg << LIST;
	return msg;
}

Message StdMessages::mode(const QByteArray &target, const QByteArray &mode)
{
	Message msg;
	msg << MODE << target << mode;
	return msg;
}

Message StdMessages::motd(const QByteArray &server)
{
	Message msg;
	msg << MOTD;
	if (!server.isNull())
		msg<<server;
	return msg;
}

KIrc::Message StdMessages::nick(const QByteArray &newNickName)
{
//	if (newNickName.isEmpty()) newNickName = KUser().loginName();

	Message msg;
	msg << NICK << newNickName;
	return msg;
}

KIrc::Message StdMessages::notice(const QByteArray &target, const QByteArray &content)
{
	Message msg;
	msg << NOTICE << target;
	msg.setSuffix(content);
	return msg;
}

/* This will part a channel with 'reason' as the reason for parting
 */
KIrc::Message StdMessages::part(const QByteArray &channel, const QByteArray &reason)
{
	Message msg;
	msg << PART << channel;
	msg.setSuffix(reason);
	return msg;
}

KIrc::Message StdMessages::pass(const QByteArray &password)
{
	Message msg;
	msg << PASS << password;
	return msg;
}

KIrc::Message StdMessages::privmsg(const QByteArray &contact, const QByteArray &content)
{
	Message msg;
	msg << PRIVMSG << contact;
	msg.setSuffix(content);
	return msg;
}

KIrc::Message StdMessages::quit(const QByteArray &reason)
{
	Message msg;
	msg << QUIT;
	msg.setSuffix(reason);
	return msg;
}

Message StdMessages::topic(const QByteArray &channel, const QByteArray &topic)
{
	Message msg;
	msg << TOPIC << channel;
	msg.setSuffix(topic);
	return msg;
}

Message StdMessages::topic(const QByteArray &channel )
{
	Message msg;
	msg<< TOPIC <<channel;
	return msg;
}

/* RFC1459: "<username> <hostname> <servername> <realname>"
 * The USER command is used at the beginning of connection to specify
 * the username, hostname and realname of a new user.
 * hostname is usually set to "127.0.0.1"
 */
Message StdMessages::user(const QByteArray &user, const QByteArray &hostName, const QByteArray &serverName, const QByteArray &realName)
{
//	if (user.isEmpty())     user     = KUser().loginName();
//	if (realName.isEmpty()) realName = KUser().fullName();

	Message msg;
	msg << USER <<user << hostName << serverName;
	msg.setSuffix(realName);
	return msg;
}

Message StdMessages::user(const QByteArray &user, UserMode mode, const QByteArray &realName)
{
//      if (user.isEmpty())     user     = KUser().loginName();
//      if (realName.isEmpty()) realName = KUser().fullName();

	Message msg;
	msg << USER << user << QByteArray::number(mode) << "*";
	msg.setSuffix(realName);
	return msg;
}

Message StdMessages::whois(const QByteArray &user)
{
	Message msg;
	msg << WHOIS << user;
	return msg;
}

