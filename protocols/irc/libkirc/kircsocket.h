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

#include "kircmessage.h"

#include <kbufferedsocket.h>
#include <kresolver.h>

#include <qregexp.h>
#include <qstring.h>
#include <qstringlist.h>

class QTextCodec;

namespace KIRC
{

/**
 * @author Nick Betcher <nbetcher@kde.org>
 * @author Michel Hermier <michel.hermier@wanadoo.fr>
 * @author Jason Keirstead <jason@keirstead.org>
 */
class Socket
	: public QObject
{
	Q_OBJECT

	Q_ENUMS(SocketState)

public:
	//Static regexes
	static const QRegExp m_RemoveLinefeeds;

	enum SocketState
	{
		Idle,
		Connecting,
		Authentifying,
		Connected,
		Closing
	};

	Socket(QObject *parent = 0);
	~Socket();

//	QString getHost() const;
//	Q_UINT16 getPort() const;

//	bool useSSL() const;

	KIRC::Socket::SocketState state() const
		{ return m_state; }

signals:
	/**
	 * This message is emitted each time an internal error is detected.
	 *
	 * @param errStr the string describing the error.
	 *
	 * @note The signal can be fired on non fatal error also.
	 *       It's the emiter responsability to change the status accordingly.
	 */
	void internalError(const QString &errStr);

	void stateChanged(KIRC::Socket::SocketState newstate);

	void receivedMessage(KIRC::Message &message);

public slots:
	/**
	 * @return true if the socket is got no error trying to establish the connection.
	 */
	void connectToServer(const QString &host, Q_UINT16 port, bool useSSL);
	void connectToServer(const KNetwork::KResolverEntry &entry, bool useSSL);
//	void bind();
	void close();

	void writeRawMessage(const QByteArray &rawMessage);
//	void writeRawMessage(const QString &message, QTextCodec *codec = 0);

	void showInfoDialog();

protected:
	bool setupSocket(bool useSSL);

protected slots:
	void slotReadyRead();

	void socketStateChanged(int newstate);

	void socketGotError(int code);

private:
	KNetwork::KBufferedSocket *m_socket;
	bool m_useSSL;

	KIRC::Socket::SocketState m_state;
};

}

#endif
