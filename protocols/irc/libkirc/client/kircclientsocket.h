/*
    kircclientsocket.h - IRC Client Socket

    Copyright (c) 2002      by Nick Betcher <nbetcher@kde.org>
    Copyright (c) 2003      by Jason Keirstead <jason@keirstead.org>
    Copyright (c) 2003-2008 by Michel Hermier <michel.hermier@gmail.com>

    Kopete    (c) 2002-2008 by the Kopete developers <kopete-devel@kde.org>

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

#include <QtCore/QUrl>

namespace KIrc
{

class ClientSocketPrivate;

/**
 * @author Nick Betcher <nbetcher@kde.org>
 * @author Michel Hermier <michel.hermier@gmail.com>
 * @author Jason Keirstead <jason@keirstead.org>
 */
class KIRCCLIENT_EXPORT ClientSocket
	: public KIrc::Socket
{
	Q_OBJECT
	Q_DECLARE_PRIVATE(ClientSocket)
	Q_PROPERTY(QUrl url READ url)
//	Q_PROPERTY(KIrc::Entity *server READ server)

private:
	Q_DISABLE_COPY(ClientSocket)

public:
	explicit ClientSocket(Context *context = 0);
	~ClientSocket();

public: // READ properties accessors.
	KIrc::EntityPtr server() const;

	QUrl url() const;

public Q_SLOTS: 
	void setAuthentified();
	KIrc::EntityPtr joinChannel(const QByteArray& channelName);

	virtual void connectToServer(const QUrl &url);
	void quit(const QByteArray& quitMessage);

protected:
	void connectToServer(const QUrl &url, QAbstractSocket *socket);

protected Q_SLOTS:
	void socketStateChanged(QAbstractSocket::SocketState newstate);
};

}

#endif
