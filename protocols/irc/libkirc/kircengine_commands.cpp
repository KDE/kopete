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
#include "kircfunctors.h"

#include <kextsock.h>

#include <qtimer.h>

using namespace KIRC;

void Engine::registerCommands()
{
//	The following order is based on the RFC2812.

//	Connection Registration
	addIrcMethod("NICK",	&Engine::nickChange,	0,	0);
	addIrcMethod("QUIT",	new KIRCMethodFunctor_SS_PrefixSuffix<Engine>(this, &Engine::incomingQuitIRC,	0,	0));
//	addIrcMethod("SQUIT",	new KIRCMethodFunctor_SS_PrefixSuffix<Engine>(this, &Engine::incomingServerQuitIRC,	1,	1));

//	Channel operations
	addIrcMethod("JOIN",	&Engine::joinChannel,	0,	1);
	addIrcMethod("PART",	&Engine::partChannel,	1,	1);
	addIrcMethod("MODE",	&Engine::modeChange,	1,	1);
	addIrcMethod("TOPIC",	&Engine::topicChange,	1,	1);
	addIrcMethod("KICK",	&Engine::kick,		2,	2);

//	Sending messages
	addIrcMethod("PRIVMSG",	&Engine::privateMessage,	1,	1);
	addIrcMethod("NOTICE",	&Engine::notice,		1,	1);

//	Miscellaneous messages
	addIrcMethod("PING",	&Engine::ping,	0,	0);
	addIrcMethod("PONG",	&Engine::pong,	0,	0);
}

// Normal order for a command:
// Request_*
// Query_*
// Reply_* (if any)

void Engine::changeNickname(const QString &newNickname)
{
	m_PendingNick = newNickname;
	writeMessage("NICK", newNickname, QString::null, false);
}

bool Engine::nickChange(const Message &msg)
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

	return true;
}

void Engine::changeUser(const QString &newUsername, const QString &hostname, const QString &newRealname)
{
	/* RFC1459: "<username> <hostname> <servername> <realname>"
	 * The USER command is used at the beginning of connection to specify
	 * the username, hostname and realname of a new user.
	 * hostname is usualy set to "127.0.0.1" */
	m_Username = newUsername;
	m_Realname = newRealname;

	writeMessage("USER", join( m_Username, hostname, m_Host ), m_Realname, false);
}

void Engine::changeUser(const QString &newUsername, Q_UINT8 mode, const QString &newRealname)
{
	/* RFC2812: "<user> <mode> <unused> <realname>"
	 * mode is a numeric value (from a bit mask).
	 * 0x00 normal
	 * 0x04 request +w
	 * 0x08 request +i */
	m_Username = newUsername;
	m_Realname = newRealname;

	writeMessage("USER", join(m_Username, QString::number(mode), QChar('*') ), m_Realname, false);
}

void Engine::quitIRC(const QString &reason, bool now)
{
	kdDebug(14120) << k_funcinfo << reason << endl;

	if(isDisconnected())
		return;

	if( now || !canSend(true) )
	{
		setStatus(Disconnected);
		m_sock->close();
		m_sock->reset();
	}
	else
	{
		writeMessage("QUIT", QString::null, reason, false);
		setStatus(Closing);
		QTimer::singleShot(5000, this, SLOT(quitTimeout()));
	}
}

void Engine::quitTimeout()
{
	if(	m_sock->socketStatus() > KExtendedSocket::nothing &&
		m_sock->socketStatus() < KExtendedSocket::done &&
		m_status == Closing)
	{
		setStatus(Disconnected);
		m_sock->close();
		m_sock->reset();
	}
}

bool Engine::quitIRC(const Message &msg)
{
	/* This signal emits when a user quits irc.
	 */
	kdDebug(14120) << "User quiting" << endl;
	emit incomingQuitIRC(msg.prefix(), msg.suffix());
	return true;
}

void Engine::joinChannel(const QString &name, const QString &key)
{
	if ( !key.isNull() )
		writeMessage("JOIN", join( name, key ) );
	else
		writeMessage("JOIN", name);
}

