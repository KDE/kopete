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

public:
	explicit ClientEventHandler(Context *context,QObject* parent=0);
	~ClientEventHandler();


public Q_SLOTS:
	virtual KIrc::Handler::Handled onMessage(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);

private:
#if 0
	void postEvent(QEvent *event);

	bool postEvent(MessageEvent *ev, const QByteArray &eventId, Entity::Ptr &from, QString &text = QString());
	bool postEvent(MessageEvent *ev, const QByteArray &eventId, Entity::Ptr &from, Entity::List &to, QString &text = QString());
	bool postEvent(MessageEvent *ev, const QByteArray &eventId, Entity::Ptr &from, Entity::List &to, Entity::List &victims, QString &text = QString());
#endif

	void receivedServerMessage(const KIrc::Message& msg);
private:
	void error(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	void join(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	void kick(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	void mode(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	void nick(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	void notice(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	void part(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	void ping(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	void pong(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	void privmsg(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
//	void squit(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	void quit(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	void topic(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);

	void numericReply_001(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	void numericReply_002(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	void numericReply_003(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	void numericReply_004(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	void numericReply_005(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	void numericReply_250(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	void numericReply_251(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	void numericReply_252(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	void numericReply_253(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	void numericReply_254(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	void numericReply_255(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	void numericReply_263(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	void numericReply_265(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	void numericReply_266(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	void numericReply_301(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	void numericReply_303(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	void numericReply_305(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	void numericReply_306(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
//	void numericReply_307(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
//	void numericReply_311(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
//	void numericReply_312(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
//	void numericReply_313(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
//	void numericReply_314(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
//	void numericReply_315(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
//	void numericReply_317(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
//	void numericReply_318(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	void numericReply_319(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	void numericReply_320(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	void numericReply_322(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	void numericReply_323(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	void numericReply_324(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	void numericReply_328(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	void numericReply_329(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	void numericReply_331(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	void numericReply_332(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	void numericReply_333(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	void numericReply_352(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	void numericReply_353(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	void numericReply_366(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	void numericReply_369(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	void numericReply_372(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	void numericReply_375(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	void numericReply_376(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);

	void numericReply_401(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	void numericReply_404(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	void numericReply_406(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	void numericReply_422(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	void numericReply_433(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	void numericReply_442(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	void numericReply_464(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	void numericReply_471(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	void numericReply_473(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	void numericReply_474(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	void numericReply_475(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);

private:
	Q_DISABLE_COPY(ClientEventHandler)

	ClientEventHandlerPrivate * const d_ptr;
};

}

#endif

