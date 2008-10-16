/*
    kircsocket.cpp - IRC socket.

    Copyright (c) 2002      by Nick Betcher <nbetcher@kde.org>
    Copyright (c) 2003-2007 by Michel Hermier <michel.hermier@gmail.com>
    Copyright (c) 2006      by Tommi Rantala <tommi.rantala@cs.helsinki.fi>

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

#ifndef KIRCSOCKET_P_H
#define KIRCSOCKET_P_H

#include "kircsocket.h"

namespace KIrc
{

class Context;
class Handler;

class KIRC_EXPORT SocketPrivate
	: public QObject
{
	Q_OBJECT
	Q_DECLARE_PUBLIC(KIrc::Socket)

public:
	explicit SocketPrivate(KIrc::Socket *socket);

public Q_SLOTS:
	void socketGotError(QAbstractSocket::SocketError);
	void socketReadyRead();

public:
	KIrc::Socket *q_ptr;

	KIrc::Context *context;
	QAbstractSocket *socket;
	KIrc::Socket::ConnectionState state;
	KIrc::EntityPtr owner;
};

}

#endif

