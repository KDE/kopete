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

#include <kextsock.h>

#include <qtimer.h>

using namespace KIRC;

void Engine::bindCommands()
{
	bind("ERROR",	this, SLOT(error(KIRC::Message &)),	0, 0);
	bind("JOIN",	this, SLOT(join(KIRC::Message &)),	0, 1);
	bind("KICK",	this, SLOT(kick(KIRC::Message &)),	2, 2);
	bind("NICK",	this, SLOT(nick(KIRC::Message &)),	0, 0);
	bind("MODE",	this, SLOT(mode(KIRC::Message &)),	1, 1);
	bind("NOTICE",	this, SLOT(notice(KIRC::Message &)),	1, 1);
	bind("PART",	this, SLOT(part(KIRC::Message &)),	1, 1);
	bind("PING",	this, SLOT(ping(KIRC::Message &)),	0, 0);
	bind("PONG",	this, SLOT(pong(KIRC::Message &)),	0, 0);
	bind("PRIVMSG",	this, SLOT(privmsg(KIRC::Message &)),	1, 1);
	bind("QUIT",	this, SLOT(quit(KIRC::Message &)),	0, 0);
//	bind("SQUIT",	this, SLOT(squit(KIRC::Message &)),	1, 1);
	bind("TOPIC",	this, SLOT(topic(KIRC::Message &)),	1, 1);
}

void Engine::away(bool isAway, const QString &awayMessage)
{
	if(isAway)
		if( !awayMessage.isEmpty() )
			writeMessage("AWAY", QString::null, awayMessage);
	else
		writeMessage("AWAY", QString::null, QString::fromLatin1("I'm away."));
	else
		writeMessage("AWAY", QString::null);
}

// FIXME: Really handle this message
void Engine::error(Message &)
{
	setStatus(Closing);
}

