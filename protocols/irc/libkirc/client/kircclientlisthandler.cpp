/*
    kircclientlisthandler.cpp - IRC Client List Handler

    Copyright (c) 2002      by Nick Betcher <nbetcher@kde.org>
    Copyright (c) 2003      by Jason Keirstead <jason@keirstead.org>
    Copyright (c) 2003-2008 by Michel Hermier <michel.hermier@wanadoo.fr>

    Kopete    (c) 2002-2008 by the Kopete developers <kopete-devel@kde.org>

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

#include "kirccontext.h"

#include <kdebug.h>
#include <klocale.h>

#include <QDateTime>
#include <qfileinfo.h>
#include <qregexp.h>

#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// For now lets define it to be empty
#define CHECK_ARGS(min, max)

class KIrc::ClientListHandlerPrivate
{
public:
	KIrc::Context *context;
};

using namespace KIrc;

ClientListHandler::ClientListHandler(Context *context)
	: Handler(context)
	, d_ptr(new ClientListHandlerPrivate)
{
	Q_D(ClientListHandler);

	d->context = context;
}

ClientListHandler::~ClientListHandler()
{
	delete d_ptr;
}

#if 0
void ClientListHandler::LIST(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	Q_D(ClientListHandler);

	CHECK_ARGS(0, 0);

#if 0
	MessageEvent *reply;
//	reply.setCommand(PONG);
//	reply.setArgs(msg.rawArg(0));
	reply.setSuffix(msg.rawSuffix());

//	msg->client->writeMessage(reply);
#endif
}
#endif

/* 321: "Channel :Users  Name"
 * RFC1459: Declared.
 * RFC2812: Obsoleted.
 */
void ClientListHandler::RPL_LISTSTART(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
//	emit (listbegin)
}

/* 322: "<channel> <# visible> :<topic>"
 * Received one channel from the LIST command.
 */
void ClientListHandler::RPL_LIST(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	CHECK_ARGS(2, 2);

//	emit incomingListedChan(msg.arg(1), msg.arg(2).toUInt(), msg.suffix());
}

/* 323: ":End of LIST"
 * End of the LIST command.
 */
void ClientListHandler::RPL_LISTEND(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	CHECK_ARGS(2, 2);

//	emit receivedServerMessage(msg);
}

/* 402: ""<server name> :No such server""
 * Gives a signal to indicate that the command issued failed because the person/channel not being on IRC.
 *  - Used to indicate the nickname parameter supplied to a command is currently unused.
 */
void ClientListHandler::ERR_NOSUCHSERVER(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	CHECK_ARGS(2, 2);

//	i18n("The server \"%1\" does not exist").arg(nick)
}
