/*
    dcchandler.h - DCC Handler

    Copyright (c) 2002      by Nick Betcher <nbetcher@kde.org>

    Kopete    (c) 2002      by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef DCCHANDLER_H
#define DCCHANDLER_H

#include <qhostaddress.h>
#include <qsocket.h>

#include <qserversocket.h>

class QFile;
class QTextCodec;

class DCCClient : public QSocket
{
Q_OBJECT
public:
	enum Type {
		Chat = 0,
		File = 1
	};
	DCCClient(QHostAddress host, unsigned int port, unsigned int size, Type type);
	void dccAccept();
	void dccAccept(const QString &filename);
	void dccCancel();
	bool sendMessage(const QString &message);
	const QHostAddress ipaddress() { return mHost; };
	const unsigned int hostPort() { return mPort; };
signals:
	void incomingDccMessage(const QString &, bool fromMe);
	void terminating();
	void receiveAckPercent(int);
	void sendFinished();
private:
	QHostAddress mHost;
	unsigned int mPort;
	Type mType;
	unsigned int mSize;
	QFile *mFile; // Allocate on the heap because we need pass the filename parameter to it
	QTextCodec *codec;
private slots:
	void slotReadyRead();
	void slotConnectionClosed();
	void slotError(int);
	void slotReadyReadFile();
};

class DCCServer : public QServerSocket
{
Q_OBJECT
public:
	enum Type {
		Chat = 0,
		File = 1
	};
	DCCServer(Type type, const QString filename = "");
	virtual void newConnection(int socket);
	bool sendMessage(const QString &message);
	DCCClient *mClient;
	void abort();
private:
	Type mType;
	QSocket *mSocket;
	QFile *mFile;
	void sendNextPacket();
private slots:
	void slotConnectionClosed();
	void slotReadyRead();
	void slotError(int);
signals:
	void clientConnected();
	void terminating();
	void incomingAckPercent(int);
	void sendingNonAckPercent(int);
	void sendFinished();
	void readAccessDenied();
};

#endif
/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