void Engine::ison(const QStringList &nickList)
{
	if (!nickList.isEmpty())
	{
		QString statement = QString::fromLatin1("ISON");
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
}

void Engine::join(const QString &name, const QString &key)
{
	QStringList args(name);
	if ( !key.isNull() )
		args << key;

	writeMessage("JOIN", args);
}

void Engine::join(Message &msg)
{
	/* RFC say: "( <channel> *( "," <channel> ) [ <key> *( "," <key> ) ] ) / "0""
	* suspected: ":<channel> *(" "/"," <channel>)"
	* assumed ":<channel>"
	* This is the response of someone joining a channel.
	* Remember that this will be emitted when *you* /join a room for the first time */

	if (msg.argsSize()==1)
		emit incomingJoinedChannel(Kopete::Message::unescape(msg.arg(0)), msg.nickFromPrefix());
	else
		emit incomingJoinedChannel(Kopete::Message::unescape(msg.suffix()), msg.nickFromPrefix());
}

void Engine::kick(const QString &user, const QString &channel, const QString &reason)
{
	writeMessage("KICK", QStringList(channel) << user << reason);
}

void Engine::kick(Message &msg)
{
	/* The given user is kicked.
	* "<channel> *( "," <channel> ) <user> *( "," <user> ) [<comment>]"
	*/
	emit incomingKick( Kopete::Message::unescape(msg.arg(0)), msg.nickFromPrefix(), msg.arg(1), msg.suffix());
}

void Engine::mode(const QString &target, const QString &mode)
{
	writeMessage("MODE", QStringList(target) << mode);
}

void Engine::mode(Message &msg)
{
	/* Change the mode of a user.
	* "<nickname> *( ( "+" / "-" ) *( "i" / "w" / "o" / "O" / "r" ) )"
	*/
	QStringList args = msg.args();
	args.pop_front();
	if( Entity::isChannel( msg.arg(0) ) )
		emit incomingChannelModeChange( msg.arg(0), msg.nickFromPrefix(), args.join(" "));
	else
		emit incomingUserModeChange( msg.nickFromPrefix(), args.join(" "));
}

void Engine::nick(const QString &newNickname)
{
	m_PendingNick = newNickname;
	writeMessage("NICK", newNickname);
}

void Engine::nick(Message &msg)
{
	/* Nick name of a user changed
	 * "<nickname>" */
	QString oldNick = msg.prefix().section('!', 0, 0);
	QString newNick = msg.suffix();

	if( codecs[ oldNick ] )
	{
		QTextCodec *c = codecs[ oldNick ];
		codecs.remove( oldNick );
		codecs.insert( newNick, c );
	}

	if (oldNick.lower() == m_Nickname.lower())
	{
		emit successfullyChangedNick(oldNick, msg.suffix());
		m_Nickname = msg.suffix();
	}
	else
		emit incomingNickChange(oldNick, msg.suffix());
}

void Engine::part(const QString &channel, const QString &reason)
{
	/* This will part a channel with 'reason' as the reason for parting
	*/
	writeMessage("PART", channel, reason);
}

void Engine::part(Message &msg)
{
	/* This signal emits when a user parts a channel
	* "<channel> *( "," <channel> ) [ <Part Message> ]"
	*/
	kdDebug(14120) << "User parting" << endl;
	emit incomingPartedChannel(msg.arg(0), msg.nickFromPrefix(), msg.suffix());
}

void Engine::pass(const QString &password)
{
	writeMessage("PASS", password);
}

void Engine::ping(Message &msg)
{
	writeMessage("PONG", msg.arg(0), msg.suffix());
}

void Engine::pong(Message &/*msg*/)
{
}

void Engine::quit(const QString &reason, bool /*now*/)
{
	kdDebug(14120) << k_funcinfo << reason << endl;

	if (isDisconnected())
		return;

	if (isConnected())
		writeMessage("QUIT", QString::null, reason);

	setStatus(Closing);
}

void Engine::quit(Message &msg)
{
	/* This signal emits when a user quits irc.
	 */
	kdDebug(14120) << "User quiting" << endl;
	emit incomingQuitIRC(msg.prefix(), msg.suffix());
}

void Engine::user(const QString &newUserName, const QString &hostname, const QString &newRealName)
{
	/* RFC1459: "<username> <hostname> <servername> <realname>"
	* The USER command is used at the beginning of connection to specify
	* the username, hostname and realname of a new user.
	* hostname is usualy set to "127.0.0.1" */
	m_Username = newUserName;
	m_realName = newRealName;

	writeMessage("USER", QStringList(m_Username) << hostname << m_Host, m_realName);
}

void Engine::user(const QString &newUserName, Q_UINT8 mode, const QString &newRealName)
{
	/* RFC2812: "<user> <mode> <unused> <realname>"
	* mode is a numeric value (from a bit mask).
	* 0x00 normal
	* 0x04 request +w
	* 0x08 request +i */
	m_Username = newUserName;
	m_realName = newRealName;

	writeMessage("USER", QStringList(m_Username) << QString::number(mode) << QChar('*'), m_realName);
}

void Engine::topic(const QString &channel, const QString &topic)
{
	writeMessage("TOPIC", channel, topic);
}

void Engine::topic(Message &msg)
{
	/* The topic of a channel changed. emit the channel, new topic, and the person who changed it.
	 * "<channel> [ <topic> ]"
	 */
	emit incomingTopicChange(msg.arg(0), msg.nickFromPrefix(), msg.suffix());
}

void Engine::list()
{
	writeMessage("LIST", QString::null);
}

void Engine::motd(const QString &server)
{
	writeMessage("MOTD", server);
}

void Engine::privmsg(const QString &contact, const QString &message)
{
	writeMessage("PRIVMSG", contact, message, codecForNick( contact ) );
}

void Engine::privmsg(Message &msg)
{
	/* This is a signal that indicates there is a new message.
	 * This can be either from a channel or from a specific user. */
	Message m = msg;
	if (!m.suffix().isEmpty())
	{
		QString user = m.arg(0);
		QString message = m.suffix();
		const QTextCodec *codec = codecForNick( user );
		if (codec != defaultCodec) {
			m.decodeAgain( codec );
			message = m.suffix();
		}
		if (Entity::isChannel(user))
			emit incomingMessage(m.nickFromPrefix(), Kopete::Message::unescape(m.arg(0)), message );
		else
			emit incomingPrivMessage(m.nickFromPrefix(), Kopete::Message::unescape(m.arg(0)), message );
//		emit receivedMessage(PrivateMessage, msg.entityFrom(), msg.entityTo(), message);
	}

	if( m.hasCtcpMessage() )
	{
		invokeCtcpCommandOfMessage(m_ctcpQueries, m);
	}
}

void Engine::notice(const QString &target, const QString &message)
{
	writeMessage("NOTICE", target, message);
}

void Engine::notice(Message &msg)
{
	if(!msg.suffix().isEmpty())
		emit incomingNotice(msg.prefix(), msg.suffix());

	if(msg.hasCtcpMessage())
		invokeCtcpCommandOfMessage(m_ctcpReplies, msg);
}

void Engine::whois(const QString &user)
{
	writeMessage("WHOIS", user);
}
