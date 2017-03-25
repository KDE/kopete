/*
 *kircclientcommands.h - IRC Client Commands

 *Copyright (c) 2002      by Nick Betcher <nbetcher@kde.org>
 *Copyright (c) 2003      by Jason Keirstead <jason@keirstead.org>
 *Copyright (c) 2003-2006 by Michel Hermier <michel.hermier@wanadoo.fr>

 *Kopete    (c) 2002-2006 by the Kopete developers <kopete-devel@kde.org>

 *************************************************************************
 *                                                                       *
 * This program is free software; you can redistribute it and/or modify  *
 * it under the terms of the GNU General Public License as published by  *
 * the Free Software Foundation; either version 2 of the License, or     *
 * (at your option) any later version.                                   *
 *                                                                       *
 *************************************************************************
*/

#ifndef KIRCI18NTASK_H
#define KIRCI18NTASK_H

#include "kirctask.h"

namespace KIrc {
class Event;

/**
 * @author Nick Betcher <nbetcher@kde.org>
 * @author Jason Keirstead <jason@keirstead.org>
 * @author Michel Hermier <michel.hermier@wanadoo.fr>
 */
class KIRCCLIENT_EXPORT I18nTask : public KIrc::Task
{
    Q_OBJECT

public:
#warning Make singleton
    explicit I18nTask(QObject *parent = 0);
    ~I18nTask();

private slots:
    void postServerEvent(const KIrc::Event *msg, const QString &message = QString());

private slots:
    void error(KIrc::Event *e);
    void join(KIrc::Event *e);
    void kick(KIrc::Event *e);
    void mode(KIrc::Event *e);
    void nick(KIrc::Event *e);
    void notice(KIrc::Event *e);
    void part(KIrc::Event *e);
    void ping(KIrc::Event *e);
    void pong(KIrc::Event *e);
    void privmsg(KIrc::Event *e);
//	void squit(KIrc::Event *e);
    void quit(KIrc::Event *e);
    void topic(KIrc::Event *e);

    void numericReply_001(KIrc::Event *e);
    void numericReply_002(KIrc::Event *e);
    void numericReply_003(KIrc::Event *e);
    void numericReply_004(KIrc::Event *e);
    void numericReply_005(KIrc::Event *e);
    void numericReply_250(KIrc::Event *e);
    void numericReply_251(KIrc::Event *e);
    void numericReply_252(KIrc::Event *e);
    void numericReply_253(KIrc::Event *e);
    void numericReply_254(KIrc::Event *e);
    void numericReply_255(KIrc::Event *e);
    void numericReply_263(KIrc::Event *e);
    void numericReply_265(KIrc::Event *e);
    void numericReply_266(KIrc::Event *e);
    void numericReply_301(KIrc::Event *e);
    void numericReply_303(KIrc::Event *e);
    void numericReply_305(KIrc::Event *e);
    void numericReply_306(KIrc::Event *e);
    void numericReply_307(KIrc::Event *e);
    void numericReply_311(KIrc::Event *e);
    void numericReply_312(KIrc::Event *e);
    void numericReply_313(KIrc::Event *e);
    void numericReply_314(KIrc::Event *e);
    void numericReply_315(KIrc::Event *e);
    void numericReply_317(KIrc::Event *e);
    void numericReply_318(KIrc::Event *e);
    void numericReply_319(KIrc::Event *e);
    void numericReply_320(KIrc::Event *e);
    void numericReply_322(KIrc::Event *e);
    void numericReply_323(KIrc::Event *e);
    void numericReply_324(KIrc::Event *e);
    void numericReply_328(KIrc::Event *e);
    void numericReply_329(KIrc::Event *e);
    void numericReply_331(KIrc::Event *e);
    void numericReply_332(KIrc::Event *e);
    void numericReply_333(KIrc::Event *e);
    void numericReply_352(KIrc::Event *e);
    void numericReply_353(KIrc::Event *e);
    void numericReply_366(KIrc::Event *e);
    void numericReply_369(KIrc::Event *e);
    void numericReply_372(KIrc::Event *e);
    void numericReply_375(KIrc::Event *e);
    void numericReply_376(KIrc::Event *e);

    void numericReply_401(KIrc::Event *e);
    void numericReply_404(KIrc::Event *e);
    void numericReply_406(KIrc::Event *e);
    void numericReply_422(KIrc::Event *e);
    void numericReply_433(KIrc::Event *e);
    void numericReply_442(KIrc::Event *e);
    void numericReply_464(KIrc::Event *e);
    void numericReply_471(KIrc::Event *e);
    void numericReply_473(KIrc::Event *e);
    void numericReply_474(KIrc::Event *e);
    void numericReply_475(KIrc::Event *e);

//#ifndef KIRC_STRICT
#if 0
    void CtcpQuery_action(KIrc::Event *e);
    void CtcpQuery_clientinfo(KIrc::Event *e);
    void CtcpQuery_finger(KIrc::Event *e);
    void CtcpQuery_dcc(KIrc::Event *e);
    void CtcpQuery_ping(KIrc::Event *e);
    void CtcpQuery_source(KIrc::Event *e);
    void CtcpQuery_time(KIrc::Event *e);
    void CtcpQuery_userinfo(KIrc::Event *e);
    void CtcpQuery_version(KIrc::Event *e);

    void CtcpReply_errmsg(KIrc::Event *e);
    void CtcpReply_ping(KIrc::Event *e);
    void CtcpReply_version(KIrc::Event *e);
#endif // KIRC_STRICT

private:
    Q_DISABLE_COPY(I18nTask)

    class Private;
    Private *const d;
};
}

#endif
