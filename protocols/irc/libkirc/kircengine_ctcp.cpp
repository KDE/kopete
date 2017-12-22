/*
    kirc_ctcp.h - IRC Client

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

#include "kircengine.h"

#include "kirctransferhandler.h"

#include <kdebug.h>

#include <qfileinfo.h>
#include <qregexp.h>

#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using namespace KIRC;

void Engine::bindCtcp()
{
    bindCtcpQuery("ACTION", this, SLOT(CtcpQuery_action(KIRC::Message&)),
                  -1, -1);
    bindCtcpQuery("CLIENTINFO", this, SLOT(CtcpQuery_clientinfo(KIRC::Message&)),
                  -1, 1);
    bindCtcpQuery("DCC", this, SLOT(CtcpQuery_dcc(KIRC::Message&)),
                  4, 5);
    bindCtcpQuery("FINGER", this, SLOT(CtcpQuery_finger(KIRC::Message&)),
                  -1, 0);
    bindCtcpQuery("PING", this, SLOT(CtcpQuery_ping(KIRC::Message&)),
                  1, 1);
    bindCtcpQuery("SOURCE", this, SLOT(CtcpQuery_source(KIRC::Message&)),
                  -1, 0);
    bindCtcpQuery("TIME", this, SLOT(CtcpQuery_time(KIRC::Message&)),
                  -1, 0);
    bindCtcpQuery("USERINFO", this, SLOT(CtcpQuery_userinfo(KIRC::Message&)),
                  -1, 0);
    bindCtcpQuery("VERSION", this, SLOT(CtcpQuery_version(KIRC::Message&)),
                  -1, 0);

    bindCtcpReply("ERRMSG", this, SLOT(CtcpReply_errmsg(KIRC::Message&)),
                  1, -1);
    bindCtcpReply("PING", this, SLOT(CtcpReply_ping(KIRC::Message&)),
                  1, 1, "");
    bindCtcpReply("VERSION", this, SLOT(CtcpReply_version(KIRC::Message&)),
                  -1, -1, "");
}

void Engine::writeCtcpMessage(const QString &command, const QString &to, const QString &ctcpMessage, QTextCodec *codec)
{
#ifdef __GNUC__
    #warning FIXME CTCP MESSAGE NOT SENT
#endif
//	writeRawMessage(command, to, ctcpMessage, codec);
}

void Engine::writeCtcpQueryMessage(const QString &to, const QString &ctcpQueryMessage, QTextCodec *codec)
{
    writeCtcpMessage("PRIVMSG", to, ctcpQueryMessage, codec);
}

void Engine::writeCtcpReplyMessage(const QString &to, const QString &ctcpReplyMessage, QTextCodec *codec)
{
    writeCtcpMessage("NOTICE", to, ctcpReplyMessage, codec);
}

void Engine::writeCtcpErrorMessage(const QString &to, const QString &ctcpLine, const QString &errorMsg, QTextCodec *codec)
{
#ifdef __GNUC__
    #warning FIXME CTCP ERROR MESSAGE NOT SENT
#endif
//	writeCtcpReplyMessage(to, "ERRMSG", ctcpLine, errorMsg);
}

// Normal order for a ctcp command:
// CtcpRequest_*
// CtcpQuery_*
// CtcpReply_* (if any)

/* Generic ctcp commnd for the /ctcp trigger */
void Engine::CtcpRequestCommand(const QString &contact, const QString &command)
{
    writeCtcpQueryMessage(contact, Message::formatCtcp(command));
}

void Engine::CtcpRequest_action(const QString &contact, const QString &message)
{
    writeCtcpQueryMessage(contact, Message::formatCtcp(QString::fromLatin1("ACTION %1").arg(message)));
}

void Engine::CtcpQuery_action(Message &msg)
{
/*	QString target = msg.arg(0);
    if (target[0] == '#' || target[0] == '!' || target[0] == '&')
        emit incomingAction(target, msg, msg.ctcpMessage().ctcpRaw());
    else
        emit incomingPrivAction(msg, target, msg.ctcpMessage().ctcpRaw());*/
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
    QString clientinfo = QString::fromLatin1("The following commands are supported, but "
                                             "without sub-command help: VERSION, CLIENTINFO, USERINFO, TIME, SOURCE, PING,"
                                             "ACTION.");

//	writeCtcpReplyMessage(	msg.prefix(), QString(),
//				msg.ctcpMessage().command(), QString(), clientinfo);
}

