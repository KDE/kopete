/*
    kircclientmotdhandler.cpp - IRC Client Message Of The Day Handler

    Copyright (c) 2008      by Michel Hermier <michel.hermier@gmail.com>

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

class KIrc::ClientMotdHandlerPrivate : public KIrc::HandlerPrivate
{
public:
    // FIXME: use a QMap instead so that we don't mix multiple MOTD when agregating
    QString motd;
};

// FIXME: MOTD can comes from other servers, with our current implementation
//        we say that MOTD come only from our server which is wrong

using namespace KIrc;

ClientMotdHandler::ClientMotdHandler(QObject *parent)
    : Handler(new ClientMotdHandlerPrivate, parent)
{
    registerAliases();
}

ClientMotdHandler::ClientMotdHandler(KIrc::Handler *parent)
    : Handler(new ClientMotdHandlerPrivate, parent)
{
    registerAliases();
}

ClientMotdHandler::~ClientMotdHandler()
{
}

void ClientMotdHandler::registerAliases()
{
    registerMessageAlias("375", "RPL_MOTDSTART");
    registerMessageAlias("372", "RPL_MOTD");
    registerMessageAlias("376", "RPL_ENDOFMOTD");

    registerMessageAlias("422", "ERR_NOMOTD");
}

void ClientMotdHandler::receivedServerMessage(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
    KIrc::ClientSocket *client = static_cast<KIrc::ClientSocket *>(socket);
    KIrc::TextEvent *event = new KIrc::TextEvent("ServerInfo", client->server(), client->owner(), message.suffix());
    context->postEvent(event);
}

/* 375: ":- <server> MessageMotd *of the day - "
 * Beginging the motd. This isn't emitted because the Motd is sent out line by line.
 */
KIrc::Handler::Handled ClientMotdHandler::RPL_MOTDSTART(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
    CHECK_ARGS(1, 1);
    Q_D(ClientMotdHandler);

    KIrc::ClientSocket *client = static_cast<KIrc::ClientSocket *>(socket);
    KIrc::TextEvent *event;

    d->motd.clear();

    event = new KIrc::TextEvent("MOTD_START", client->server(), client->owner(), message.suffix());
    context->postEvent(event);

    return KIrc::Handler::CoreHandled;
}

/* 372: ":- <text>"
 * Part of the Motd.
 */
KIrc::Handler::Handled ClientMotdHandler::RPL_MOTD(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
    CHECK_ARGS(1, 1);
    Q_D(ClientMotdHandler);

    KIrc::ClientSocket *client = static_cast<KIrc::ClientSocket *>(socket);
    KIrc::TextEvent *event;

    //remove the "- " in front.
    QByteArray text = message.suffix();
    if (text.startsWith("- ")) {
        text.remove(0, 2);
    }

    if (!d->motd.isEmpty()) {
        d->motd.append('\n');
    }
    d->motd.append(text);

    event = new KIrc::TextEvent("MOTD", client->server(), client->owner(), text);
    context->postEvent(event);

    return KIrc::Handler::CoreHandled;
}

/* 376: ":End of Motd command"
 * End of the motd.
 */
KIrc::Handler::Handled ClientMotdHandler::RPL_ENDOFMOTD(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
    CHECK_ARGS(1, 1);
    Q_D(ClientMotdHandler);

    KIrc::ClientSocket *client = static_cast<KIrc::ClientSocket *>(socket);
    KIrc::TextEvent *event;

    event = new KIrc::TextEvent("MOTD_FULL", client->server(), client->owner(), d->motd);
    context->postEvent(event);
    d->motd.clear();

    event = new KIrc::TextEvent("MOTD_END", client->server(), client->owner(), message.suffix());
    context->postEvent(event);

    return KIrc::Handler::CoreHandled;
}

/* 422: ":Motd File is missing"
 *
 * Server's Motd file could not be opened by the server.
 */
KIrc::Handler::Handled ClientMotdHandler::ERR_NOMOTD(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
    CHECK_ARGS(1, 1);

//	postErrorEvent(message);
    return KIrc::Handler::NotHandled;
}
