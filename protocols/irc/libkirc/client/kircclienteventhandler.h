/*
    kircclienteventhandler.h - IRC Client Task

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

#ifndef KIRCCLIENTEVENTHANDLER_H
#define KIRCCLIENTEVENTHANDLER_H

#include "kirceventhandler.h"
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
	: public KIrc::EventHandler
{
	Q_OBJECT
	Q_DECLARE_PRIVATE(KIrc::ClientEventHandler)

public:
	explicit ClientEventHandler(Context *context);
	~ClientEventHandler();

private:
	void postEvent(QEvent *event);

	bool postEvent(MessageEvent *ev, const QByteArray &eventId, Entity::Ptr &from, QString &text = QString());
	bool postEvent(MessageEvent *ev, const QByteArray &eventId, Entity::Ptr &from, Entity::List &to, QString &text = QString());
	bool postEvent(MessageEvent *ev, const QByteArray &eventId, Entity::Ptr &from, Entity::List &to, Entity::List &victims, QString &text = QString());

private:
	void error(KIrc::MessageEvent *msg);
	void join(KIrc::MessageEvent *msg);
	void kick(KIrc::MessageEvent *msg);
	void mode(KIrc::MessageEvent *msg);
	void nick(KIrc::MessageEvent *msg);
	void notice(KIrc::MessageEvent *msg);
	void part(KIrc::MessageEvent *msg);
	void ping(KIrc::MessageEvent *msg);
	void pong(KIrc::MessageEvent *msg);
	void privmsg(KIrc::MessageEvent *msg);
//	void squit(KIrc::MessageEvent *msg);
	void quit(KIrc::MessageEvent *msg);
	void topic(KIrc::MessageEvent *msg);

	void numericReply_001(KIrc::MessageEvent *msg);
	void numericReply_002(KIrc::MessageEvent *msg);
	void numericReply_003(KIrc::MessageEvent *msg);
	void numericReply_004(KIrc::MessageEvent *msg);
	void numericReply_005(KIrc::MessageEvent *msg);
	void numericReply_250(KIrc::MessageEvent *msg);
	void numericReply_251(KIrc::MessageEvent *msg);
	void numericReply_252(KIrc::MessageEvent *msg);
	void numericReply_253(KIrc::MessageEvent *msg);
	void numericReply_254(KIrc::MessageEvent *msg);
	void numericReply_255(KIrc::MessageEvent *msg);
	void numericReply_263(KIrc::MessageEvent *msg);
	void numericReply_265(KIrc::MessageEvent *msg);
	void numericReply_266(KIrc::MessageEvent *msg);
	void numericReply_301(KIrc::MessageEvent *msg);
	void numericReply_303(KIrc::MessageEvent *msg);
	void numericReply_305(KIrc::MessageEvent *msg);
	void numericReply_306(KIrc::MessageEvent *msg);
	void numericReply_307(KIrc::MessageEvent *msg);
	void numericReply_311(KIrc::MessageEvent *msg);
	void numericReply_312(KIrc::MessageEvent *msg);
	void numericReply_313(KIrc::MessageEvent *msg);
	void numericReply_314(KIrc::MessageEvent *msg);
	void numericReply_315(KIrc::MessageEvent *msg);
	void numericReply_317(KIrc::MessageEvent *msg);
	void numericReply_318(KIrc::MessageEvent *msg);
	void numericReply_319(KIrc::MessageEvent *msg);
	void numericReply_320(KIrc::MessageEvent *msg);
	void numericReply_322(KIrc::MessageEvent *msg);
	void numericReply_323(KIrc::MessageEvent *msg);
	void numericReply_324(KIrc::MessageEvent *msg);
	void numericReply_328(KIrc::MessageEvent *msg);
	void numericReply_329(KIrc::MessageEvent *msg);
	void numericReply_331(KIrc::MessageEvent *msg);
	void numericReply_332(KIrc::MessageEvent *msg);
	void numericReply_333(KIrc::MessageEvent *msg);
	void numericReply_352(KIrc::MessageEvent *msg);
	void numericReply_353(KIrc::MessageEvent *msg);
	void numericReply_366(KIrc::MessageEvent *msg);
	void numericReply_369(KIrc::MessageEvent *msg);
	void numericReply_372(KIrc::MessageEvent *msg);
	void numericReply_375(KIrc::MessageEvent *msg);
	void numericReply_376(KIrc::MessageEvent *msg);

	void numericReply_401(KIrc::MessageEvent *msg);
	void numericReply_404(KIrc::MessageEvent *msg);
	void numericReply_406(KIrc::MessageEvent *msg);
	void numericReply_422(KIrc::MessageEvent *msg);
	void numericReply_433(KIrc::MessageEvent *msg);
	void numericReply_442(KIrc::MessageEvent *msg);
	void numericReply_464(KIrc::MessageEvent *msg);
	void numericReply_471(KIrc::MessageEvent *msg);
	void numericReply_473(KIrc::MessageEvent *msg);
	void numericReply_474(KIrc::MessageEvent *msg);
	void numericReply_475(KIrc::MessageEvent *msg);

private:
	Q_DISABLE_COPY(ClientEventHandler)

	ClientEventHandlerPrivate * const d_ptr;
};

}

#endif

