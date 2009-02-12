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

class ClientEventHandlerPrivate;

/**
 * @author Nick Betcher <nbetcher@kde.org>
 * @author Michel Hermier <michel.hermier@wanadoo.fr>
 * @author Jason Keirstead <jason@keirstead.org>
 */
class KIRCCLIENT_EXPORT ClientEventHandler
	: public KIrc::Handler
{
	Q_OBJECT
	Q_DECLARE_PRIVATE(KIrc::ClientEventHandler)
	
private:
	Q_DISABLE_COPY(ClientEventHandler)

public:
	explicit ClientEventHandler(QObject* parent=0);
	~ClientEventHandler();

private:
	void bindNumericReplies();
public Q_SLOTS:
	virtual KIrc::Handler::Handled onMessage(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);

private Q_SLOTS:
	Handler::Handled receivedServerMessage(KIrc::Context *context, const KIrc::Message& msg, KIrc::Socket *socket);

	Handler::Handled ERROR(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	Handler::Handled MODE(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	Handler::Handled NICK(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);

	Handler::Handled RPL_WELCOME(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	Handler::Handled RPL_MYINFO(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	Handler::Handled RPL_ISUPPORT(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	Handler::Handled RPL_LUSEROP(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	Handler::Handled RPL_LUSERUNKNOWN(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	Handler::Handled RPL_LUSERCHANNELS(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	Handler::Handled RPL_TRYAGAIN(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	Handler::Handled RPL_AWAY(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	Handler::Handled RPL_ISON(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	Handler::Handled RPL_UNAWAY(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	Handler::Handled RPL_NOWAWAY(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
//	Handler::Handled numericReply_307(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
//	Handler::Handled numericReply_311(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
//	Handler::Handled numericReply_312(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
//	Handler::Handled numericReply_313(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
//	Handler::Handled numericReply_314(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
//	Handler::Handled numericReply_315(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
//	Handler::Handled numericReply_317(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
//	Handler::Handled numericReply_318(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);

	Handler::Handled ERR_NOSUCHNICK(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	Handler::Handled ERR_CANNOTSENDTOCHAN(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	Handler::Handled ERR_WASNOSUCHNICK(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	Handler::Handled ERR_NICKNAMEINUSE(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	Handler::Handled ERR_PASSWDMISMATCH(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
};

}

#endif

