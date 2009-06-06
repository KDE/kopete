/*
    kircclientsocket_p.h - IRC Client Socket Private

    Copyright (c) 2002      by Nick Betcher <nbetcher@kde.org>
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

#ifndef KIRCCLIENTSOCKET_P_H
#define KIRCCLIENTSOCKET_P_H

#include "kircclientsocket.h"

#include "kircsocket_p.h"

class KIrc::ClientSocketPrivate
	: public KIrc::SocketPrivate
{
	Q_OBJECT
	Q_DECLARE_PUBLIC(KIrc::ClientSocket)

public:
	ClientSocketPrivate(ClientSocket *socket);

public:
	QUrl url;
	KIrc::EntityPtr server;

	bool failedNickOnLogin : 1;
};

#endif

