/*
 * socks.h - SOCKS5 TCP proxy client/server
 * Copyright (C) 2003  Justin Karneges
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef CS_SOCKS_H
#define CS_SOCKS_H

#include"bytestream.h"

// CS_NAMESPACE_BEGIN

class QHostAddress;
class SocksServer;

class SocksClient : public ByteStream
{
	Q_OBJECT
public:
	enum Error { ErrConnectionRefused = ErrCustom, ErrHostNotFound, ErrProxyConnect, ErrProxyNeg, ErrProxyAuth };
	enum Method { AuthNone=0x0001, AuthUsername=0x0002 };
	SocksClient(QObject *parent=0);
	SocksClient(int, QObject *parent=0);
	~SocksClient();

	bool isIncoming() const;

	// outgoing
	void setAuth(const QString &user, const QString &pass="");
	void connectToHost(const QString &proxyHost, int proxyPort, const QString &host, int port);

	// incoming
	void chooseMethod(int);
	void authGrant(bool);
	void requestGrant(bool);

	// from ByteStream
	bool isOpen() const;
	void close();
	void write(const QByteArray &);
	QByteArray read(int bytes=0);
	int bytesAvailable() const;
	int bytesToWrite() const;

	QHostAddress peerAddress() const;
	Q_UINT16 peerPort() const;

signals:
	// outgoing
	void connected();

	// incoming
	void incomingMethods(int);
	void incomingAuth(const QString &user, const QString &pass);
	void incomingRequest(const QString &host, int port);

private slots:
	void sock_connected();
	void sock_connectionClosed();
	void sock_delayedCloseFinished();
	void sock_readyRead();
	void sock_bytesWritten(int);
	void sock_error(int);
	void serve();

private:
	class Private;
	Private *d;

	void init();
	void reset(bool clear=false);
	void do_request();
	void processOutgoing(const QByteArray &);
	void processIncoming(const QByteArray &);
	void continueIncoming();
	void writeData(const QByteArray &a);
};

class SocksServer : public QObject
{
	Q_OBJECT
public:
	SocksServer(QObject *parent=0);
	~SocksServer();

	bool isActive() const;
	bool listen(Q_UINT16 port);
	void stop();
	int port() const;
	QHostAddress address() const;
	SocksClient *takeIncoming();

signals:
	void incomingReady();

private slots:
	void connectionReady(int);
	void connectionError();

private:
	class Private;
	Private *d;
};

// CS_NAMESPACE_END

#endif
