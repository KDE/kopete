/*
    kircclienteventhandler.cpp - IRC Client Message Handler

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

#include "kircclientsocket.h"

#include "kircclientmotdhandler.h"

#include "kirccontext.h"
#include "kircentity.h"
#include "kircevent.h"
#include "kirchandler_p.h"

#include <kdebug.h>
#include <KLocalizedString>

#include <QDateTime>
#include <qfileinfo.h>
#include <qregexp.h>

#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// For now lets define it to be empty
#define CHECK_ARGS(min, max)

class KIrc::ClientPingPongHandlerPrivate : public KIrc::HandlerPrivate
{
public:
};

using namespace KIrc;

ClientPingPongHandler::ClientPingPongHandler(QObject *parent)
    : Handler(new ClientPingPongHandlerPrivate, parent)
{
}

ClientPingPongHandler::~ClientPingPongHandler()
{
}

KIrc::Handler::Handled ClientPingPongHandler::PING(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
    Q_D(ClientPingPongHandler);

    CHECK_ARGS(0, 0);

#if 0
    MessageEvent *reply;
//	reply.setCommand(PONG);
//	reply.setArgs(message.rawArg(0));
    reply.setSuffix(message.rawSuffix());

//	message->client->writeMessage(reply);
#endif
    return NotHandled;
}

KIrc::Handler::Handled ClientPingPongHandler::PONG(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
    Q_D(ClientPingPongHandler);

    CHECK_ARGS(0, 0);

    return NotHandled;
}
