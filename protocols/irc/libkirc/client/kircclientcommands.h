/*
    kircclientcommands.h - IRC Client Commands

    Copyright (c) 2002      by Nick Betcher <nbetcher@kde.org>
    Copyright (c) 2003      by Jason Keirstead <jason@keirstead.org>
    Copyright (c) 2003-2007 by Michel Hermier <michel.hermier@gmail.com>

    Kopete    (c) 2002-2007 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef KIRCCLIENTCOMMANDS_H
#define KIRCCLIENTCOMMANDS_H

#include "kircplugin.h"

namespace KIrc
{

class Event;

/**
 * @author Nick Betcher <nbetcher@kde.org>
 * @author Michel Hermier <michel.hermier@gmail.com>
 * @author Jason Keirstead <jason@keirstead.org>
 */
class ClientCommands
	: public KIrc::Plugin
{
	Q_OBJECT

public:
	explicit ClientCommands(QObject *parent = 0);
	~ClientCommands();

private slots:
//	void postEvent(const KIrc::Message &msg, KIrc::Message::Type messageType, const QString &message);
	void postErrorEvent(const KIrc::Message &msg, const QString &message = QString());
	void postInfoEvent(const KIrc::Message &msg, const QString &message = QString());
	void postMOTDEvent(const KIrc::Message &msg, const QString &message = QString());

	void receivedServerMessage(const KIrc::Message &msg) KDE_DEPRECATED; // emit the suffix of the message.
	void receivedServerMessage(const KIrc::Message &msg, const QString &message) KDE_DEPRECATED;

private slots:
	void error(const KIrc::Message &msg);
	void join(const KIrc::Message &msg);
	void kick(const KIrc::Message &msg);
	void mode(const KIrc::Message &msg);
	void nick(const KIrc::Message &msg);
	void notice(const KIrc::Message &msg);
	void part(const KIrc::Message &msg);
	void ping(const KIrc::Message &msg);
	void pong(const KIrc::Message &msg);
	void privmsg(const KIrc::Message &msg);
//	void squit(const KIrc::Message &msg);
	void quit(const KIrc::Message &msg);
	void topic(const KIrc::Message &msg);

	void numericReply_001(const KIrc::Message &msg);
	void numericReply_002(const KIrc::Message &msg);
	void numericReply_003(const KIrc::Message &msg);
	void numericReply_004(const KIrc::Message &msg);
	void numericReply_005(const KIrc::Message &msg);
	void numericReply_250(const KIrc::Message &msg);
	void numericReply_251(const KIrc::Message &msg);
	void numericReply_252(const KIrc::Message &msg);
	void numericReply_253(const KIrc::Message &msg);
	void numericReply_254(const KIrc::Message &msg);
	void numericReply_255(const KIrc::Message &msg);
	void numericReply_263(const KIrc::Message &msg);
	void numericReply_265(const KIrc::Message &msg);
	void numericReply_266(const KIrc::Message &msg);
	void numericReply_301(const KIrc::Message &msg);
	void numericReply_303(const KIrc::Message &msg);
	void numericReply_305(const KIrc::Message &msg);
	void numericReply_306(const KIrc::Message &msg);
	void numericReply_307(const KIrc::Message &msg);
	void numericReply_311(const KIrc::Message &msg);
	void numericReply_312(const KIrc::Message &msg);
	void numericReply_313(const KIrc::Message &msg);
	void numericReply_314(const KIrc::Message &msg);
	void numericReply_315(const KIrc::Message &msg);
	void numericReply_317(const KIrc::Message &msg);
	void numericReply_318(const KIrc::Message &msg);
	void numericReply_319(const KIrc::Message &msg);
	void numericReply_320(const KIrc::Message &msg);
	void numericReply_322(const KIrc::Message &msg);
	void numericReply_323(const KIrc::Message &msg);
	void numericReply_324(const KIrc::Message &msg);
	void numericReply_328(const KIrc::Message &msg);
	void numericReply_329(const KIrc::Message &msg);
	void numericReply_331(const KIrc::Message &msg);
	void numericReply_332(const KIrc::Message &msg);
	void numericReply_333(const KIrc::Message &msg);
	void numericReply_352(const KIrc::Message &msg);
	void numericReply_353(const KIrc::Message &msg);
	void numericReply_366(const KIrc::Message &msg);
	void numericReply_369(const KIrc::Message &msg);
	void numericReply_372(const KIrc::Message &msg);
	void numericReply_375(const KIrc::Message &msg);
	void numericReply_376(const KIrc::Message &msg);

	void numericReply_401(const KIrc::Message &msg);
	void numericReply_404(const KIrc::Message &msg);
	void numericReply_406(const KIrc::Message &msg);
	void numericReply_422(const KIrc::Message &msg);
	void numericReply_433(const KIrc::Message &msg);
	void numericReply_442(const KIrc::Message &msg);
	void numericReply_464(const KIrc::Message &msg);
	void numericReply_471(const KIrc::Message &msg);
	void numericReply_473(const KIrc::Message &msg);
	void numericReply_474(const KIrc::Message &msg);
	void numericReply_475(const KIrc::Message &msg);

private:
	Q_DISABLE_COPY(ClientCommands)

	class Private;
	Private * const d;
};

}

#endif

