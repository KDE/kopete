/*
    kircclientchannelhandler.h - IRC Client Handler

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

#ifndef KIRCCLIENTCHANNELHANDLER_H
#define KIRCCLIENTCHANNELHANDLER_H

#include "kirchandler.h"
#include "kircmessage.h"

namespace KIrc
{

class ClientChannelHandlerPrivate;

class KIRCCLIENT_EXPORT ClientChannelHandler
	: public KIrc::Handler
{
	Q_OBJECT
	Q_DECLARE_PRIVATE(KIrc::ClientChannelHandler)
	
private:
	Q_DISABLE_COPY(ClientChannelHandler)

public:
	explicit ClientChannelHandler(QObject* parent=0);
	explicit ClientChannelHandler(Handler* parent);
	~ClientChannelHandler();

	virtual KIrc::Command handledCommands();

private Q_SLOTS:
	Handler::Handled JOIN(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	Handler::Handled KICK(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	Handler::Handled MODE(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	Handler::Handled NICK(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	Handler::Handled NOTICE(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	Handler::Handled PART(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	Handler::Handled PRIVMSG(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
//	Handler::Handled SQUIT(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	Handler::Handled QUIT(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	Handler::Handled TOPIC(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);

	Handler::Handled numericReply_004(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	Handler::Handled numericReply_005(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	Handler::Handled numericReply_254(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	Handler::Handled numericReply_255(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	Handler::Handled numericReply_263(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	Handler::Handled numericReply_265(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	Handler::Handled numericReply_266(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	Handler::Handled numericReply_301(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	Handler::Handled numericReply_303(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	Handler::Handled numericReply_305(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	Handler::Handled numericReply_306(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
//	Handler::Handled numericReply_307(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
//	Handler::Handled numericReply_311(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
//	Handler::Handled numericReply_312(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
//	Handler::Handled numericReply_313(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
//	Handler::Handled numericReply_314(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
//	Handler::Handled numericReply_315(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
//	Handler::Handled numericReply_317(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
//	Handler::Handled numericReply_318(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	Handler::Handled numericReply_319(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	Handler::Handled numericReply_320(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	Handler::Handled numericReply_322(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	Handler::Handled numericReply_323(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	Handler::Handled numericReply_324(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	Handler::Handled numericReply_328(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	Handler::Handled numericReply_329(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	Handler::Handled numericReply_331(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	Handler::Handled numericReply_332(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	Handler::Handled numericReply_333(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	Handler::Handled numericReply_352(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	Handler::Handled numericReply_353(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	Handler::Handled numericReply_366(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	Handler::Handled numericReply_369(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);

	Handler::Handled numericReply_401(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	Handler::Handled numericReply_404(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	Handler::Handled numericReply_406(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	Handler::Handled numericReply_433(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	Handler::Handled numericReply_442(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	Handler::Handled numericReply_464(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	Handler::Handled numericReply_471(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	Handler::Handled numericReply_473(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	Handler::Handled numericReply_474(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	Handler::Handled numericReply_475(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);

//Commands

	Handler::Handled CMD_JOIN(KIrc::Context* context, const KIrc::Command &command, KIrc::Socket* socket);
	Handler::Handled CMD_PART(KIrc::Context* context, const KIrc::Command &command, KIrc::Socket* socket);
	Handler::Handled CMD_PRIVMSG(KIrc::Context* context, const KIrc::Command &command, KIrc::Socket* socket);
	Handler::Handled CMD_TOPIC(KIrc::Context* context, const KIrc::Command &command, KIrc::Socket* socket);

	Handler::Handled CMD_REJOIN(KIrc::Context* context, const KIrc::Command&command, KIrc::Socket* socket);
};

}

#endif

