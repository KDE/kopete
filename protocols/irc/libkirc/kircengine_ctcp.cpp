/*
    kirc_ctcp.h - IRC Client

    Copyright (c) 2003      by Michel Hermier <michel.hermier@wanadoo.fr>
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

#include "config.h"

#include "kircengine.h"
#include "kirctransferhandler.h"
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#include <netinet/in.h>
#include <arpa/inet.h>
#include <kextsock.h>

#include <qfileinfo.h>
#include <qregexp.h>

using namespace KIRC;

void Engine::bindCtcp()
{
	bindCtcpQuery("ACTION",		this, SLOT(CtcpQuery_action(KIRC::Message &)),
		-1,	-1);
	bindCtcpQuery("CLIENTINFO",	this, SLOT(CtcpQuery_clientinfo(KIRC::Message &)),
		-1,	1);
	bindCtcpQuery("DCC",		this, SLOT(CtcpQuery_dcc(KIRC::Message &)),
		4,	5);
	bindCtcpQuery("FINGER",		this, SLOT(CtcpQuery_finger(KIRC::Message &)),
		-1,	0);
	bindCtcpQuery("PING",		this, SLOT(CtcpQuery_ping(KIRC::Message &)),
		1,	1);
	bindCtcpQuery("SOURCE",		this, SLOT(CtcpQuery_source(KIRC::Message &)),
		-1,	0);
	bindCtcpQuery("TIME",		this, SLOT(CtcpQuery_time(KIRC::Message &)),
		-1,	0);
	bindCtcpQuery("USERINFO",	this, SLOT(CtcpQuery_userinfo(KIRC::Message &)),
		-1,	0);
	bindCtcpQuery("VERSION",	this, SLOT(CtcpQuery_version(KIRC::Message &)),
		-1,	0);

	bindCtcpReply("ERRMSG",		this, SLOT(CtcpReply_errmsg(KIRC::Message &)),
		1,	-1);
	bindCtcpReply("PING",		this, SLOT(CtcpReply_ping(KIRC::Message &)),
		1,	1,	"");
	bindCtcpReply("VERSION",	this, SLOT(CtcpReply_version(KIRC::Message &)),
		-1,	-1,	"");
}

// Normal order for a ctcp command:
// CtcpRequest_*
// CtcpQuery_*
// CtcpReply_* (if any)

/* Generic ctcp commnd for the /ctcp trigger */
void Engine::CtcpRequestCommand(const QString &contact, const QString &command)
{
	if(m_status == Connected)
	{
		writeCtcpQueryMessage(contact, QString::null, command);
//		emit ctcpCommandMessage( contact, command );
	}
}

void Engine::CtcpRequest_action(const QString &contact, const QString &message)
{
	if(m_status == Connected)
	{
		writeCtcpQueryMessage(contact, QString::null, "ACTION", message);

		if( Entity::isChannel(contact) )
			emit incomingAction(Kopete::Message::unescape(contact), Kopete::Message::unescape(m_Nickname), message);
		else
			emit incomingPrivAction(Kopete::Message::unescape(m_Nickname), Kopete::Message::unescape(contact), message);
	}
}

void Engine::CtcpQuery_action(Message &msg)
{
	QString target = msg.arg(0);
	if (target[0] == '#' || target[0] == '!' || target[0] == '&')
		emit incomingAction(target, msg.nickFromPrefix(), msg.ctcpMessage().ctcpRaw());
	else
		emit incomingPrivAction(msg.nickFromPrefix(), Kopete::Message::unescape(target), msg.ctcpMessage().ctcpRaw());
}

/*
NO REPLY EXIST FOR THE CTCP ACTION COMMAND !
bool Engine::CtcpReply_action(Message &msg)
{
}
*/

//	FIXME: the API can now answer to help commands.
void Engine::CtcpQuery_clientinfo(Message &msg)
{
	QString clientinfo = customCtcpMap[ QString::fromLatin1("clientinfo") ];

	if (clientinfo.isNull())
		clientinfo = QString::fromLatin1("The following commands are supported, but "
			"without sub-command help: VERSION, CLIENTINFO, USERINFO, TIME, SOURCE, PING,"
			"ACTION.");

	writeCtcpReplyMessage(	msg.nickFromPrefix(), QString::null,
				msg.ctcpMessage().command(), QString::null, clientinfo);
}