void Engine::CtcpRequest_dcc(const QString &nickname, const QString &fileName, uint port, Transfer::Type type)
{
/*	if(	m_status != Connected ||
        m_socket->localAddress() == 0 ||
        m_socket->localAddress()->nodeName().isNull())
        return;

    switch(type)
    {
        case Transfer::Chat:
        {
            writeCtcpQueryMessage(nickname, QString(),
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
            QString ipNumber = QString::number( ntohl( inet_addr( ip.toLatin1() ) ) );

            kDebug(14120) << "Starting DCC file outgoing transfer.";

            writeCtcpQueryMessage(nickname, QString(),
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
    }*/
}

void Engine::CtcpQuery_dcc(Message &msg)
{
    Message &ctcpMsg = msg.ctcpMessage();
    QString dccCommand = ctcpMsg.arg(0).toUpper();

    if (dccCommand == QString::fromLatin1("CHAT")) {
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
        if (okayHost && okayPort) {
            kDebug(14120) << "Starting DCC chat window.";
            TransferHandler::self()->createClient(
                this, msg.prefix(),
                address, port,
                Transfer::Chat);
        }
    } else if (dccCommand == QString::fromLatin1("SEND")) {
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
        if (okayHost && okayPort && okaySize) {
            kDebug(14120) << "Starting DCC send file transfert for file:" << ctcpMsg.arg(1);
            TransferHandler::self()->createClient(
                this, msg.prefix(),
                address, port,
                Transfer::FileIncoming,
                ctcpMsg.arg(1), size);
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

void Engine::CtcpQuery_finger(Message &)
{
    // To be implemented
}

void Engine::CtcpRequest_ping(const QString &target)
{
    kDebug(14120);
/*
    timeval time;
    if (gettimeofday(&time, 0) == 0)
    {
        QString timeReply;

        if( Entity::isChannel(target) )
            timeReply = QString::fromLatin1("%1.%2").arg(time.tv_sec).arg(time.tv_usec);
        else
            timeReply = QString::number( time.tv_sec );

        writeCtcpQueryMessage(target, QString(), "PING", timeReply);
    }
//	else
//		((MessageRedirector *)sender())->error("failed to get current time");*/
}

void Engine::CtcpQuery_ping(Message &msg)
{
//	writeCtcpReplyMessage(	msg.prefix(), QString(),
//				msg.ctcpMessage().command(), msg.ctcpMessage().arg(0));
}

void Engine::CtcpReply_ping(Message &msg)
{
/*	timeval time;
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

        emit incomingCtcpReply(QString::fromLatin1("PING"), msg.prefix(), diffString);
    }
//	else
//		((MessageRedirector *)sender())->error("failed to get current time");*/
}

void Engine::CtcpQuery_source(Message &msg)
{
//	writeCtcpReplyMessage(msg.prefix(), QString(),
//			      msg.ctcpMessage().command(), m_SourceString);
}

void Engine::CtcpQuery_time(Message &msg)
{
//	writeCtcpReplyMessage(msg.prefix(), QString(),
//			      msg.ctcpMessage().command(), QDateTime::currentDateTime().toString(),
//			      QString(), false);
}

void Engine::CtcpQuery_userinfo(Message &msg)
{
    QString userinfo = m_UserString;

//	writeCtcpReplyMessage(msg.prefix(), QString(),
//			      msg.ctcpMessage().command(), QString(), userinfo);
}

void Engine::CtcpRequest_version(const QString &target)
{
//	writeCtcpQueryMessage(target, QString(), "VERSION");
}

void Engine::CtcpQuery_version(Message &msg)
{
    QString response = m_VersionString;

//	writeCtcpReplyMessage(msg.prefix(),
//		msg.ctcpMessage().command() + " " + response);
}

void Engine::CtcpReply_version(Message &msg)
{
//	emit incomingCtcpReply(msg.ctcpMessage().command(), msg.prefix(), msg.ctcpMessage().ctcpRaw());
}
