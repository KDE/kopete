/*
 * dtcp.h - direct connection protocol via tcp
 * Copyright (C) 2001, 2002  Justin Karneges
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

#ifndef JABBER_DTCP_H
#define JABBER_DTCP_H

#include<qobject.h>
#include<qserversocket.h>
#include<qdom.h>
#include<qstring.h>
#include<qptrlist.h>
#include<qvaluelist.h>
#include"xmpp_jid.h"
#include"xmpp_client.h"
#include"xmpp_bytestream.h"

class QSocket;

namespace Jabber
{
	class DTCPSocketHandler;
	class DTCPServer;
	class DTCPManager;
	class HostPort;
	typedef QValueList<HostPort> HostPortList;

	// this is a dtcp connection object.  use it much like a qsocket
	class DTCPConnection : public ByteStream
	{
		Q_OBJECT
	public:
		enum { ErrRequest, ErrConnect, ErrSocket };
		enum { Idle, Requesting, Connecting, WaitingForAccept, Active };
		DTCPConnection(DTCPManager *);
		~DTCPConnection();

		void connectToJid(const Jid &peer, const QDomElement &comment);
		void accept();
		void close();

		bool isRemote() const;
		int state() const;
		Jid peer() const;
		QDomElement comment() const;

		bool isConnected() const;
		void write(const QByteArray &);
		QByteArray read();
		bool canRead() const;
		int bytesToWrite() const;

	signals:
		void accepted();
		void connected();

	private slots:
		void dsh_connectionClosed();
		void dsh_delayedCloseFinished();
		void dsh_readyRead();
		void dsh_bytesWritten(int);
		void dsh_error(int);

		void dtcp_finished();
		void out_result(bool);
		void postAccept();
		void postConnect();
		void postContinue();
		void t_timeout();

	private:
		class Private;
		Private *d;

		void reset(bool clear=false);
		void setSocketHandler(DTCPSocketHandler *);
		void checkGaveUp();

		friend class DTCPManager;
		friend class DTCPServer;
		friend class DTCPSocketHandler;
		void setIncomingHandler(DTCPSocketHandler *);
		void waitForAccept(const Jid &peer, const QString &localKey, const QString &remoteKey, const HostPortList &hosts, const QDomElement &comment, const QString &iq_id);
		void onError(int, const QString &);

		QString localKey() const;
		QString remoteKey() const;
		bool hasRemoteKey() const;
	};

	typedef QPtrList<DTCPConnection> DTCPConnectionList;
	typedef QPtrListIterator<DTCPConnection> DTCPConnectionListIt;
	class DTCPManager : public QObject
	{
		Q_OBJECT
	public:
		DTCPManager(Client *);
		~DTCPManager();

		Client *client() const;
		DTCPServer *server() const;
		void setServer(DTCPServer *);

		DTCPConnection *takeIncoming();

	signals:
		void incomingReady();

	private slots:
		void pdtcp_incoming(const Jid &from, const QString &id, const QString &key, const HostPortList &hosts, const QDomElement &comment);
		void pdtcp_error(const Jid &from, const QString &key, int, const QString &);

	private:
		class Private;
		Private *d;

		QString genKey() const;

		friend class DTCPSocketHandler;
		friend class DTCPConnection;
		friend class DTCPServer;
		QString genUniqueKey() const;
		void link(DTCPConnection *);
		void unlink(DTCPConnection *);
		void doAccept(DTCPConnection *c, const QString &id, const HostPortList &);
		void doReject(DTCPConnection *c, const QString &id, int, const QString &);
		DTCPConnection *findConnection(const QString &) const;
		DTCPConnection *findRemoteConnection(const Jid &, const QString &) const;
		void continueAfterWait(const QString &);
		void sendError(const Jid &to, const QString &key, int code, const QString &str);
	};

	class DTCPOutgoing : public QObject
	{
		Q_OBJECT
	public:
		DTCPOutgoing(DTCPManager *);
		~DTCPOutgoing();

		void start(const HostPortList &, const Jid &peer, const QString &keyA, const QString &keyB, bool requestor);
		void stop();
		DTCPSocketHandler *takeHandler() const;
		void reset();

	signals:
		void result(bool);

	private slots:
		void dsh_connected();
		void dsh_error(int);
		void conn();

	private:
		class Private;
		Private *d;
	};

	// listens on a port for serving
	class DTCPServer : public QObject
	{
		Q_OBJECT
	public:
		DTCPServer(QObject *par=0);
		~DTCPServer();

		bool isActive() const;
		bool listen(int port);
		int port() const;
		void setHostList(const QStringList &);
		QStringList hostList() const;

	private slots:
		void connectionReady(int);
		void dsh_connected();
		void dsh_error(int);

	private:
		class Private;
		Private *d;

		friend class DTCPManager;
		void link(DTCPManager *);
		void unlink(DTCPManager *);
		void continueAfterWait(const QString &);

		friend class DTCPSocketHandler;
		DTCPConnection *findConnection(const QString &key) const;
	};

	// handles dtcp socket connections as either client or server
	class DTCPSocketHandler : public QObject
	{
		Q_OBJECT
	public:
		enum { ErrConnect, ErrSocket, ErrHandshake, ErrTimeout };
		enum { Client, Server };
		DTCPSocketHandler(DTCPManager *);
		DTCPSocketHandler(DTCPServer *);
		~DTCPSocketHandler();

		void handle(const QString &host, int port, const Jid &peer, const QString &keyA, const QString &keyB, bool requestor);
		void handle(int s);
		void close();

		QString host() const;
		int port() const;
		int mode() const;
		Jid peer() const;
		bool isConnected() const;
		bool isWaiting() const;
		QString localKey() const;
		void continueAfterWait();

		void write(const QByteArray &);
		QByteArray read();
		bool canRead() const;
		int bytesToWrite() const;

	signals:
		void connected();
		void connectionClosed();
		void delayedCloseFinished();
		void readyRead();
		void bytesWritten(int);
		void error(int);

	private slots:
		void ndns_done();

		void sock_connected();
		void sock_connectionClosed();
		void sock_delayedCloseFinished();
		void sock_readyRead();
		void sock_bytesWritten(int);
		void sock_error(int);

		void t_timeout();
		void postConnect();

	private:
		class Private;
		Private *d;

		void init();
		void reset(bool clear=false);
		void serverReset();
		void doSuccess();
		void doError(int);
		void writeLine(const QString &);
		QString extractLine(QByteArray *, bool *) const;
		bool processLine(const QString &);
		bool validate(const QString &);
	};

	class JT_DTCP : public Task
	{
		Q_OBJECT
	public:
		JT_DTCP(Task *);
		~JT_DTCP();

		void request(const Jid &to, const QString &key, const HostPortList &hosts, const QDomElement &comment);

		void onGo();
		bool take(const QDomElement &);

		Jid jid() const;
		QString key() const;
		HostPortList hostList() const;

	private:
		class Private;
		Private *d;
	};

	class JT_PushDTCP : public Task
	{
		Q_OBJECT
	public:
		JT_PushDTCP(Task *);
		~JT_PushDTCP();

		void respondSuccess(const Jid &to, const QString &id, const QString &key, const HostPortList &hosts);
		void respondError(const Jid &to, const QString &id, int code, const QString &str);

		bool take(const QDomElement &);

	signals:
		void incoming(const Jid &from, const QString &id, const QString &key, const HostPortList &hosts, const QDomElement &comment);
		void error(const Jid &from, const QString &key, int code, const QString &str);
	};

	class HostPort
	{
	public:
		HostPort(const QString &host="", int port=0);

		const QString & host() const;
		int port() const;
		void setHost(const QString &);
		void setPort(int);

	private:
		QString v_host;
		int v_port;
	};

	class ServSock : public QServerSocket
	{
		Q_OBJECT
	public:
		ServSock(int port);

	signals:
		void connectionReady(int);

	protected:
		// reimplemented
		void newConnection(int);
	};
}

#endif
