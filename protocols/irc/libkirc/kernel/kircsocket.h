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

#include "kircentity.h"
#include "kircevent.h"
#include "kircmessage.h"

#include <QtCore/QUrl>

class QAbstractSocket;

namespace KIrc
{

class Context;
class SocketPrivate;

/**
 * @author Michel Hermier <michel.hermier@gmail.com>
 */
class KIRC_EXPORT Socket
	: public QObject
{
	Q_OBJECT
	Q_DECLARE_PRIVATE(KIrc::Socket)

	Q_PROPERTY(ConnectionState connectionState READ connectionState)
	Q_PROPERTY(KIrc::Entity::Ptr owner READ owner)
	Q_ENUMS(ConnectionState)

protected:
	KIrc::SocketPrivate * const d_ptr;

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

	KIrc::Entity::Ptr owner() const;

public:

public slots:
	void writeMessage(const Message &message);

	void close();

signals:
	void connectionStateChanged(KIrc::Socket::ConnectionState newState);

	void receivedMessage(const KIrc::Message &message);

protected:
	Socket(KIrc::Context *context,
		KIrc::SocketPrivate *socketp,
		KIrc::Entity::Ptr owner = KIrc::Entity::Ptr());

	void setSocket(QAbstractSocket *socket);
	QAbstractSocket *socket();

private:
	Q_DISABLE_COPY(Socket)
};

}

#endif

