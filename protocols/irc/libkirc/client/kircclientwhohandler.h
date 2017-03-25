/*
    kircclientwhohandler.h - IRC Client Who Handler

    Copyright (c) 2008      by Michel Hermier <michel.hermier@wanadoo.fr>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef KIRCCLIENTWHOHANDLER_H
#define KIRCCLIENTWHOHANDLER_H

#include "kirchandler.h"
#include "kircmessage.h"

namespace KIrc {
class ClientWhoHandlerPrivate;

/**
 * @author Nick Betcher <nbetcher@kde.org>
 * @author Michel Hermier <michel.hermier@wanadoo.fr>
 * @author Jason Keirstead <jason@keirstead.org>
 */
class KIRCCLIENT_EXPORT ClientWhoHandler : public KIrc::Handler
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(KIrc::ClientWhoHandler)

public:
    explicit ClientWhoHandler(Context *context);
    ~ClientWhoHandler();

public:
    void WHO(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
    void WHOIS(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
    void WHOWAS(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);

private:
    /* WHO replies */
    void RPL_WHOREPLY(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);

    void RPL_ENDOFWHO(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);

    /* WHOIS replies */
    void RPL_AWAY(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
    void RPL_WHOISCHANNELS(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
    void RPL_WHOISIDLE(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
    void RPL_WHOISOPERATOR(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
    void RPL_WHOISUSER(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);

    void RPL_ENDOFWHOIS(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);

    /* WHOWAS replies */
    void RPL_WHOWASUSER(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);

    void RPL_ENDOFWHOWAS(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);

    void ERR_WASNOSUCHNICK(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);

    /* Common replies */
    void RPL_WHOISSERVER(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);

    void ERR_NONICKNAMEGIVEN(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
    void ERR_NOSUCHNICK(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
    void ERR_NOSUCHSERVER(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);

private:
    Q_DISABLE_COPY(ClientWhoHandler)

    ClientWhoHandlerPrivate * const d_ptr;
};
}

#endif
