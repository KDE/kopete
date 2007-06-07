/*
    kircclientsocket.h - IRC Client

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

#ifndef KIRCCLIENTSOCKET_H
#define KIRCCLIENTSOCKET_H

#include "kircsocket.h"

namespace KIrc
{

class ClientCommandHandler;

/**
 * @author Nick Betcher <nbetcher@kde.org>
 * @author Michel Hermier <michel.hermier@gmail.com>
 * @author Jason Keirstead <jason@keirstead.org>
 */
class KIRCCLIENT_EXPORT ClientSocket
	: public KIrc::Socket
{
	Q_OBJECT

public:
	explicit ClientSocket(QObject *parent = 0);
	~ClientSocket();

public: // READ properties accessors.

public slots: // WRITE properties accessors.

public:
	bool isDisconnected() const KDE_DEPRECATED;
	bool isConnected() const KDE_DEPRECATED;

	Entity::Ptr server();

	ClientCommandHandler *clientCommandHandler();

private slots:
	void onConnectionStateChanged(Socket::ConnectionState newState);

private:
	Q_DISABLE_COPY(ClientSocket)

	class Private;
	Private * const d;
};

}

#endif

