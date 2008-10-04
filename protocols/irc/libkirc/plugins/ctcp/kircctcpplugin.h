/*
    kircctcpplugin.h - IRC CTCP plugin handler.

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

#include "kircmessage.h"
#include "kircevent.h"

namespace KIrc
{

class Event;

/**
 * @author Nick Betcher <nbetcher@kde.org>
 * @author Michel Hermier <michel.hermier@gmail.com>
 * @author Jason Keirstead <jason@keirstead.org>
 */
class CtcpPlugin
	: public QObject
	, public KIrc::CommandHandlerInterface
	, public KIrc::MessageHandlerInterface
{
	Q_OBJECT
	Q_INTERFACES(Kirc::CommandHandlerInterface Kirc::MessageHandlerInterface)

public:
	explicit CtcpPlugin(QObject *parent = 0);
	~CtcpPlugin();

private slots:
	void postEvent(const KIrc::Message &msg, KIrc::Message::Type messageType, const QString &message);
	void postErrorEvent(const KIrc::Message &msg, const QString &message = QString());
	void postInfoEvent(const KIrc::Message &msg, const QString &message = QString());
	void postMOTDEvent(const KIrc::Message &msg, const QString &message = QString());

	void receivedServerMessage(KIrc::Message msg) KDE_DEPRECATED; // emit the suffix of the message.
	void receivedServerMessage(KIrc::Message msg, const QString &message) KDE_DEPRECATED;

private slots:

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
	Q_DISABLE_COPY(CtcpPlugin)

	class Private;
	Private * const d;
};

}

#endif

