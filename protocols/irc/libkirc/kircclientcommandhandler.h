/*
    kircclientcommands.h - IRC Client Commands

    Copyright (c) 2002      by Nick Betcher <nbetcher@kde.org>
    Copyright (c) 2003      by Jason Keirstead <jason@keirstead.org>
    Copyright (c) 2003-2005 by Michel Hermier <michel.hermier@wanadoo.fr>

    Kopete    (c) 2002-2005 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef KIRCCLIENTCOMMANDHANDLER_H
#define KIRCCLIENTCOMMANDHANDLER_H

#include "kirccommandhandler.h"

namespace KIRC
{

class Message;

/**
 * @author Nick Betcher <nbetcher@kde.org>
 * @author Michel Hermier <michel.hermier@wanadoo.fr>
 * @author Jason Keirstead <jason@keirstead.org>
 */
class ClientCommandHandler
	: public CommandHandler
{
	Q_OBJECT

public:
	ClientCommandHandler(QObject *parent = 0);
	~ClientCommandHandler();

signals:
	/**
	 * Emit a received message.
	 * The received message could have been translated to your locale.
	 *
	 * @param type the message type.
	 * @param from the originator of the message.
	 * @param to is the list of entities that are related to this message.
	 * @param msg the message (usually translated).
	 *
	 * @note Most of the following numeric messages should be deprecated, and call this method instead.
	 *	 Most of the methods, using it, update KIRC::Entities.
	 *	 Lists based messages are sent via dedicated API, therefore they don't use this.
	 */
	// @param args the args to apply to this message.
	void receivedMessage(	KIRC::MessageType type,
				const KIRC::Entity::Ptr &from,
				const KIRC::Entity::List &to,
				const QString &msg);

private slots:

	void onConnectionStateChanged(KIRC::Socket::ConnectionState state);
	void onReceivedMessage( KIRC::Message &msg );

	void destroyed(KIRC::Entity *entity);

	void ignoreMessage(KIRC::Message &msg);
	void receivedServerMessage(KIRC::Message &msg); // emit the suffix of the message.
	void receivedServerMessage(KIRC::Message &msg, const QString &message);

	void error(KIRC::Message &msg);
	void join(KIRC::Message &msg);
	void kick(KIRC::Message &msg);
	void mode(KIRC::Message &msg);
	void nick(KIRC::Message &msg);
	void notice(KIRC::Message &msg);
	void part(KIRC::Message &msg);
	void ping(KIRC::Message &msg);
	void pong(KIRC::Message &msg);
	void privmsg(KIRC::Message &msg);
//	void squit(KIRC::Message &msg);
	void quit(KIRC::Message &msg);
	void topic(KIRC::Message &msg);

	void numericReply_001(KIRC::Message &msg);
	void numericReply_002(KIRC::Message &msg);
	void numericReply_003(KIRC::Message &msg);
	void numericReply_004(KIRC::Message &msg);
	void numericReply_005(KIRC::Message &msg);
	void numericReply_250(KIRC::Message &msg);
	void numericReply_251(KIRC::Message &msg);
	void numericReply_252(KIRC::Message &msg);
	void numericReply_253(KIRC::Message &msg);
	void numericReply_254(KIRC::Message &msg);
	void numericReply_255(KIRC::Message &msg);
	void numericReply_263(KIRC::Message &msg);
	void numericReply_265(KIRC::Message &msg);
	void numericReply_266(KIRC::Message &msg);
	void numericReply_301(KIRC::Message &msg);
	void numericReply_303(KIRC::Message &msg);
	void numericReply_305(KIRC::Message &msg);
	void numericReply_306(KIRC::Message &msg);
	void numericReply_307(KIRC::Message &msg);
	void numericReply_311(KIRC::Message &msg);
	void numericReply_312(KIRC::Message &msg);
	void numericReply_313(KIRC::Message &msg);
	void numericReply_314(KIRC::Message &msg);
	void numericReply_315(KIRC::Message &msg);
	void numericReply_317(KIRC::Message &msg);
	void numericReply_318(KIRC::Message &msg);
	void numericReply_319(KIRC::Message &msg);
	void numericReply_320(KIRC::Message &msg);
	void numericReply_322(KIRC::Message &msg);
	void numericReply_323(KIRC::Message &msg);
	void numericReply_324(KIRC::Message &msg);
	void numericReply_328(KIRC::Message &msg);
	void numericReply_329(KIRC::Message &msg);
	void numericReply_331(KIRC::Message &msg);
	void numericReply_332(KIRC::Message &msg);
	void numericReply_333(KIRC::Message &msg);
	void numericReply_352(KIRC::Message &msg);
	void numericReply_353(KIRC::Message &msg);
	void numericReply_366(KIRC::Message &msg);
	void numericReply_369(KIRC::Message &msg);
	void numericReply_372(KIRC::Message &msg);
	void numericReply_376(KIRC::Message &msg);

	void numericReply_401(KIRC::Message &msg);
	void numericReply_404(KIRC::Message &msg);
	void numericReply_406(KIRC::Message &msg);
	void numericReply_422(KIRC::Message &msg);
	void numericReply_433(KIRC::Message &msg);
	void numericReply_442(KIRC::Message &msg);
	void numericReply_464(KIRC::Message &msg);
	void numericReply_471(KIRC::Message &msg);
	void numericReply_473(KIRC::Message &msg);
	void numericReply_474(KIRC::Message &msg);
	void numericReply_475(KIRC::Message &msg);

	void CtcpQuery_action(KIRC::Message &msg);
	void CtcpQuery_clientinfo(KIRC::Message &msg);
	void CtcpQuery_finger(KIRC::Message &msg);
	void CtcpQuery_dcc(KIRC::Message &msg);
	void CtcpQuery_ping(KIRC::Message &msg);
	void CtcpQuery_source(KIRC::Message &msg);
	void CtcpQuery_time(KIRC::Message &msg);
	void CtcpQuery_userinfo(KIRC::Message &msg);
	void CtcpQuery_version(KIRC::Message &msg);

	void CtcpReply_errmsg(KIRC::Message &msg);
	void CtcpReply_ping(KIRC::Message &msg);
	void CtcpReply_version(KIRC::Message &msg);

private:
	Q_DISABLE_COPY(ClientCommandHandler)

	class Private;
	Private * const d;
};

}

#endif

