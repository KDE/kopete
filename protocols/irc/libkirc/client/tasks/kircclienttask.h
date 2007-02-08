/*
    kircclienttask.h - IRC Client Task

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

#ifndef KIRCCLIENTTASK_H
#define KIRCCLIENTTASK_H

#include "tasks/kircsimpletask.h"

namespace KIrc
{

/**
 * @author Nick Betcher <nbetcher@kde.org>
 * @author Michel Hermier <michel.hermier@wanadoo.fr>
 * @author Jason Keirstead <jason@keirstead.org>
 */
class KIRCCLIENT_EXPORT ClientTask
	: public KIrc::SimpleTask
{
	Q_OBJECT

public:
	explicit ClientTask(QObject *parent = 0);
	~ClientTask();

private:
	void postErrorEvent(const KIrc::Message &msg, const QString &message = QString());
	void postInfoEvent(const KIrc::Message &msg, const QString &message = QString());
	void postMOTDEvent(const KIrc::Message &msg, const QString &message = QString());

	void receivedServerMessage(KIrc::Message msg) KDE_DEPRECATED; // emit the suffix of the message.
	void receivedServerMessage(KIrc::Message msg, const QString &message) KDE_DEPRECATED;

private:
	void error(KIrc::Message msg);
	void join(KIrc::Message msg);
	void kick(KIrc::Message msg);
	void mode(KIrc::Message msg);
	void nick(KIrc::Message msg);
	void notice(KIrc::Message msg);
	void part(KIrc::Message msg);
	void ping(KIrc::Message msg);
	void pong(KIrc::Message msg);
	void privmsg(KIrc::Message msg);
//	void squit(KIrc::Message msg);
	void quit(KIrc::Message msg);
	void topic(KIrc::Message msg);

	void numericReply_001(KIrc::Message msg);
	void numericReply_002(KIrc::Message msg);
	void numericReply_003(KIrc::Message msg);
	void numericReply_004(KIrc::Message msg);
	void numericReply_005(KIrc::Message msg);
	void numericReply_250(KIrc::Message msg);
	void numericReply_251(KIrc::Message msg);
	void numericReply_252(KIrc::Message msg);
	void numericReply_253(KIrc::Message msg);
	void numericReply_254(KIrc::Message msg);
	void numericReply_255(KIrc::Message msg);
	void numericReply_263(KIrc::Message msg);
	void numericReply_265(KIrc::Message msg);
	void numericReply_266(KIrc::Message msg);
	void numericReply_301(KIrc::Message msg);
	void numericReply_303(KIrc::Message msg);
	void numericReply_305(KIrc::Message msg);
	void numericReply_306(KIrc::Message msg);
	void numericReply_307(KIrc::Message msg);
	void numericReply_311(KIrc::Message msg);
	void numericReply_312(KIrc::Message msg);
	void numericReply_313(KIrc::Message msg);
	void numericReply_314(KIrc::Message msg);
	void numericReply_315(KIrc::Message msg);
	void numericReply_317(KIrc::Message msg);
	void numericReply_318(KIrc::Message msg);
	void numericReply_319(KIrc::Message msg);
	void numericReply_320(KIrc::Message msg);
	void numericReply_322(KIrc::Message msg);
	void numericReply_323(KIrc::Message msg);
	void numericReply_324(KIrc::Message msg);
	void numericReply_328(KIrc::Message msg);
	void numericReply_329(KIrc::Message msg);
	void numericReply_331(KIrc::Message msg);
	void numericReply_332(KIrc::Message msg);
	void numericReply_333(KIrc::Message msg);
	void numericReply_352(KIrc::Message msg);
	void numericReply_353(KIrc::Message msg);
	void numericReply_366(KIrc::Message msg);
	void numericReply_369(KIrc::Message msg);
	void numericReply_372(KIrc::Message msg);
	void numericReply_375(KIrc::Message msg);
	void numericReply_376(KIrc::Message msg);

	void numericReply_401(KIrc::Message msg);
	void numericReply_404(KIrc::Message msg);
	void numericReply_406(KIrc::Message msg);
	void numericReply_422(KIrc::Message msg);
	void numericReply_433(KIrc::Message msg);
	void numericReply_442(KIrc::Message msg);
	void numericReply_464(KIrc::Message msg);
	void numericReply_471(KIrc::Message msg);
	void numericReply_473(KIrc::Message msg);
	void numericReply_474(KIrc::Message msg);
	void numericReply_475(KIrc::Message msg);

#if 0
	void CtcpQuery_action(KIrc::Message msg);
	void CtcpQuery_clientinfo(KIrc::Message msg);
	void CtcpQuery_finger(KIrc::Message msg);
	void CtcpQuery_dcc(KIrc::Message msg);
	void CtcpQuery_ping(KIrc::Message msg);
	void CtcpQuery_source(KIrc::Message msg);
	void CtcpQuery_time(KIrc::Message msg);
	void CtcpQuery_userinfo(KIrc::Message msg);
	void CtcpQuery_version(KIrc::Message msg);

	void CtcpReply_errmsg(KIrc::Message msg);
	void CtcpReply_ping(KIrc::Message msg);
	void CtcpReply_version(KIrc::Message msg);
#endif

private:
	Q_DISABLE_COPY(ClientTask)

	class Private;
	Private * const d;
};

}

#endif