void Engine::CtcpRequest_dcc(const QString &nickname, const QString &fileName, uint port, Transfer::Type type)
{
	if(	m_status != Connected ||
		m_sock->localAddress() == 0 ||
		m_sock->localAddress()->nodeName().isNull())
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

void Engine::CtcpQuery_dcc(Message &msg)
{
	Message &ctcpMsg = msg.ctcpMessage();
	QString dccCommand = ctcpMsg.arg(0).upper();

	if (dccCommand == QString::fromLatin1("CHAT"))
	{
//		if(ctcpMsg.argsSize()!=4) return false;

		/* DCC CHAT type longip port
		 *
		 *  type   = Either Chat or Talk, but almost always Chat these days
		 *  longip = 32-bit Internet address of originator's machine
		 *  port   = Port on which the originator is waitng for a DCC chat
		 */
		bool okayHost, okayPort;
		// should ctctMsg.arg(1) be tested?
		QHostAddress address(ctcpMsg.arg(2).toUInt(&okayHost));
		unsigned int port = ctcpMsg.arg(3).toUInt(&okayPort);
		if (okayHost && okayPort)
		{
			kdDebug(14120) << "Starting DCC chat window." << endl;
			TransferHandler::self()->createClient(
				this, msg.nickFromPrefix(),
				address, port,
				Transfer::Chat );
		}
	}
	else if (dccCommand == QString::fromLatin1("SEND"))
	{
//		if(ctcpMsg.argsSize()!=5) return false;

		/* DCC SEND (filename) (longip) (port) (filesize)
		 *
		 *  filename = Name of file being sent
		 *  longip   = 32-bit Internet address of originator's machine
		 *  port     = Port on which the originator is waiitng for a DCC chat
		 *  filesize = Size of file being sent
		 */
		bool okayHost, okayPort, okaySize;
//		QFileInfo realfile(msg.arg(1));
		QHostAddress address(ctcpMsg.arg(2).toUInt(&okayHost));
		unsigned int port = ctcpMsg.arg(3).toUInt(&okayPort);
		unsigned int size = ctcpMsg.arg(4).toUInt(&okaySize);
		if (okayHost && okayPort && okaySize)
		{
			kdDebug(14120) << "Starting DCC send file transfert for file:" << ctcpMsg.arg(1) << endl;
			TransferHandler::self()->createClient(
				this, msg.nickFromPrefix(),
				address, port,
				Transfer::FileIncoming,
				ctcpMsg.arg(1), size );
		}
	}
//	else
//		((MessageRedirector *)sender())->error("Unknow dcc command");
}

/*
NO REPLY EXIST FOR THE CTCP DCC COMMAND !
bool Engine::CtcpReply_dcc(Message &msg)
{
}
*/

void Engine::CtcpReply_errmsg(Message &)
{
	// should emit one signal
}

void Engine::CtcpQuery_finger( Message &)
{
	// To be implemented
}

void Engine::CtcpRequest_ping(const QString &target)
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

void Engine::CtcpQuery_ping(Message &msg)
{
	writeCtcpReplyMessage(	msg.nickFromPrefix(), QString::null,
				msg.ctcpMessage().command(), msg.ctcpMessage().arg(0));
}

void Engine::CtcpReply_ping(Message &msg)
{
	timeval time;
	if (gettimeofday(&time, 0) == 0)
	{
		// FIXME: the time code is wrong for usec
		QString timeReply = QString::fromLatin1("%1.%2").arg(time.tv_sec).arg(time.tv_usec);
		double newTime = timeReply.toDouble();
		double oldTime = msg.suffix().section(' ',0, 0).toDouble();
		double difference = newTime - oldTime;
		QString diffString;

		if (difference < 1)
		{
			diffString = QString::number(difference);
			diffString.remove((diffString.find('.') -1), 2);
			diffString.truncate(3);
			diffString.append("milliseconds");
		}
		else
		{
			diffString = QString::number(difference);
			QString seconds = diffString.section('.', 0, 0);
			QString millSec = diffString.section('.', 1, 1);
			millSec.remove(millSec.find('.'), 1);
			millSec.truncate(3);
			diffString = QString::fromLatin1("%1 seconds, %2 milliseconds").arg(seconds).arg(millSec);
		}

		emit incomingCtcpReply(QString::fromLatin1("PING"), msg.nickFromPrefix(), diffString);
	}
//	else
//		((MessageRedirector *)sender())->error("failed to get current time");
}

void Engine::CtcpQuery_source(Message &msg)
{
	writeCtcpReplyMessage(msg.nickFromPrefix(), QString::null,
			      msg.ctcpMessage().command(), m_SourceString);
}

void Engine::CtcpQuery_time(Message &msg)
{
	writeCtcpReplyMessage(msg.nickFromPrefix(), QString::null,
			      msg.ctcpMessage().command(), QDateTime::currentDateTime().toString(),
			      QString::null, false);
}

void Engine::CtcpQuery_userinfo(Message &msg)
{
	QString userinfo = customCtcpMap[ QString::fromLatin1("userinfo") ];

	if (userinfo.isNull())
		userinfo = m_UserString;

	writeCtcpReplyMessage(msg.nickFromPrefix(), QString::null,
			      msg.ctcpMessage().command(), QString::null, userinfo);
}

void Engine::CtcpRequest_version(const QString &target)
{
	writeCtcpQueryMessage(target, QString::null, "VERSION");
}

void Engine::CtcpQuery_version(Message &msg)
{
	QString response = customCtcpMap[ QString::fromLatin1("version") ];
	kdDebug(14120) << "Version check: " << response << endl;

	if (response.isNull())
		response = m_VersionString;

	writeCtcpReplyMessage(msg.nickFromPrefix(),
		msg.ctcpMessage().command() + " " + response);
}

void Engine::CtcpReply_version(Message &msg)
{
	emit incomingCtcpReply(msg.ctcpMessage().command(), msg.nickFromPrefix(), msg.ctcpMessage().ctcpRaw());
}
