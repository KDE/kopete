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

#ifndef KIRCCLIENTWHOEVENTHANDLER_H
#define KIRCCLIENTWHOEVENTHANDLER_H

#include "kirceventhandler.h"
#include "kircmessage.h"

namespace KIrc
{

class ClientWhoEventPrivate;

/**
 * @author Nick Betcher <nbetcher@kde.org>
 * @author Michel Hermier <michel.hermier@wanadoo.fr>
 * @author Jason Keirstead <jason@keirstead.org>
 */
class KIRCCLIENT_EXPORT ClientWhoHandler
	: public KIrc::Handler
{
	Q_OBJECT
	Q_DECLARE_PRIVATE(KIrc::ClientWhoHandler)

public:
	explicit ClientWhoEventHandler(Context *context);
	~ClientWhoEventHandler();

private:
//	void numericReply_307(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
//	void numericReply_311(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
//	void numericReply_312(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
//	void numericReply_313(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
//	void numericReply_314(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
//	void numericReply_315(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
//	void numericReply_317(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
//	void numericReply_318(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);

private:
	Q_DISABLE_COPY(ClientWhoHandler)

	ClientWhoHandlerPrivate * const d_ptr;
};

}

#endif

