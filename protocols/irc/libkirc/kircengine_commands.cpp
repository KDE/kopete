/*
    kirc_commands.h - IRC Client

    Copyright (c) 2003-2004 by Michel Hermier <michel.hermier@wanadoo.fr>
    Copyright (c) 2002      by Nick Betcher <nbetcher@kde.org>
    Copyright (c) 2003      by Jason Keirstead <jason@keirstead.org>

    Kopete    (c) 2002-2003 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "kircengine.h"

#include <kdebug.h>

using namespace KIRC;

void Engine::registerStandardCommands(CommandManager *cm)
{
	cm->registerCommand(ERROR,	this, SLOT(error(KIRC::Message &))
		, 0, 0);

	bind(JOIN,	this, SLOT(join(KIRC::Message &)),	0, 1);

	bind(KICK,	this, SLOT(kick(KIRC::Message &)),	2, 2);

	bind(MODE,	this, SLOT(mode(KIRC::Message &)),	1, 1);

	bind(NICK,	this, SLOT(nick(KIRC::Message &)),	0, 0);

	bind(NOTICE,	this, SLOT(notice(KIRC::Message &)),	1, 1);

	bind(PART,	this, SLOT(part(KIRC::Message &)),	1, 1);

	bind(PING,	this, SLOT(ping(KIRC::Message &)),	0, 0);

	bind(PONG,	this, SLOT(pong(KIRC::Message &)),	0, 0);

	bind(PRIVMSG,	this, SLOT(privmsg(KIRC::Message &)),	1, 1);

	bind(QUIT,	this, SLOT(quit(KIRC::Message &)),	0, 0);

//	bind(SQUIT,	this, SLOT(squit(KIRC::Message &)),	1, 1);

	bind(TOPIC,	this, SLOT(topic(KIRC::Message &)),	1, 1);
}

void Engine::away(bool isAway, const QString &awayMessage)
{
	QString suffix;

	if (isAway)
	{
		if (!awayMessage.isEmpty())
			suffix = awayMessage;
		else
			suffix = QString::fromLatin1("I'm away.");
	}

	writeMessage(
		Message::format(
			AWAY,
			QByteArrayList(),
			suffix)
		);
}

// FIXME: Really handle this message
void Engine::error(Message &)
{
	close();
}

void Engine::ison(const QStringList &nickList)
{
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
}

void Engine::join(const QString &name, const QString &key)
{
	QStringList args(name);
	if (!key.isNull())
		args << key;

	writeMessage(
		Message::format(
			JOIN,
			args)
		);
}

/* RFC say: "( <channel> *( "," <channel> ) [ <key> *( "," <key> ) ] ) / "0""
 * suspected: ":<channel> *(" "/"," <channel>)"
 * assumed ":<channel>"
 * This is the response of someone joining a channel.
 * Remember that this will be emitted when *you* /join a room for the first time
 */
void Engine::join(Message &msg)
{
/*
	if (msg.argsSize()==1)
		emit incomingJoinedChannel(msg.arg(0), msg.prefix());
	else
		emit incomingJoinedChannel(msg.suffix(), msg.prefix());

	emit receivedMessage(
		JoinMessage,
		fromEntity,
		toEntity,
		i18n(""));
*/
}

void Engine::kick(const QString &user, const QString &channel, const QString &reason)
{
	writeMessage(
		Message::format(
			KICK,
			QStringList(channel) << user,
			reason)
		);
}

/* The given user is kicked.
 * "<channel> *( "," <channel> ) <user> *( "," <user> ) [<comment>]"
 */
void Engine::kick(Message &msg)
{
/*
	emit incomingKick(msg.arg(0), msg.prefix(), msg.arg(1), msg.suffix());
	emit receivedMessage(
		PartMessage,
		fromEntity,
		toEntity,
		i18n(""));
*/
}

void Engine::list()
{
	writeMessage(LIST);
}

void Engine::mode(const QString &target, const QString &mode)
{
	writeMessage(
		Message::format(
			MODE,
			QStringList(target) << mode)
		);
}

/* Change the mode of a user.
 * "<nickname> *( ( "+" / "-" ) *( "i" / "w" / "o" / "O" / "r" ) )"
 */
void Engine::mode(Message &msg)
{
	QStringList args = msg.argList();
	args.pop_front();
/*
	EntityPtr *fromEntity = msg.entityFromPrefix();
	EntityPtr *toEntity = msg.entityFromArg(0)

	emit receivedMessage(
		Info,
		fromEntity,
		KIRC::EntityPtrList::null,
		i18n(""));

	toEntity->setModes(args.join(" "));
*/
}

void Engine::motd(const QString &server)
{
/*
	writeMessage(
		Message::format(
			MOTD,
			server)
		);
*/
}

void Engine::nick(const QString &newNickname)
{
	m_PendingNick = newNickname;
/*
	writeMessage(
		Message::format(
			NICK,
			newNickname)
		);
*/
}

