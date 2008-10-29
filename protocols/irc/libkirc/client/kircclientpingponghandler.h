/*
    kircclienthandler.h - IRC Client Handler

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

#ifndef KIRCCLIENTEVENTHANDLER_H
#define KIRCCLIENTEVENTHANDLER_H

#include "kirchandler.h"
#include "kircmessage.h"

namespace KIrc
{

class ClientPingPongHandlerPrivate;

/**
 * @author Nick Betcher <nbetcher@kde.org>
 * @author Michel Hermier <michel.hermier@wanadoo.fr>
 * @author Jason Keirstead <jason@keirstead.org>
 */
class KIRCCLIENT_EXPORT ClientPingPongHandler
	: public KIrc::Handler
{
	Q_OBJECT
	Q_DECLARE_PRIVATE(KIrc::ClientPingPongHandler)
	
private:
	Q_DISABLE_COPY(ClientPingPongHandler)

public:
	explicit ClientPingPongHandler(QObject* parent=0);
	~ClientPingPongHandler();

private Q_SLOTS:
	Handler::Handled PING(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	Handler::Handled PONG(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);

};

}

#endif

