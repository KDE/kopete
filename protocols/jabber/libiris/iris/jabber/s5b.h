/*
 * s5b.h - direct connection protocol via tcp
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

#ifndef XMPP_S5B_H
#define XMPP_S5B_H

#include<qobject.h>
#include<qptrlist.h>
#include<qvaluelist.h>
#include"im.h"
#include"bytestream.h"

class SocksClient;

namespace XMPP
{
	class StreamHost;
	class S5BConnection;
	class S5BManager;
	class S5BServer;
	struct S5BRequest;
	typedef QValueList<StreamHost> StreamHostList;
	typedef QPtrList<S5BConnection> S5BConnectionList;
	typedef QPtrListIterator<S5BConnection> S5BConnectionListIt;

	class S5BConnection : public ByteStream
	{
		Q_OBJECT
	public:
		enum { ErrRefused, ErrConnect, ErrProxy, ErrSocket };
		enum { Idle, Requesting, Connecting, WaitingForAccept, Active };
		~S5BConnection();

		Jid proxy() const;
		void setProxy(const Jid &proxy);

		void connectToJid(const Jid &peer, const QString &sid);
		void accept();
		void close();

		Jid peer() const;
		QString sid() const;
		bool isRemote() const;
		int state() const;

		bool isOpen() const;
		void write(const QByteArray &);
		QByteArray read(int bytes=0);
		int bytesAvailable() const;
		int bytesToWrite() const;

	signals:
		void proxyQuery();                             // querying proxy for streamhost information
		void proxyResult(bool b);                      // query success / fail
		void requesting();                             // sent actual S5B request (initiator only)
		void accepted();                               // target accepted (initiator only
		void tryingHosts(const StreamHostList &hosts); // currently connecting to these hosts
		void proxyConnect();                           // connecting to proxy
		void waitingForActivation();                   // waiting for activation (target only)
		void connected();                              // connection active

	private slots:
		void doPending();

		void sc_connectionClosed();
		void sc_delayedCloseFinished();
		void sc_readyRead();
		void sc_bytesWritten(int);
		void sc_error(int);

	private:
		class Private;
		Private *d;

		void reset(bool clear=false);

		friend class S5BManager;
		void man_waitForAccept(const S5BRequest &r);
		void man_clientReady(SocksClient *);
		void man_failed(int);
		S5BConnection(S5BManager *, QObject *parent=0);
	};

	class S5BManager : public QObject
	{
		Q_OBJECT
	public:
		S5BManager(Client *);
		~S5BManager();

		Client *client() const;
		S5BServer *server() const;
		void setServer(S5BServer *s);

		bool isAcceptableSID(const Jid &peer, const QString &sid) const;
		QString genUniqueSID(const Jid &peer) const;

		S5BConnection *createConnection();
		S5BConnection *takeIncoming();

		class Item;
		class Entry;

	signals:
		void incomingReady();

	private slots:
		void ps_incoming(const S5BRequest &req);
		void item_accepted();
		void item_tryingHosts(const StreamHostList &list);
		void item_proxyConnect();
		void item_waitingForActivation();
		void item_connected();
		void item_error(int);
		void query_finished();

	private:
		class Private;
		Private *d;

		S5BConnection *findIncoming(const Jid &from, const QString &sid) const;
		Entry *findEntry(S5BConnection *) const;
		Entry *findEntry(Item *) const;
		Entry *findEntryByHash(const QString &key) const;
		Entry *findEntryBySID(const Jid &peer, const QString &sid) const;
		Entry *findServerEntryByHash(const QString &key) const;

		void entryContinue(Entry *e);
		void queryProxy(Entry *e);
		bool targetShouldOfferProxy(Entry *e);

		friend class S5BConnection;
		void con_connect(S5BConnection *);
		void con_accept(S5BConnection *);
		void con_reject(S5BConnection *);
		void con_unlink(S5BConnection *);

		friend class S5BServer;
		bool srv_ownsHash(const QString &key) const;
		void srv_incomingReady(SocksClient *sc, const QString &key);
		void srv_unlink();

		friend class Item;
		void doSuccess(const Jid &peer, const QString &id, const Jid &streamHost);
		void doError(const Jid &peer, const QString &id, int, const QString &);
	};

	class S5BConnector : public QObject
	{
		Q_OBJECT
	public:
		S5BConnector(QObject *parent=0);
		~S5BConnector();

		void reset();
		void start(const StreamHostList &hosts, const QString &key);
		SocksClient *takeClient();
		StreamHost streamHostUsed() const;

		class Item;

	signals:
		void result(bool);

	private slots:
		void item_result(bool);
		void t_timeout();

	private:
		class Private;
		Private *d;
	};

	// listens on a port for serving
	class S5BServer : public QObject
	{
		Q_OBJECT
	public:
		S5BServer(QObject *par=0);
		~S5BServer();

		bool isActive() const;
		bool start(int port);
		void stop();
		int port() const;
		void setHostList(const QStringList &);
		QStringList hostList() const;

		class Item;

	private slots:
		void ss_incomingReady();
		void item_result(bool);

	private:
		class Private;
		Private *d;

		friend class S5BManager;
		void link(S5BManager *);
		void unlink(S5BManager *);
		void unlinkAll();
		const QPtrList<S5BManager> & managerList() const;
	};

	class JT_S5B : public Task
	{
		Q_OBJECT
	public:
		JT_S5B(Task *);
		~JT_S5B();

		void request(const Jid &to, const QString &sid, const StreamHostList &hosts, bool fast);
		void requestProxyInfo(const Jid &to);
		void requestActivation(const Jid &to, const QString &sid, const Jid &target);

		void onGo();
		void onDisconnect();
		bool take(const QDomElement &);

		Jid streamHostUsed() const;
		StreamHost proxyInfo() const;

	private slots:
		void t_timeout();

	private:
		class Private;
		Private *d;
	};

	struct S5BRequest
	{
		Jid from;
		QString id, sid;
		StreamHostList hosts;
		bool fast;
	};
	class JT_PushS5B : public Task
	{
		Q_OBJECT
	public:
		JT_PushS5B(Task *);
		~JT_PushS5B();

		void respondSuccess(const Jid &to, const QString &id, const Jid &streamHost);
		void respondError(const Jid &to, const QString &id, int code, const QString &str);

		bool take(const QDomElement &);

	signals:
		void incoming(const S5BRequest &req);
	};

	class StreamHost
	{
	public:
		StreamHost();

		const Jid & jid() const;
		const QString & host() const;
		int port() const;
		bool isProxy() const;
		void setJid(const Jid &);
		void setHost(const QString &);
		void setPort(int);
		void setIsProxy(bool);

	private:
		Jid j;
		QString v_host;
		int v_port;
		bool proxy;
	};
}

#endif
