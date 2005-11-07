/*
    kircclientcommands.cpp - IRC Client Commands

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

#ifdef HAVE_CONFIG
#include "config.h"
#endif

#include "kircstdcommands.h"

#include "kircclient.h"
#include "kircmessage.h"
#include "kirctransferhandler.h"

#include <kdebug.h>
#include <klocale.h>

#include <QDateTime>
#include <qfileinfo.h>
#include <qregexp.h>

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#include <netinet/in.h>
#include <arpa/inet.h>

using namespace KIRC;
#warning make usage of KUser and make more usefull using some default strings.
/*
static QString getUsername()
{
	return QString::fromLatin1(getpwuid(getuid())->pw_name);
}

static QString getRealname()
{
	return QString::fromLatin1(getpwuid(getuid())->pw_gecos);
}
*/
void StdCommands::away(KIRC::Socket *socket, QString awayMessage)
{
	Q_ASSERT(socket);

	Message msg;
	msg.setCommand(AWAY);
	msg.setSuffix(awayMessage);

	socket->writeMessage(msg);
}

void StdCommands::ison(KIRC::Socket *socket, QStringList nickList)
{
	Q_ASSERT(socket);

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

void StdCommands::join(KIRC::Socket *socket, QString name, QString key)
{
	Q_ASSERT(socket);

	Message msg;
	msg.setCommand(JOIN);
	QStringList args(name);
	if (!key.isNull())
		args << key;
	msg.setArgList(args);

	socket->writeMessage(msg);
}

void StdCommands::kick(KIRC::Socket *socket, QString user, QString channel, QString reason)
{
	Q_ASSERT(socket);

	Message msg;
	msg.setCommand(KICK);
	msg.setArgList(QStringList(channel) << user);
	msg.setSuffix(reason);

	socket->writeMessage(msg);
}

void StdCommands::list(KIRC::Socket *socket)
{
	Q_ASSERT(socket);

	socket->writeMessage(LIST);
}

void StdCommands::mode(KIRC::Socket *socket, QString target, QString mode)
{
	Q_ASSERT(socket);

	Message msg;
	msg.setCommand(MODE);
	msg.setArgList(QStringList(target) << mode);

	socket->writeMessage(msg);
}

void StdCommands::motd(KIRC::Socket *socket, QString server)
{
	Q_ASSERT(socket);

	Message msg;
	msg.setCommand(MOTD);
	msg.setArgs(server);

	socket->writeMessage(msg);
}

void StdCommands::nick(KIRC::Socket *socket, QString newNickname)
{
	Q_ASSERT(socket);

//	m_PendingNick = newNickname;
	Message msg;
	msg.setCommand(NICK);
	msg.setArgs(newNickname);

	socket->writeMessage(msg);
}

void StdCommands::notice(KIRC::Socket *socket, QString target, QString content)
{
	Q_ASSERT(socket);

	Message msg;
	msg.setCommand(NOTICE);
	msg.setArgs(target);
	msg.setSuffix(content/*, target->codec()*/);

	socket->writeMessage(msg);
}

/* This will part a channel with 'reason' as the reason for parting
 */
void StdCommands::part(KIRC::Socket *socket, QString channel, QString reason)
{
	Q_ASSERT(socket);

	Message msg;
	msg.setCommand(PART);
	msg.setArgs(channel);
	msg.setSuffix(reason/*, channel->codec()*/);
	
	socket->writeMessage(msg);
}

void StdCommands::pass(KIRC::Socket *socket, QString password)
{
	Q_ASSERT(socket);

	Message msg;
	msg.setCommand(PASS);
	msg.setArgs(password);

	socket->writeMessage(msg);
}

void StdCommands::privmsg(KIRC::Socket *socket, QString contact, QString content)
{
	Q_ASSERT(socket);

	Message msg;
	msg.setCommand(PRIVMSG);
	msg.setArgs(contact);
	msg.setSuffix(content/*, contact->codec*/);

	socket->writeMessage(msg);
}

void StdCommands::quit(KIRC::Socket *socket, QString reason)
{
	Q_ASSERT(socket);

//	if (isDisconnected())
//		return;

	Message msg;
	msg.setCommand(QUIT);
	msg.setSuffix(reason);

	socket->writeMessage(msg);
	socket->close();
}

void StdCommands::topic(KIRC::Socket *socket, QString channel, QString topic)
{
	Q_ASSERT(socket);

	Message msg;
	msg.setCommand(TOPIC);
	msg.setArgs(channel);
	msg.setSuffix(topic/*, channel->codec*/);

	socket->writeMessage(msg);
}

/* RFC1459: "<username> <hostname> <servername> <realname>"
 * The USER command is used at the beginning of connection to specify
 * the username, hostname and realname of a new user.
 * hostname is usualy set to "127.0.0.1"
 */
void StdCommands::user(KIRC::Socket *socket, QString newUserName, QString hostName, QString newRealName)
{
	Q_ASSERT(socket);

//	m_Username = newUserName;
//	m_realName = newRealName;
//	Use KUser here

	Message msg;
	msg.setCommand(USER);
//	msg.setArgList(QStringList(newUserName) << hostName << serverName);
	msg.setSuffix(newRealName);

	socket->writeMessage(msg);
}

/* RFC2812: "<user> <mode> <unused> <realname>"
 * mode is a numeric value (from a bit mask).
 * 0x00 normal
 * 0x04 request +w
 * 0x08 request +i
 */
void StdCommands::user(KIRC::Socket *socket, QString newUserName, Modes modes, QString newRealName)
{
	Q_ASSERT(socket);

//	m_Username = newUserName;
//	m_realName = newRealName;
//	Use KUser here

	Message msg;
	msg.setCommand(USER);
	msg.setArgList(QStringList(newUserName) << QString::number(modes) << QString::fromLatin1("*"));
	msg.setSuffix(newRealName);

	socket->writeMessage(msg);
}

void StdCommands::whois(KIRC::Socket *socket, QString user)
{
	Message msg;
	msg.setCommand(WHOIS);
//	msg.setArgs(user);

	socket->writeMessage(msg);
}
/*
void StdCommands::writeCtcpMessage(const QString &command, const QString &to, const QString &ctcpMessage, QTextCodec *codec)
{
	#warning FIXME CTCP MESSAGE NOT SENT
//	writeRawMessage(command, to, ctcpMessage, codec);
}

void StdCommands::writeCtcpQueryMessage(const QString &to, const QString &ctcpQueryMessage, QTextCodec *codec)
{
	writeCtcpMessage("PRIVMSG", to, ctcpQueryMessage, codec);
}

void StdCommands::writeCtcpReplyMessage(const QString &to, const QString &ctcpReplyMessage, QTextCodec *codec)
{
	writeCtcpMessage("NOTICE", to, ctcpReplyMessage, codec);
}

void StdCommands::writeCtcpErrorMessage(const QString &to, const QString &ctcpLine, const QString &errorMsg, QTextCodec *codec)
{
	writeCtcpReplyMessage(to, "ERRMSG", ctcpLine, errorMsg);
}

// Normal order for a ctcp command:
// CtcpRequest_*
// CtcpQuery_*
// CtcpReply_* (if any)

void StdCommands::CtcpRequestCommand(const QString &contact, const QString &command)
{
//	writeCtcpQueryMessage(contact, Message::formatCtcp(command));
}

void StdCommands::CtcpRequest_action(const QString &contact, const QString &message)
{
//	writeCtcpQueryMessage(contact, Message::formatCtcp(QString::fromLatin1("ACTION %1").arg(message)));
}

void StdCommands::CtcpRequest_dcc(const QString &nickname, const QString &fileName, uint port, Transfer::Type type)
{
	if(	m_status != Connected ||
		m_socket->localAddress() == 0 ||
		m_socket->localAddress()->nodeName().isNull())
		return;

	switch(type)
	{
		case Transfer::Chat:
		{
			writeCtcpQueryMessage(nickname, QString::null,
				QString::fromLatin1("DCC"),
				QStringList(QString::fromLatin1("CHAT")) << QString::fromLatin1("chat") <<
					m_sock->localAddress()->nodeName() << QString::number(port)
			);
			break;
		}

		case Transfer::FileOutgoing:
		{
			QFileInfo file(fileName);
			QString noWhiteSpace = file.fileName();
			if (noWhiteSpace.contains(' ') > 0)
				noWhiteSpace.replace(QRegExp("\\s+"), "_");

			TransferServer *server = TransferHandler::self()->createServer(this, nickname, type, fileName, file.size());

			QString ip = m_sock->localAddress()->nodeName();
			QString ipNumber = QString::number( ntohl( inet_addr( ip.latin1() ) ) );

			kdDebug(14120) << "Starting DCC file outgoing transfer." << endl;

			writeCtcpQueryMessage(nickname, QString::null,
				QString::fromLatin1("DCC"),
				QStringList(QString::fromLatin1("SEND")) << noWhiteSpace << ipNumber <<
					QString::number(server->port()) << QString::number(file.size())
			);
			break;
		}

		case Transfer::FileIncoming:
		case Transfer::Unknown:
		default:
			break;
	}
}

void StdCommands::CtcpRequest_ping(const QString &target)
{
	kdDebug(14120) << k_funcinfo << endl;

	timeval time;
	if (gettimeofday(&time, 0) == 0)
	{
		QString timeReply;

		if( Entity::isChannel(target) )
			timeReply = QString::fromLatin1("%1.%2").arg(time.tv_sec).arg(time.tv_usec);
		else
		 	timeReply = QString::number( time.tv_sec );

		writeCtcpQueryMessage(	target, QString::null, "PING", timeReply);
	}
//	else
//		((MessageRedirector *)sender())->error("failed to get current time");
}

void StdCommands::CtcpRequest_version(const QString &target)
{
//	writeCtcpQueryMessage(target, QString::null, "VERSION");
}
*/
