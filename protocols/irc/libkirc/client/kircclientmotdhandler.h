/*
    kircclientmotdhandler.cpp - IRC Client Message Of The Day Handler

    Copyright (c) 2008      by Michel Hermier <michel.hermier@gmail.com>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef KIRCCLIENTMOTDHANDLER_H
#define KIRCCLIENTMOTDHANDLER_H

#include "kirchandler.h"

namespace KIrc
{

class ClientMotdHandlerPrivate;

class KIRCCLIENT_EXPORT ClientMotdHandler
	: public KIrc::Handler
{
	Q_OBJECT
	Q_DECLARE_PRIVATE(KIrc::ClientMotdHandler)

public:
	explicit ClientMotdHandler(QObject* parent=0);
	explicit ClientMotdHandler(KIrc::Handler *handler);
	~ClientMotdHandler();

private:
	void registerAliases();
#if 0
	void postEvent(QEvent *event);

	bool postEvent(MessageEvent *ev, const QByteArray &eventId, Entity::Ptr &from, QString &text = QString());
	bool postEvent(MessageEvent *ev, const QByteArray &eventId, Entity::Ptr &from, Entity::List &to, QString &text = QString());
	bool postEvent(MessageEvent *ev, const QByteArray &eventId, Entity::Ptr &from, Entity::List &to, Entity::List &victims, QString &text = QString());
#endif

	void receivedServerMessage(KIrc::Context *context, const KIrc::Message& msg, KIrc::Socket *socket);
private Q_SLOTS:
	// Handler::Handled MOTD(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);

	Handler::Handled RPL_MOTDSTART(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	Handler::Handled RPL_MOTD(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	Handler::Handled RPL_ENDOFMOTD(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);
	Handler::Handled ERR_NOMOTD(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket);

private:
	Q_DISABLE_COPY(ClientMotdHandler)
};

}

#endif

