/*
    kircctcpplugin.cpp - IRC CTCP plugin handler.

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

#include "kircclientsocket.h"
#include "kirctransferhandler.h"

#include "kircevent.h"

#include <kdebug.h>
#include <klocale.h>

#include <QDateTime>
#include <qfileinfo.h>
#include <qregexp.h>

#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
/*
class CtcpPlugin::Private
{
public:
};
*/
using namespace KIrc;

CtcpPlugin::CtcpPlugin(QObject *parent)
    : QObject(parent)
    , d(0)
{
}

CtcpPlugin::~CtcpPlugin()
{
//	delete d;
}

/* Return true if the given string is a special command string
 * (i.e start and finish with the ascii code \001), and the given
 * string is splited to get the first part of the message and fill the ctcp command.
 * FIXME: The code currently only match for a textual message or a ctcp message not both mixed as it can be (even if very rare).
 */
/*
bool Message::extractCtcpCommand()
{
    if (d->suffix.isEmpty())
        return false;

    uint len = message.length();

    if (message[0] == 1 && message[len-1] == 1)
    {
        ctcpline = ctcpUnquote(unquote(message.mid(1,len-2)));
        message.truncate(0);

                msg.d->ctcpMessage = new Message(msg.d->engine);
                msg.d->ctcpMessage->d->raw = msg.d->ctcpRaw.latin1();

                int space = msg.d->ctcpRaw.find(' ');
                if (!matchForIRCRegExp(msg.d->ctcpMessage->d->raw, codec, *msg.d->ctcpMessage))
                {
                    if (space > 0)
                        msg.d->ctcpMessage->d->command = msg.d->ctcpRaw.mid(0, space).upper();
                    else
                        msg.d->ctcpMessage->d->command = msg.d->ctcpRaw.upper();
                }

                if (space > 0)
                    msg.d->ctcpMessage->d->ctcpRaw = msg.d->ctcpRaw.mid(space).latin1();

        return true;
    }
    return false;
}
*/

void CtcpPlugin::postEvent(const Message &msg, Message::Type messageType, const QString &message)
{
    Event event;
//	event.setMessageType(messageType);
//	event.setFrom(msg.prefix());
//	event.setCc(msg.socket().owner()); // server instead ?

    if (message.isEmpty()) {
        event.setText(msg.suffix());
    } else {
        event.setText(message);
    }

//	post the message here ... still dunno where, socket ?
}

void CtcpPlugin::postErrorEvent(const Message &msg, const QString &message)
{
//	postEvent(msg, Event::ErrorMessage, message);
}

void CtcpPlugin::postInfoEvent(const Message &msg, const QString &message)
{
    postEvent(msg, Event::InfoMessage, message);
}

void CtcpPlugin::postMOTDEvent(const Message &msg, const QString &message)
{
//	postEvent(msg, Event::MOTDMessage, message);
}

void CtcpPlugin::receivedServerMessage(Message msg)
{
    receivedServerMessage(msg, msg.suffix());
}

void CtcpPlugin::receivedServerMessage(Message msg, const QString &message)
{
//	emit receivedMessage(InfoMessage, msg.prefix(), Entity::List(), message);
}

#if 0
//#ifndef KIRC_STRICT

/*
void CtcpPlugin::bindCtcp()
{
    bindCtcpQuery("ACTION",		this, SLOT(CtcpQuery_action(Message&)),
        -1,	-1);
    bindCtcpQuery("CLIENTINFO",	this, SLOT(CtcpQuery_clientinfo(Message&)),
        -1,	1);
    bindCtcpQuery("DCC",		this, SLOT(CtcpQuery_dcc(Message&)),
        4,	5);
    bindCtcpQuery("FINGER",		this, SLOT(CtcpQuery_finger(Message&)),
        -1,	0);
    bindCtcpQuery("PING",		this, SLOT(CtcpQuery_ping(Message&)),
        1,	1);
    bindCtcpQuery("SOURCE",		this, SLOT(CtcpQuery_source(Message&)),
        -1,	0);
    bindCtcpQuery("TIME",		this, SLOT(CtcpQuery_time(Message&)),
        -1,	0);
    bindCtcpQuery("USERINFO",	this, SLOT(CtcpQuery_userinfo(Message&)),
        -1,	0);
    bindCtcpQuery("VERSION",	this, SLOT(CtcpQuery_version(Message&)),
        -1,	0);

    bindCtcpReply("ERRMSG",		this, SLOT(CtcpReply_errmsg(Message&)),
        1,	-1);
    bindCtcpReply("PING",		this, SLOT(CtcpReply_ping(Message&)),
        1,	1,	"");
    bindCtcpReply("VERSION",	this, SLOT(CtcpReply_version(Message&)),
        -1,	-1,	"");
}
*/

