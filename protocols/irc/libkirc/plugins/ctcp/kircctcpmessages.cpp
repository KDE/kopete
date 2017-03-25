/*
    kircctcpmessages.cpp - IRC CTCP messages factory.

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

#include "kircctcpmessages.h"

#include "kircmessage.h"
#include "kircsocket.h"

#include <kdebug.h>
#include <klocale.h>
#include <kuser.h>

#include <QDateTime>
#include <qfileinfo.h>
#include <qregexp.h>

#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using namespace KIrc;
#ifdef __GNUC__
#warning make usage of KUser (done!) and make more useful using some default strings (todo!)
#endif

/*
void StdCommands::writeCtcpMessage(const QString &command, const QString &to, const QString &ctcpMessage, QTextCodec *codec)
{
#ifdef __GNUC__
    #warning FIXME CTCP MESSAGE NOT SENT
#endif
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
    }
}

void StdCommands::CtcpRequest_ping(const QString &target)
{
    kDebug(14120) ;

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
//		((MessageRedirector *)sender())->error("failed to get current time");
}

void StdCommands::CtcpRequest_version(const QString &target)
{
//	writeCtcpQueryMessage(target, QString(), "VERSION");
}
*/