/* Nick name of a user changed
 * "<nickname>"
 */
void Engine::nick(Message &msg)
{
/*
	// FIXME: Find better i18n strings

	QString message;

	if (oldNick.lower() == m_Nickname.lower())
	{
		m_Nickname = msg.suffix();
		message = i18n("Your nick has changed from %1 to %2");
	}
	else
		message = i18n("User nick has changed from %1 to %2");

	emit receivedMessage(
		InfoMessage,
		msg.entityFromPrefix(),
		KIRC::EntityPtrList::null,
		message);

	fromEntity->rename();
*/
}

void Engine::notice(const QString &target, const QString &message)
{
	writeMessage(
		Message::format(
			NOTICE,
			target,
			message)
		// FIXME: target->codec()
		);
}

void Engine::notice(Message &msg)
{
	if (!msg.suffix().isEmpty())
	{
/*
		emit receivedMessage(
			NoticeMessage,
			msg.entityFromPrefix(),
			msg.entityFromArg(0), // shoul allways return myself
			msg.suffix()
		);
*/
	}

	if(msg.hasCtcpMessage())
		invokeCtcpCommandOfMessage(m_ctcpReplies, msg);
}

/* This will part a channel with 'reason' as the reason for parting
 */
void Engine::part(const QString &channel, const QString &reason)
{
	writeMessage(
		Message::format(
			PART,
			channel,
			reason)
		// FIXME: channel->codec()
		);
}

/* This signal emits when a user parts a channel
 * "<channel> *( "," <channel> ) [ <Part Message> ]"
 */
void Engine::part(Message &msg)
{
/*
	emit receivedMessage(
		PartMessage,
		msg.entityFromPrefix(),
		msg.entityFromArg(0),
		msg.suffix());
*/
}

void Engine::pass(const QString &password)
{
	writeMessage(
		Message::format(
			PASS,
			password)
		);
}

void Engine::ping(Message &msg)
{
	writeMessage(
		Message::format(
			PONG,
			msg.arg(0),
			msg.suffix())
		// FIXME: entityFromPrefix()->codec();
		);
}

void Engine::pong(Message &/*msg*/)
{
}

void Engine::privmsg(const QString &contact, const QString &message)
{
	writeMessage(
		Message::format(
			PRIVMSG,
			contact,
			message)
		// FIXME: contact->codec()
		);
}

void Engine::privmsg(Message &msg)
{
	if (!msg.suffix().isEmpty())
	{
/*
		emit receivedMessage(
			PrivateMessage,
			msg.entityFromPrefix(),
			msg.entityFromArg(0),
			msg.suffix());
*/
	}

	if (msg.hasCtcpMessage())
	{
		invokeCtcpCommandOfMessage(m_ctcpQueries, msg);
	}
}

void Engine::quit(const QString &reason, bool /*now*/)
{
	kdDebug(14120) << k_funcinfo << reason << endl;

//	if (isDisconnected())
//		return;

	writeMessage(
		Message::format(
			QUIT,
			QStringList(),
			reason)
		);

	close();
}

void Engine::quit(Message &msg)
{
/*
	emit receivedMessage(
		QuitMessage,
		msg.prefixEntity(),
		m_server,
		msg.suffix());
*/
}

void Engine::topic(const QString &channel, const QString &topic)
{
	writeMessage(
		Message::format(
			TOPIC,
			channel,
			topic)
		// FIXME: channel->codec();
		);
}

/* "<channel> [ <topic> ]"
 * The topic of a channel changed. emit the channel, new topic, and the person who changed it.
 */
void Engine::topic(Message &msg)
{
/*
	emit incomingTopicChange(msg.arg(0), msg.prefix(), msg.suffix());
	emit receivedMessage(
		QuitMessage,
		msg.prefixEntity(),
		m_server,
		msg.suffix());
*/
}

/* RFC1459: "<username> <hostname> <servername> <realname>"
 * The USER command is used at the beginning of connection to specify
 * the username, hostname and realname of a new user.
 * hostname is usualy set to "127.0.0.1"
 */
void Engine::user(const QString &newUserName, const QString &hostname, const QString &newRealName)
{
	m_Username = newUserName;
	m_realName = newRealName;

	writeMessage(
		Message::format(
			USER,
			QStringList(m_Username) << hostname << m_Host,
			m_realName)
		);
}

/* RFC2812: "<user> <mode> <unused> <realname>"
 * mode is a numeric value (from a bit mask).
 * 0x00 normal
 * 0x04 request +w
 * 0x08 request +i
 */
void Engine::user(const QString &newUserName, Q_UINT8 mode, const QString &newRealName)
{
	m_Username = newUserName;
	m_realName = newRealName;
/*
	writeMessage(
		Message::format(
			USER,
			QStringList(m_Username) << QString::number(mode) << QChar('*'),
			m_realName)
		);
*/
}

void Engine::whois(const QString &user)
{
	writeMessage(
		Message::format(
			WHOIS,
			user)
		);
}