bool Engine::joinChannel(const Message &msg)
{
	/* RFC say: "( <channel> *( "," <channel> ) [ <key> *( "," <key> ) ] ) / "0""
	 * suspected: ":<channel> *(" "/"," <channel>)"
	 * assumed ":<channel>"
	 * This is the response of someone joining a channel.
	 * Remember that this will be emitted when *you* /join a room for the first time */

	if (msg.argsSize()==1)
		emit incomingJoinedChannel(msg.arg(0), msg.nickFromPrefix());
	else
		emit incomingJoinedChannel(msg.suffix(), msg.nickFromPrefix());

	return true;
}

void Engine::partChannel(const QString &channel, const QString &reason)
{
	/* This will part a channel with 'reason' as the reason for parting
	 */
	writeMessage("PART", channel, reason);
}

bool Engine::partChannel(const Message &msg)
{
	/* This signal emits when a user parts a channel
	 * "<channel> *( "," <channel> ) [ <Part Message> ]"
	 */
	kdDebug(14120) << "User parting" << endl;
	emit incomingPartedChannel(msg.arg(0), msg.nickFromPrefix(), msg.suffix());
	return true;
}

void Engine::changeMode(const QString &target, const QString &mode)
{
	writeMessage("MODE", join( target, mode ) );
}

bool Engine::modeChange(const Message &msg)
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
	return true;
}

void Engine::setTopic(const QString &channel, const QString &topic)
{
	writeMessage("TOPIC", channel, topic);
}

bool Engine::topicChange(const Message &msg)
{
	/* The topic of a channel changed. emit the channel, new topic, and the person who changed it.
	 * "<channel> [ <topic> ]"
	 */
	emit incomingTopicChange(msg.arg(0), msg.nickFromPrefix(), msg.suffix());
	return true;
}

void Engine::list()
{
	writeMessage("LIST", QString::null);
}

void Engine::motd(const QString &server)
{
	writeMessage("MOTD", server);
}

void Engine::kickUser(const QString &user, const QString &channel, const QString &reason)
{
	writeMessage("KICK", join( channel, user, reason ) );
}

bool Engine::kick(const Message &msg)
{
	/* The given user is kicked.
	 * "<channel> *( "," <channel> ) <user> *( "," <user> ) [<comment>]"
	 */
	emit incomingKick( msg.arg(0), msg.nickFromPrefix(), msg.arg(1), msg.suffix());
	return true;
}

//void Engine::sendPrivateMessage(const QString &contact, const QString &message)
void Engine::messageContact(const QString &contact, const QString &message)
{
	writeMessage("PRIVMSG", contact, message);
}

bool Engine::privateMessage(const Message &msg)
{
	/* This is a signal that indicates there is a new message.
	 * This can be either from a channel or from a specific user. */
	Message m = msg;
	if (!m.suffix().isEmpty())
	{
		QString user = m.arg(0);
		QString message = m.suffix();

		if( Entity::isChannel(user) )
			emit incomingMessage(msg.nickFromPrefix(), msg.arg(0), message );
		else
			emit incomingPrivMessage(msg.nickFromPrefix(), msg.arg(0), message );
	}

	if( msg.hasCtcpMessage() )
	{
		invokeCtcpCommandOfMessage(msg, m_IrcCTCPQueryMethods);
	}

	return true;
}

void Engine::sendNotice(const QString &target, const QString &message)
{
	writeMessage("NOTICE", target, message);
}

bool Engine::notice(const Message &msg)
{
	if(!msg.suffix().isEmpty())
		emit incomingNotice(msg.prefix(), msg.suffix());

	if(msg.hasCtcpMessage())
		invokeCtcpCommandOfMessage(msg, m_IrcCTCPReplyMethods);

	return true;
}

void Engine::whoisUser(const QString &user)
{
	writeMessage("WHOIS", user);
}

bool Engine::ping(const Message &msg)
{
	writeMessage("PONG", msg.arg(0), msg.suffix(), false);
	return true;
}

bool Engine::pong(const Message &/*msg*/)
{
	return true;
}

void Engine::setAway(bool isAway, const QString &awayMessage)
{
	if(isAway)
		if( !awayMessage.isEmpty() )
			writeMessage("AWAY", QString::null, awayMessage);
		else
			writeMessage("AWAY", QString::null, QString::fromLatin1("I'm away."));
	else
		writeMessage("AWAY", QString::null);
}

void Engine::isOn(const QStringList &nickList)
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