void CtcpPlugin::CtcpQuery_action(Message msg)
{
/*	QString target = msg.arg(0);
    if (target[0] == '#' || target[0] == '!' || target[0] == '&')
        emit incomingAction(target, msg, msg.ctcpMessage().ctcpRaw());
    else
        emit incomingPrivAction(msg, target, msg.ctcpMessage().ctcpRaw());*/
}

/*
NO REPLY EXIST FOR THE CTCP ACTION COMMAND !
bool CtcpPlugin::CtcpReply_action(Message msg)
{
}
*/

//	FIXME: the API can now answer to help commands.
void CtcpPlugin::CtcpQuery_clientinfo(Message msg)
{
    QString clientinfo = QString::fromLatin1("The following commands are supported, but "
                                             "without sub-command help: VERSION, CLIENTINFO, USERINFO, TIME, SOURCE, PING,"
                                             "ACTION.");

//	writeCtcpReplyMessage(	msg.prefix(), QString::null,
//				msg.ctcpMessage().command(), QString::null, clientinfo);
}

void CtcpPlugin::CtcpQuery_dcc(Message msg)
{
//	Message &ctcpMsg = msg.ctcpMessage();
    Message ctcpMsg;

    QString dccCommand = ctcpMsg.arg(0).upper();

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
            kDebug(14120) << "Starting DCC chat window." << endl;
//			TransferHandler::self()->createClient(
//				this, msg.prefix(),
//				address, port,
//				Transfer::Chat );
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
            kDebug(14120) << "Starting DCC send file transfert for file:" << ctcpMsg.arg(1) << endl;
//			TransferHandler::self()->createClient(
//				this, msg.prefix(),
//				address, port,
//				Transfer::FileIncoming,
//				ctcpMsg.arg(1), size );
        }
    }
//	else
//		((MessageRedirector *)sender())->error("Unknow dcc command");
}

/*
NO REPLY EXIST FOR THE CTCP DCC COMMAND !
bool CtcpPlugin::CtcpReply_dcc(Message msg)
{
}
*/

void CtcpPlugin::CtcpReply_errmsg(Message /*msg*/)
{
    // should emit one signal
}

void CtcpPlugin::CtcpQuery_finger(Message /*msg*/)
{
    // To be implemented
}

void CtcpPlugin::CtcpQuery_ping(Message msg)
{
//	writeCtcpReplyMessage(	msg.prefix(), QString::null,
//				msg.ctcpMessage().command(), msg.ctcpMessage().arg(0));
}

void CtcpPlugin::CtcpReply_ping(Message msg)
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

void CtcpPlugin::CtcpQuery_source(Message msg)
{
//	writeCtcpReplyMessage(msg.prefix(), QString::null,
//			      msg.ctcpMessage().command(), m_SourceString);
}

void CtcpPlugin::CtcpQuery_time(Message msg)
{
//	writeCtcpReplyMessage(msg.prefix(), QString::null,
//			      msg.ctcpMessage().command(), QDateTime::currentDateTime().toString(),
//			      QString::null, false);
}

void CtcpPlugin::CtcpQuery_userinfo(Message msg)
{
//	QString userinfo = m_UserString;

//	writeCtcpReplyMessage(msg.prefix(), QString::null,
//			      msg.ctcpMessage().command(), QString::null, userinfo);
}

void CtcpPlugin::CtcpQuery_version(Message msg)
{
//	QString response = m_VersionString;

//	writeCtcpReplyMessage(msg.prefix(),
//		msg.ctcpMessage().command() + " " + response);
}

void CtcpPlugin::CtcpReply_version(Message msg)
{
//	emit incomingCtcpReply(msg.ctcpMessage().command(), msg.prefix(), msg.ctcpMessage().ctcpRaw());
}

#endif
