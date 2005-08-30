/*
    kircsocket.h - IRC socket.

    Copyright (c) 2002      by Nick Betcher <nbetcher@kde.org>
    Copyright (c) 2003      by Jason Keirstead <jason@keirstead.org>
    Copyright (c) 2003-2005 by Michel Hermier <michel.hermier@wanadoo.fr>

    Kopete    (c) 2002-2005 by the Kopete developers <kopete-devel@kde.org>

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

#include "kircconst.h"
#include "kircmessage.h"

#include <kbufferedsocket.h>
#include <kresolver.h>

class QTextCodec;

namespace KIRC
{

class SocketPrivate;

/**
 * @author Nick Betcher <nbetcher@kde.org>
 * @author Michel Hermier <michel.hermier@wanadoo.fr>
 * @author Jason Keirstead <jason@keirstead.org>
 */
class Socket
	: public QObject
{
	Q_OBJECT

public:
	Socket(QObject *parent = 0);
	~Socket();

	QByteArray encode(const QString &str, bool *success, QTextCodec *codec = 0) const;

	QTextCodec *defaultCodec() const;

//	QString getHost() const;
//	Q_UINT16 getPort() const;

//	bool useSSL() const;

	KIRC::ConnectionState connectionState() const;

signals:
	/**
	 * This message is emitted each time an internal error is detected.
	 *
	 * @param errStr the string describing the error.
	 *
	 * @note The signal can be fired on non fatal error also.
	 *       It's the emiter responsability to change the state accordingly.
	 */
	void internalError(const QString &errStr);

	void connectionStateChanged(KIRC::ConnectionState newstate);

	void receivedMessage(KIRC::Message &message);

public slots:
	void setDefaultCodec(QTextCodec *codec);

	/**
	 * @return true if the socket is got no error trying to establish the connection.
	 */
	void connectToServer(const QString &host, Q_UINT16 port, bool useSSL);
	void connectToServer(const KNetwork::KResolverEntry &entry, bool useSSL);
//	void bind();
	void close();

	void writeMessage(const char *message);
	void writeMessage(const QByteArray &message);
	void writeMessage(const QString &message, QTextCodec *codec = 0);
//	void writeMessage(const KIRC::Message &message);

	void showInfoDialog();

protected:
	void setConnectionState(KIRC::ConnectionState newstate);

private slots:
	void slotReadyRead();

	void socketStateChanged(int newstate);

	void socketGotError(int code);

private:
	bool setupSocket(bool useSSL);

	SocketPrivate *d;
};

}

#endif
