/*
    kircsocket.h - IRC socket.

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

class CommandHandler;
class Entity;
class EntityManager;

/**
 * @author Michel Hermier <michel.hermier@wanadoo.fr>
 */
class Socket
	: public QObject
{
	Q_OBJECT

	Q_PROPERTY(ConnectionState connectionState READ connectionState)
	Q_ENUMS(ConnectionState)

public:
	enum ConnectionState
	{
		Idle,
		HostLookup,
		HostFound,
//		Bound,
		Connecting,
		Authentifying,
		Open,
		Closing
	};

	Socket(QObject *parent = 0);
//	Socket(KIRC::Entity *owner, QObject *parent = 0);
	~Socket();

public: // READ properties accessors.
	ConnectionState connectionState() const;

//public slots: // WRITE properties accessors.

public:
	KNetwork::KStreamSocket *socket();

	QTextCodec *defaultCodec() const;

	KIRC::CommandHandler *commandHandler();
	KIRC::EntityManager *entityManager();
	KIRC::Entity *owner();

public slots:
	void setDefaultCodec(QTextCodec *codec);

	void setCommandHandler(KIRC::CommandHandler *newCommandHandler);
	void setEntityManager(KIRC::EntityManager *newEntityManager);
	void setOwner(KIRC::Entity *newOwner);

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
	void writeMessage(const KIRC::Message &message);

	void showInfoDialog();

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

	void connectionStateChanged(KIRC::Socket::ConnectionState newstate);

	void receivedMessage(KIRC::Message &message);

protected:
	void setConnectionState(KIRC::Socket::ConnectionState newstate);

private slots:
	void onReadyRead();

	void socketStateChanged(int newstate);

	void socketGotError(int code);

private:
	QByteArray encode(const QString &str, bool *success, QTextCodec *codec = 0) const;
	bool setupSocket(bool useSSL);

	Q_DISABLE_COPY(Socket)

	class Private;
	Private * const d;
};

}

#endif
