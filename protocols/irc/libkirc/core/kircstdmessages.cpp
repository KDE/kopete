/*
    kircstdmessages.cpp - IRC Standard messages factory.

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

Message StdMessages::away(QString awayMessage)
{
	Message msg;
	msg.setCommand(AWAY);
	msg.setSuffix(awayMessage);
	return msg;
}

Message StdMessages::ison(QStringList nickList)
{
	kDebug(14120) << k_funcinfo << endl;

	#warning FIXME bogus length check
/*
	if (!nickList.isEmpty())
	{
		QString statement = ISON;
		for (QStringList::ConstIterator it = nickList.begin(); it != nickList.end(); ++it)
		{
			if ((statement.length()+(*it).length())>509) // 512(max buf)-2("\r\n")-1(<space separator>)
			{
				writeMessage(statement);
				statement = QString::fromLatin1("ISON ") +  (*it);
			}
			else
				statement.append(QChar(' ') + (*it));
		}
		writeMessage(statement);
	}
*/
	Message msg;
	return msg;
}

Message StdMessages::join(QString name, QString key)
{
	Message msg;
	msg.setCommand(JOIN);
	QStringList args(name);
	if (!key.isNull())
		args << key;
	msg.setArgs(args);
	return msg;
}

Message StdMessages::kick(QString user, QString channel, QString reason)
{
	Message msg;
	msg.setCommand(KICK);
	msg.setArgs(QStringList(channel) << user);
	msg.setSuffix(reason);
	return msg;
}

Message StdMessages::list()
{
	Message msg;
	msg.setCommand(LIST);
	return msg;
}

Message StdMessages::mode(QString target, QString mode)
{
	Message msg;
	msg.setCommand(MODE);
	msg.setArgs(QStringList(target) << mode);
	return msg;
}

Message StdMessages::motd(QString server)
{
	Message msg;
	msg.setCommand(MOTD);
	msg.setArgs(server);
	return msg;
}

KIrc::Message StdMessages::nick(QString newNickName)
{
	if (newNickName.isEmpty()) newNickName = KUser().loginName();

//	m_PendingNick = newNickname;
	Message msg;
	msg.setCommand(NICK);
	msg.setArgs(newNickName);
	return msg;
}

KIrc::Message StdMessages::notice(QString target, QString content)
{
	Message msg;
	msg.setCommand(NOTICE);
	msg.setArgs(target);
	msg.setSuffix(content/*, target->codec()*/);
	return msg;
}

/* This will part a channel with 'reason' as the reason for parting
 */
KIrc::Message StdMessages::part(QString channel, QString reason)
{
	Message msg;
	msg.setCommand(PART);
	msg.setArgs(channel);
	msg.setSuffix(reason/*, channel->codec()*/);
	return msg;
}

KIrc::Message StdMessages::pass(QString password)
{
	Message msg;
	msg.setCommand(PASS);
	msg.setArgs(password);
	return msg;
}

KIrc::Message StdMessages::privmsg(QString contact, QString content)
{
	Message msg;
	msg.setCommand(PRIVMSG);
	msg.setArgs(contact);
	msg.setSuffix(content/*, contact->codec*/);
	return msg;
}

KIrc::Message StdMessages::quit(QString reason)
{
	Message msg;
	msg.setCommand(QUIT);
	msg.setSuffix(reason);
	return msg;
}

Message StdMessages::topic(QString channel, QString topic)
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
Message StdMessages::user(QString user, QString hostName, QString realName)
{
	if (user.isEmpty())     user     = KUser().loginName();
	if (realName.isEmpty()) realName = KUser().fullName();

	Message msg;
	msg.setCommand(USER);
//	msg.setArgList(QStringList(user) << hostName << serverName);
	msg.setSuffix(realName);
	return msg;
}

Message StdMessages::user(QString user, UserMode mode, QString realName)
{
	if (user.isEmpty())     user     = KUser().loginName();
	if (realName.isEmpty()) realName = KUser().fullName();

	Message msg;
	msg.setCommand(USER);
	msg.setArgs(QStringList(user) << QString::number(mode) << QLatin1String("*"));
	msg.setSuffix(realName);
	return msg;
}

Message StdMessages::whois(QString user)
{
	Message msg;
	msg.setCommand(WHOIS);
//	msg.setArgs(user);
	return msg;
}

