/*
    kircsocket.h - IRC socket.

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

#ifndef KIRCSOCKET_H
#define KIRCSOCKET_H

#include "kirc_export.h"
#include "kircglobal.h"

#include <QtNetwork/QAbstractSocket>

namespace KIrc
{

class Context;
class Entity;
class Handler;
class Message;
class SocketPrivate;
class Handler;

/**
 * @author Michel Hermier <michel.hermier@gmail.com>
 */
class KIRC_EXPORT Socket
	: public QObject
{
	Q_OBJECT
	Q_DECLARE_PRIVATE(KIrc::Socket)

	Q_PROPERTY(ConnectionState connectionState READ connectionState)
//	Q_PROPERTY(KIrc::Entity *owner READ owner)
	Q_ENUMS(ConnectionState)

private:
	Q_DISABLE_COPY(Socket)

public:
	enum ConnectionState
	{
		Idle,
		HostLookup,
		HostFound,
//		Bound, // For server socket
		Connecting,
//		Open, // For server socket
		Authentifying,
		Authentified,
		Closing
	};

	~Socket();

public: // READ properties accessors.
	KIrc::Socket::ConnectionState connectionState() const;

	KIrc::EntityPtr owner() const;

public Q_SLOTS:
	void writeMessage(const Message &message);

	void close();

Q_SIGNALS:
	void connectionStateChanged(KIrc::Socket::ConnectionState newState);

	void receivedMessage(const KIrc::Message &message);

protected:
	Socket(KIrc::Context *context, KIrc::SocketPrivate *socketp);

	QAbstractSocket *socket();
	void setSocket(QAbstractSocket *socket);

	void setConnectionState(Socket::ConnectionState newstate);

protected Q_SLOTS:
	virtual void socketStateChanged(QAbstractSocket::SocketState newstate);

protected:
	KIrc::SocketPrivate * const d_ptr;
};

}

#endif

