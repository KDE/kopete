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
	
	void bindNumericReplies();

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

	Handler::Handled RPL_MYINFO(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	Handler::Handled numericReply_320(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	Handler::Handled RPL_LIST(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	Handler::Handled RPL_LISTEND(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	Handler::Handled RPL_CHANNELMODEIS(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	Handler::Handled RPL_CHANNEL_URL(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	Handler::Handled RPL_CREATIONTIME(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	Handler::Handled RPL_NOTOPIC(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	Handler::Handled RPL_TOPIC(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	Handler::Handled RPL_TOPICWHOTIME(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	Handler::Handled RPL_NAMREPLY(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	Handler::Handled RPL_ENDOFNAMES(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);

	Handler::Handled ERR_NOSUCHNICK(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	Handler::Handled ERR_CANNOTSENDTOCHAN(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	Handler::Handled ERR_WASNOSUCHNICK(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	Handler::Handled ERR_NOTONCHANNEL(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	Handler::Handled ERR_CHANNELISFULL(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	Handler::Handled ERR_INVITEONLYCHAN(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	Handler::Handled ERR_BANNEDFROMCHAN(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	Handler::Handled ERR_BADCHANNELKEY(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);

//Commands

	Handler::Handled CMD_JOIN(KIrc::Context* context, const KIrc::Command &command, KIrc::Socket* socket);
	Handler::Handled CMD_PART(KIrc::Context* context, const KIrc::Command &command, KIrc::Socket* socket);
	Handler::Handled CMD_PRIVMSG(KIrc::Context* context, const KIrc::Command &command, KIrc::Socket* socket);
	Handler::Handled CMD_TOPIC(KIrc::Context* context, const KIrc::Command &command, KIrc::Socket* socket);

	Handler::Handled CMD_REJOIN(KIrc::Context* context, const KIrc::Command&command, KIrc::Socket* socket);
};

}

#endif

