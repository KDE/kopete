/*
 * s5b.cpp - direct connection protocol via tcp
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

/*
  TODO:

    S5BConnection 'progress' signals
      querying proxy
      requesting
      accepted
      connected
    remove roleswitch code
    support zeroconf
*/

#include"s5b.h"

#include<qtimer.h>
#include<stdlib.h>
#include<qca.h>
#include"xmpp_xmlcommon.h"
#include"socks.h"

namespace XMPP {

static QString makeKey(const QString &sid, const Jid &initiator, const Jid &target)
{
	QString str = sid + initiator.full() + target.full();
	QString result = QCA::SHA1::hashToString(str.utf8());
	printf("key=[%s]\n", result.latin1());
	return result;
}

static bool haveHost(const StreamHostList &list, const Jid &j)
{
	for(StreamHostList::ConstIterator it = list.begin(); it != list.end(); ++it) {
		if((*it).jid().compare(j))
			return true;
	}
	return false;
}

class S5BManager::Item : public QObject
{
	Q_OBJECT
public:
	enum { Idle, Initiator, Target, Active };
	enum { ErrRefused, ErrConnect, ErrWrongHost, ErrProxy, ErrRoleswitch };
	enum { Unknown, Fast, NotFast };
	S5BManager *m;
	int state;
	QString sid, key, out_key, out_id, in_id;
	Jid self, peer;
	StreamHostList in_hosts;
	JT_S5B *task, *proxy_task;
	SocksClient *client;
	S5BConnector *conn, *proxy_conn;
	QPtrList<SocksClient> clientList;
	bool wantRoleswitch, wantFast;
	bool peerDoesRoleswitch;
	bool fastFailed;
	StreamHost proxy;
	int targetMode; // initiator sets this once it figures it out
	bool fast; // target sets this
	bool activated;
	bool lateProxy;
	bool connSuccess;

	Item(S5BManager *manager);
	~Item();

	void reset();
	void startInitiator(const QString &_sid, const Jid &_self, const Jid &_peer, bool roleswitch, bool fast);
	void startTarget(const QString &_sid, const Jid &_self, const Jid &_peer, const StreamHostList &hosts, const QString &iq_id, bool roleswitch, bool fast);
	void handleFast(const StreamHostList &hosts, const QString &iq_id);

	void doOutgoing();
	void doIncoming();
	void setIncomingClient(SocksClient *sc);
	void clientReady(SocksClient *sc);

signals:
	void connected();
	void error(int);

private slots:
	void jt_finished();
	void conn_result(bool b);
	void proxy_result(bool b);
	void proxy_finished();
	void sc_readyRead();
	void sc_bytesWritten(int);
	void sc_error(int);

private:
	void doConnectError();
	void tryActivation();
	void checkForActivation();
	void finished();
};

//----------------------------------------------------------------------------
// S5BConnection
//----------------------------------------------------------------------------
class S5BConnection::Private
{
public:
	S5BManager *m;
	SocksClient *sc;
	int state;
	Jid peer;
	QString sid;
	bool remote;
	bool switched;
	bool reuseme;
	bool notifyRead, notifyClose;
	int id;
	S5BRequest req;
};

static int id_conn = 0;
static int num_conn = 0;

S5BConnection::S5BConnection(S5BManager *m, QObject *parent)
:ByteStream(parent)
{
	d = new Private;
	d->m = m;
	d->sc = 0;

	++num_conn;
	d->id = id_conn++;
	printf("S5BConnection[%d]: constructing, count=%d, %p\n", d->id, num_conn, this);

	reset();
}

S5BConnection::~S5BConnection()
{
	reset(true);

	--num_conn;
	printf("S5BConnection[%d]: destructing, count=%d\n", d->id, num_conn);

	delete d;
}

void S5BConnection::reset(bool clear)
{
	d->m->con_unlink(this);
	if(clear && d->sc) {
		delete d->sc;
		d->sc = 0;
	}
	d->state = Idle;
	d->peer = Jid();
	d->sid = QString();
	d->remote = false;
	d->switched = false;
	d->reuseme = false;
	d->notifyRead = false;
	d->notifyClose = false;
}

void S5BConnection::connectToJid(const Jid &peer, const QString &sid)
{
	reset(true);
	if(!d->m->isAcceptableSID(peer, sid))
		return;

	d->peer = peer;
	d->sid = sid;
	d->state = Requesting;
	printf("S5BConnection[%d]: connecting %s [%s]\n", d->id, d->peer.full().latin1(), d->sid.latin1());
	d->m->con_connect(this);
}

void S5BConnection::accept()
{
	if(d->state != WaitingForAccept)
		return;

	d->state = Connecting;
	printf("S5BConnection[%d]: accepting %s [%s]\n", d->id, d->peer.full().latin1(), d->sid.latin1());
	d->m->con_accept(this);
}

void S5BConnection::close()
{
	if(d->state == Idle)
		return;

	if(d->state == WaitingForAccept)
		d->m->con_reject(this);
	else if(d->state == Active)
		d->sc->close();
	printf("S5BConnection[%d]: closing %s [%s]\n", d->id, d->peer.full().latin1(), d->sid.latin1());
	reset();
}

Jid S5BConnection::peer() const
{
	return d->peer;
}

QString S5BConnection::sid() const
{
	return d->sid;
}

bool S5BConnection::isRemote() const
{
	return d->remote;
}

int S5BConnection::state() const
{
	return d->state;
}

bool S5BConnection::isOpen() const
{
	if(d->state == Active)
		return true;
	else
		return false;
}

void S5BConnection::write(const QByteArray &buf)
{
	if(d->state == Active)
		d->sc->write(buf);
}

QByteArray S5BConnection::read(int bytes)
{
	if(d->sc)
		return d->sc->read(bytes);
	else
		return QByteArray();
}

int S5BConnection::bytesAvailable() const
{
	if(d->sc)
		return d->sc->bytesAvailable();
	else
		return 0;
}

int S5BConnection::bytesToWrite() const
{
	if(d->state == Active)
		return d->sc->bytesToWrite();
	else
		return 0;
}

void S5BConnection::man_waitForAccept(const S5BRequest &r)
{
	d->state = WaitingForAccept;
	d->remote = true;
	d->req = r;
	d->peer = r.from;
	d->sid = r.sid;
}

void S5BConnection::man_clientReady(SocksClient *sc)
{
	d->sc = sc;
	connect(d->sc, SIGNAL(connectionClosed()), SLOT(sc_connectionClosed()));
	connect(d->sc, SIGNAL(delayedCloseFinished()), SLOT(sc_delayedCloseFinished()));
	connect(d->sc, SIGNAL(readyRead()), SLOT(sc_readyRead()));
	connect(d->sc, SIGNAL(bytesWritten(int)), SLOT(sc_bytesWritten(int)));
	connect(d->sc, SIGNAL(error(int)), SLOT(sc_error(int)));

	d->state = Active;
	printf("S5BConnection[%d]: %s [%s] <<< success >>>\n", d->id, d->peer.full().latin1(), d->sid.latin1());

	// bytes already in the stream?
	if(d->sc->bytesAvailable()) {
		printf("Stream has %d bytes in it.\n", d->sc->bytesAvailable());
		d->notifyRead = true;
	}
	// closed before it got here?
	if(!d->sc->isOpen()) {
		printf("Stream was closed before S5B request finished?\n");
		d->notifyClose = true;
	}
	if(d->notifyRead || d->notifyClose)
		QTimer::singleShot(0, this, SLOT(doPending()));
	connected();
}

void S5BConnection::doPending()
{
	if(d->notifyRead) {
		if(d->notifyClose)
			QTimer::singleShot(0, this, SLOT(doPending()));
		sc_readyRead();
	}
	else if(d->notifyClose)
		sc_connectionClosed();
}

void S5BConnection::man_failed(int x)
{
	// TODO: roleswitch
	//if(x == S5BManager::Item::ErrRoleswitch) {
	//}

	reset(true);
	if(x == S5BManager::Item::ErrRefused)
		error(ErrRefused);
	if(x == S5BManager::Item::ErrConnect)
		error(ErrConnect);
	if(x == S5BManager::Item::ErrWrongHost)
		error(ErrConnect);
}

void S5BConnection::sc_connectionClosed()
{
	// if we have a pending read notification, postpone close
	if(d->notifyRead) {
		printf("closed while pending read\n");
		d->notifyClose = true;
		return;
	}
	d->notifyClose = false;
	reset();
	connectionClosed();
}

void S5BConnection::sc_delayedCloseFinished()
{
	// echo
	delayedCloseFinished();
}

void S5BConnection::sc_readyRead()
{
	d->notifyRead = false;
	// echo
	readyRead();
}

void S5BConnection::sc_bytesWritten(int x)
{
	// echo
	bytesWritten(x);
}

void S5BConnection::sc_error(int)
{
	reset();
	error(ErrSocket);
}

//----------------------------------------------------------------------------
// S5BManager
//----------------------------------------------------------------------------
class S5BManager::Entry
{
public:
	Entry()
	{
		i = 0;
		query = 0;
	}

	~Entry()
	{
		delete query;
	}

	S5BConnection *c;
	Item *i;
	QString sid;
	JT_S5B *query;
	StreamHost proxyInfo;
};

class S5BManager::Private
{
public:
	Client *client;
	S5BServer *serv;
	QPtrList<Entry> activeList;
	S5BConnectionList incomingConns;
	JT_PushS5B *ps;
	Jid proxy;
};

S5BManager::S5BManager(Client *parent)
:QObject(parent)
{
	d = new Private;
	d->client = parent;
	d->serv = 0;
	d->activeList.setAutoDelete(true);

	d->ps = new JT_PushS5B(d->client->rootTask());
	connect(d->ps, SIGNAL(incoming(const S5BRequest &)), SLOT(ps_incoming(const S5BRequest &)));
}

S5BManager::~S5BManager()
{
	setServer(0);
	d->incomingConns.setAutoDelete(true);
	d->incomingConns.clear();
	delete d->ps;
	delete d;
}

Client *S5BManager::client() const
{
	return d->client;
}

S5BServer *S5BManager::server() const
{
	return d->serv;
}

void S5BManager::setServer(S5BServer *serv)
{
	if(d->serv) {
		d->serv->unlink(this);
		d->serv = 0;
	}

	if(serv) {
		d->serv = serv;
		d->serv->link(this);
	}
}

void S5BManager::setProxy(const Jid &proxy)
{
	d->proxy = proxy;
}

S5BConnection *S5BManager::createConnection()
{
	S5BConnection *c = new S5BConnection(this);
	return c;
}

S5BConnection *S5BManager::takeIncoming()
{
	if(d->incomingConns.isEmpty())
		return 0;

	S5BConnection *c = d->incomingConns.getFirst();
	d->incomingConns.removeRef(c);

	// move to activeList
	Entry *e = new Entry;
	e->c = c;
	e->sid = c->d->sid;
	d->activeList.append(e);

	return c;
}

void S5BManager::ps_incoming(const S5BRequest &req)
{
	printf("S5BManager: incoming from %s\n", req.from.full().latin1());

	bool ok = false;
	// ensure we don't already have an incoming connection from this peer+sid
	S5BConnection *c = findIncoming(req.from, req.sid);
	if(!c) {
		// do we have an active entry with this sid already?
		Entry *e = findEntryBySID(req.from, req.sid);
		if(e) {
			if(e->i) {
				// loopback
				if(req.from.compare(d->client->jid()) && (req.id == e->i->out_id)) {
					printf("ALLOWED: loopback\n");
					ok = true;
				}
				// allowed by 'fast mode'
				else if(e->i->state == Item::Initiator && e->i->targetMode == Item::Unknown) {
					printf("ALLOWED: fast-mode\n");
					e->i->handleFast(req.hosts, req.id);
					return;
				}
			}
		}
		else {
			printf("ALLOWED: we don't have it\n");
			ok = true;
		}
	}
	if(!ok) {
		d->ps->respondError(req.from, req.id, 403, "SID in use");
		return;
	}

	// create an incoming connection
	c = new S5BConnection(this);
	c->man_waitForAccept(req);
	d->incomingConns.append(c);
	incomingReady();
}

void S5BManager::doSuccess(const Jid &peer, const QString &id, const Jid &streamHost)
{
	d->ps->respondSuccess(peer, id, streamHost);
}

void S5BManager::doError(const Jid &peer, const QString &id, int code, const QString &str)
{
	d->ps->respondError(peer, id, code, str);
}

QString S5BManager::genUniqueSID(const Jid &peer) const
{
	// get unused key
	QString sid;
	do {
		sid = "s5b_";
		for(int i = 0; i < 4; ++i) {
			int word = rand() & 0xffff;
			for(int n = 0; n < 4; ++n) {
				QString s;
				s.sprintf("%x", (word >> (n * 4)) & 0xf);
				sid.append(s);
			}
		}
	} while(!isAcceptableSID(peer, sid));
	return sid;
}

bool S5BManager::isAcceptableSID(const Jid &peer, const QString &sid) const
{
	QString key = makeKey(sid, d->client->jid(), peer);

	// if we have a server, then check through it
	if(d->serv) {
		if(findServerEntryByHash(key))
			return false;
	}
	else {
		if(findEntryByHash(key))
			return false;
	}
	return true;
}

S5BConnection *S5BManager::findIncoming(const Jid &from, const QString &sid) const
{
	QPtrListIterator<S5BConnection> it(d->incomingConns);
	for(S5BConnection *c; (c = it.current()); ++it) {
		if(c->d->peer.compare(from) && c->d->sid == sid)
			return c;
	}
	return 0;
}

S5BManager::Entry *S5BManager::findEntry(S5BConnection *c) const
{
	QPtrListIterator<Entry> it(d->activeList);
	for(Entry *e; (e = it.current()); ++it) {
		if(e->c == c)
			return e;
	}
	return 0;
}

S5BManager::Entry *S5BManager::findEntry(Item *i) const
{
	QPtrListIterator<Entry> it(d->activeList);
	for(Entry *e; (e = it.current()); ++it) {
		if(e->i == i)
			return e;
	}
	return 0;
}

S5BManager::Entry *S5BManager::findEntryByHash(const QString &key) const
{
	QPtrListIterator<Entry> it(d->activeList);
	for(Entry *e; (e = it.current()); ++it) {
		if(e->i && e->i->key == key)
			return e;
	}
	return 0;
}

S5BManager::Entry *S5BManager::findEntryBySID(const Jid &peer, const QString &sid) const
{
	QPtrListIterator<Entry> it(d->activeList);
	for(Entry *e; (e = it.current()); ++it) {
		if(e->i && e->i->peer.compare(peer) && e->sid == sid)
			return e;
	}
	return 0;
}

S5BManager::Entry *S5BManager::findServerEntryByHash(const QString &key) const
{
	const QPtrList<S5BManager> &manList = d->serv->managerList();
	QPtrListIterator<S5BManager> it(manList);
	for(S5BManager *m; (m = it.current()); ++it) {
		Entry *e = m->findEntryByHash(key);
		if(e)
			return e;
	}
	return 0;
}

bool S5BManager::srv_ownsHash(const QString &key) const
{
	if(findEntryByHash(key))
		return true;
	return false;
}

void S5BManager::srv_incomingReady(SocksClient *sc, const QString &key)
{
	Entry *e = findEntryByHash(key);
	if(e->i->client || e->i->state == Item::Active) {
		sc->requestGrant(false);
		sc->deleteLater();
		return;
	}
	sc->requestGrant(true);
	e->i->setIncomingClient(sc);
}

void S5BManager::con_connect(S5BConnection *c)
{
	if(findEntry(c))
		return;
	Entry *e = new Entry;
	e->c = c;
	e->sid = c->d->sid;
	d->activeList.append(e);

	if(d->proxy.isValid()) {
		queryProxy(e);
		return;
	}
	entryContinue(e);
}

void S5BManager::con_accept(S5BConnection *c)
{
	Entry *e = findEntry(c);
	if(!e)
		return;

	if(d->proxy.isValid()) {
		if(targetShouldOfferProxy(e)) {
			queryProxy(e);
			return;
		}
	}
	entryContinue(e);
}

void S5BManager::con_reject(S5BConnection *c)
{
	d->ps->respondError(c->d->peer, c->d->req.id, 406, "Not acceptable");
}

void S5BManager::con_unlink(S5BConnection *c)
{
	Entry *e = findEntry(c);
	if(!e)
		return;

	// active incoming request?  cancel it
	if(e->i && e->i->conn)
		d->ps->respondError(e->i->peer, e->i->out_id, 406, "Not acceptable");
	delete e->i;
	d->activeList.removeRef(e);
}

void S5BManager::item_connected()
{
	Item *i = (Item *)sender();
	Entry *e = findEntry(i);

	// grab the client
	SocksClient *client = i->client;
	i->client = 0;

	// give it to the connection
	e->c->man_clientReady(client);
}

void S5BManager::item_error(int x)
{
	Item *i = (Item *)sender();
	Entry *e = findEntry(i);

	e->c->man_failed(x);
}

void S5BManager::entryContinue(Entry *e)
{
	e->i = new Item(this);
	e->i->proxy = e->proxyInfo;

	if(e->c->isRemote()) {
		connect(e->i, SIGNAL(connected()), SLOT(item_connected()));
		connect(e->i, SIGNAL(error(int)), SLOT(item_error(int)));

		const S5BRequest &req = e->c->d->req;
		e->i->startTarget(e->sid, d->client->jid(), e->c->d->peer, req.hosts, req.id, req.roleswitch, req.fast);
	}
	else {
		connect(e->i, SIGNAL(connected()), SLOT(item_connected()));
		connect(e->i, SIGNAL(error(int)), SLOT(item_error(int)));

		e->i->startInitiator(e->sid, d->client->jid(), e->c->d->peer, false, true);
	}
}

void S5BManager::queryProxy(Entry *e)
{
	printf("querying proxy: [%s]\n", d->proxy.full().latin1());
	e->query = new JT_S5B(d->client->rootTask());
	connect(e->query, SIGNAL(finished()), SLOT(query_finished()));
	e->query->requestProxyInfo(d->proxy);
	e->query->go(true);
}

void S5BManager::query_finished()
{
	JT_S5B *query = (JT_S5B *)sender();
	Entry *e;
	bool found = false;
	QPtrListIterator<Entry> it(d->activeList);
	for(; (e = it.current()); ++it) {
		if(e->query == query) {
			found = true;
			break;
		}
	}
	if(!found)
		return;
	e->query = 0;

	printf("query finished: ");
	if(query->success()) {
		e->proxyInfo = query->proxyInfo();
		printf("host/ip=[%s] port=[%d]\n", e->proxyInfo.host().latin1(), e->proxyInfo.port());
	}
	else
		printf("fail\n");

	entryContinue(e);
}

bool S5BManager::targetShouldOfferProxy(Entry *e)
{
	if(!d->proxy.isValid())
		return false;

	// if target, don't offer any proxy if the initiator already did
	const StreamHostList &hosts = e->c->d->req.hosts;
	for(StreamHostList::ConstIterator it = hosts.begin(); it != hosts.end(); ++it) {
		if((*it).isProxy())
			return false;
	}

	// ensure we don't offer the same proxy as the initiator
	if(haveHost(hosts, d->proxy))
		return false;

	return true;
}

//----------------------------------------------------------------------------
// S5BManager::Item
//----------------------------------------------------------------------------
S5BManager::Item::Item(S5BManager *manager) : QObject(0)
{
	m = manager;
	task = 0;
	proxy_task = 0;
	conn = 0;
	proxy_conn = 0;
	client = 0;
	reset();
}

S5BManager::Item::~Item()
{
	reset();
}

void S5BManager::Item::reset()
{
	delete task;
	task = 0;
	delete proxy_task;
	proxy_task = 0;
	delete conn;
	conn = 0;
	delete proxy_conn;
	proxy_conn = 0;
	clientList.setAutoDelete(true);
	clientList.clear();
	clientList.setAutoDelete(false);
	delete client;
	client = 0;
	state = Idle;
	wantRoleswitch = false;
	wantFast = false;
	peerDoesRoleswitch = false;
	fastFailed = false;
	targetMode = Unknown;
	fast = false;
	activated = false;
	lateProxy = false;
	connSuccess = false;
}

void S5BManager::Item::startInitiator(const QString &_sid, const Jid &_self, const Jid &_peer, bool roleswitch, bool fast)
{
	sid = _sid;
	self = _self;
	peer = _peer;
	key = makeKey(sid, self, peer);
	out_key = makeKey(sid, peer, self);
	wantRoleswitch = roleswitch;
	wantFast = fast;

	printf("S5BManager::Item initiating request %s [%s]\n", peer.full().latin1(), sid.latin1());
	state = Initiator;
	doOutgoing();
}

void S5BManager::Item::startTarget(const QString &_sid, const Jid &_self, const Jid &_peer, const StreamHostList &hosts, const QString &iq_id, bool roleswitch, bool _fast)
{
	sid = _sid;
	peer = _peer;
	self = _self;
	in_hosts = hosts;
	in_id = iq_id;
	fast = _fast;
	key = makeKey(sid, self, peer);
	out_key = makeKey(sid, peer, self);
	//key = makeKey(sid, peer, self);
	//out_key = makeKey(sid, self, peer);
	peerDoesRoleswitch = roleswitch;

	printf("S5BManager::Item incoming request %s [%s]\n", peer.full().latin1(), sid.latin1());
	state = Target;
	if(fast)
		doOutgoing();
	doIncoming();
}

void S5BManager::Item::handleFast(const StreamHostList &hosts, const QString &iq_id)
{
	targetMode = Fast;

	// if we already have a stream, then bounce this request
	if(client) {
		m->doError(peer, iq_id, 406, "Not Acceptable");
	}
	else {
		in_hosts = hosts;
		in_id = iq_id;
		doIncoming();
	}
}

void S5BManager::Item::doOutgoing()
{
	StreamHostList hosts;
	S5BServer *serv = m->server();
	if(serv && serv->isActive() /*&& !haveHost(in_hosts, m->client()->jid())*/) {
		QStringList hostList = serv->hostList();
		for(QStringList::ConstIterator it = hostList.begin(); it != hostList.end(); ++it) {
			StreamHost h;
			h.setJid(m->client()->jid());
			h.setHost(*it);
			h.setPort(serv->port());
			hosts += h;
		}
	}

	// if the proxy is valid, then it's ok to add (the manager already
	// ensured that it doesn't conflict)
	if(proxy.jid().isValid())
		hosts += proxy;

	if(state == Target && hosts.isEmpty()) {
		fast = false;
		return;
	}

	task = new JT_S5B(m->client()->rootTask());
	connect(task, SIGNAL(finished()), SLOT(jt_finished()));
	task->request(peer, sid, hosts, state == Initiator ? wantRoleswitch : false, state == Initiator ? wantFast : false);
	out_id = task->id();
	task->go(true);
}

void S5BManager::Item::doIncoming()
{
	if(in_hosts.isEmpty()) {
		doConnectError();
		return;
	}

	// if target, see about trying non-proxy before proxy
	StreamHostList list;
	if(state == Target) {
		if(lateProxy) {
			// take just the proxy streamhosts
			for(StreamHostList::ConstIterator it = in_hosts.begin(); it != in_hosts.end(); ++it) {
				if((*it).isProxy())
					list += *it;
			}
			lateProxy = false;
		}
		else {
			// take just the non-proxy streamhosts
			bool hasProxies;
			for(StreamHostList::ConstIterator it = in_hosts.begin(); it != in_hosts.end(); ++it) {
				if((*it).isProxy())
					hasProxies = true;
				else
					list += *it;
			}
			if(hasProxies)
				lateProxy = true;

			// hmm, no regular streamhosts?
			if(list.isEmpty()) {
				// if fast, wait for iq-error before trying proxy
				if(fast)
					return;
				// else try proxy now
				else
					list = in_hosts;
			}
		}
	}
	else
		list = in_hosts;

	conn = new S5BConnector;
	connect(conn, SIGNAL(result(bool)), SLOT(conn_result(bool)));
	conn->start(list, out_key);
}

void S5BManager::Item::setIncomingClient(SocksClient *sc)
{
	printf("S5BManager::Item: %s [%s] successful incoming connection\n", peer.full().latin1(), sid.latin1());

	connect(sc, SIGNAL(readyRead()), SLOT(sc_readyRead()));
	connect(sc, SIGNAL(bytesWritten(int)), SLOT(sc_bytesWritten(int)));
	connect(sc, SIGNAL(error(int)), SLOT(sc_error(int)));

	if(state == Initiator)
		clientReady(sc);
	else
		clientList.append(sc);
}

// This function singles out a stream.  The initiator calls this prior to activation,
// and the target calls it after activation.
void S5BManager::Item::clientReady(SocksClient *sc)
{
	// if connecting, stop and send error
	bool senderr = false;
	if(conn) {
		delete conn;
		conn = 0;
		senderr = true;
	}

	// cancel any proxy activity
	if(proxy_conn) {
		delete proxy_conn;
		proxy_conn = 0;
	}
	delete proxy_task;
	proxy_task = 0;

	// throw away unused connections
	clientList.setAutoDelete(true);
	clientList.clear();
	clientList.setAutoDelete(false);

	// make it official
	client = sc;

	if(senderr)
		doConnectError();
}

void S5BManager::Item::jt_finished()
{
	JT_S5B *j = task;
	task = 0;

	printf("jt_finished: state=%s, success=%d\n", state == Initiator ? "initiator" : "target", j->success());

	if(state == Initiator) {
		if(targetMode == Unknown)
			targetMode = NotFast;
	}

	if(j->success()) {
		Jid streamHost = j->streamHostUsed();
		// they connected to us?
		if(streamHost.compare(self)) {
			if(state == Initiator && client) {
				tryActivation();
			}
			else if(state == Target && !clientList.isEmpty()) {
				checkForActivation();
			}
			else {
				printf("S5BManager::Item %s claims to have connected to us, but we don't see this\n", peer.full().latin1());
				reset();
				error(ErrWrongHost);
			}
		}
		else if(streamHost.compare(proxy.jid())) {
			if(state == Initiator && client) {
				delete client;
				client = 0;

				// Hack trick: setting zero here will basically
				// reset everything.  clientReady() calls
				// doConnectError() if needed, which does not
				// cause signals if state == Initiator, so
				// it is safe to call here.
				clientReady(0);
			}

			printf("attempting to connect to proxy\n");
			// connect to the proxy
			proxy_conn = new S5BConnector;
			connect(proxy_conn, SIGNAL(result(bool)), SLOT(proxy_result(bool)));
			StreamHostList list;
			list += proxy;
			proxy_conn->start(list, key);
		}
		else {
			printf("S5BManager::Item %s claims to have connected to a streamhost we never offered\n", peer.full().latin1());
			reset();
			error(ErrWrongHost);
		}
	}
	else {
		printf("S5BManager::Item %s [%s] error\n", peer.full().latin1(), sid.latin1());
		if(state == Target) {
			fastFailed = true;

			// now that fast has failed, tend to some pending things
			if(lateProxy) {
				if(!conn)
					doIncoming();
			}
			else {
				if(connSuccess)
					checkForActivation();
				else
					doConnectError();
			}
		}
		// initiator
		else {
			reset();
			if(wantRoleswitch && j->statusCode() == 300)
				error(ErrRoleswitch);
			else {
				if(j->statusCode() == 404)
					error(ErrConnect);
				else
					error(ErrRefused);
			}
		}
	}
}

void S5BManager::Item::conn_result(bool b)
{
	if(b) {
		SocksClient *sc = conn->takeClient();
		StreamHost h = conn->streamHostUsed();
		delete conn;
		conn = 0;
		connSuccess = true;

		printf("S5BManager::Item: %s [%s] successful outgoing connection\n", peer.full().latin1(), sid.latin1());

		connect(sc, SIGNAL(readyRead()), SLOT(sc_readyRead()));
		connect(sc, SIGNAL(bytesWritten(int)), SLOT(sc_bytesWritten(int)));
		connect(sc, SIGNAL(error(int)), SLOT(sc_error(int)));

		m->doSuccess(peer, in_id, h.jid());

		if(state == Initiator) {
			clientReady(sc);
			tryActivation();
		}
		else {
			clientList.append(sc);
			checkForActivation();
		}
	}
	else {
		delete conn;
		conn = 0;

		// if we delayed the proxies for later, try now
		if(lateProxy) {
			if(!fast || fastFailed)
				doIncoming();
		}
		else
			doConnectError();
	}
}

void S5BManager::Item::proxy_result(bool b)
{
	printf("proxy_result: %d\n", b);
	if(b) {
		SocksClient *sc = proxy_conn->takeClient();
		delete proxy_conn;
		proxy_conn = 0;

		connect(sc, SIGNAL(readyRead()), SLOT(sc_readyRead()));
		connect(sc, SIGNAL(bytesWritten(int)), SLOT(sc_bytesWritten(int)));
		connect(sc, SIGNAL(error(int)), SLOT(sc_error(int)));

		if(state == Initiator)
			clientReady(sc);
		else
			clientList.append(sc);

		// activate
		printf("activating proxy stream\n");
		proxy_task = new JT_S5B(m->client()->rootTask());
		connect(proxy_task, SIGNAL(finished()), SLOT(proxy_finished()));
		proxy_task->requestActivation(proxy.jid(), sid, peer);
		proxy_task->go(true);
	}
	else {
		delete proxy_conn;
		proxy_conn = 0;
		reset();
		error(ErrProxy);
	}
}

void S5BManager::Item::proxy_finished()
{
	JT_S5B *j = proxy_task;
	proxy_task = 0;

	if(j->success()) {
		printf("proxy stream activated\n");
		if(state == Initiator)
			tryActivation();
		else
			checkForActivation();
	}
	else {
		reset();
		error(ErrProxy);
	}
}

void S5BManager::Item::sc_readyRead()
{
	printf("sc_readyRead: %p\n", this);
	// only targets check for activation, and only should do it if there is no pending outgoing iq-set
	if(state == Target && !task)
		checkForActivation();
}

void S5BManager::Item::sc_bytesWritten(int)
{
	printf("sc_bytesWritten: %p\n", this);
	// this should only happen to the initiator, and should always be 1 byte (the '\r' sent earlier)
	finished();
}

void S5BManager::Item::sc_error(int)
{
	printf("sc_error: %p\n", this);
	SocksClient *sc = (SocksClient *)sender();

	if(sc == client) {
		reset();
		error(ErrConnect);
	}
	else {
		// drop
		clientList.removeRef(sc);
		delete sc;
	}
}

void S5BManager::Item::doConnectError()
{
	if(state == Target && fast) {
		if(conn || !fastFailed)
			return;
	}

	// try role-switch
	peerDoesRoleswitch = false; // TODO: support roleswitch
	if(peerDoesRoleswitch) {
		m->doError(peer, in_id, 300, "Role-switch");
		if(state == Target) {
			reset();
			error(ErrRoleswitch);
		}
	}
	else {
		m->doError(peer, in_id, 404, "Could not connect to given hosts");
		if(state == Target) {
			reset();
			error(ErrConnect);
		}
	}
}

void S5BManager::Item::tryActivation()
{
	printf("tryActivation\n");
	if(activated) {
		printf("already activated\n");
		return;
	}

	if(targetMode == NotFast) {
		printf("tryActivation: NotFast\n");
		// nothing to activate, we're done
		finished();
	}
	else if(targetMode == Fast) {
		printf("sending extra CR (%p)\n", client);
		// must send [CR] to activate target streamhost
		activated = true;
		QByteArray a(1);
		a[0] = '\r';
		client->write(a);
	}
}

void S5BManager::Item::checkForActivation()
{
	QPtrListIterator<SocksClient> it(clientList);
	for(SocksClient *sc; (sc = it.current()); ++it) {
		printf("checking for activation\n");
		if(fast) {
			printf("need CR (%p)\n", sc);
			if(sc->bytesAvailable() >= 1) {
				clientList.removeRef(sc);
				QByteArray a = sc->read(1);
				if(a[0] != '\r') {
					delete sc;
					return;
				}
				sc->disconnect(this);
				printf("activation success\n");
				clientReady(sc);
				break;
			}
		}
		else {
			printf("not fast mode, no need to wait for anything\n");
			clientList.removeRef(sc);
			sc->disconnect(this);
			clientReady(sc);
			break;
		}
	}

	if(client)
		finished();
}

void S5BManager::Item::finished()
{
	printf("finished: %p\n", this);
	client->disconnect(this);
	state = Active;
	printf("S5BManager::Item %s [%s] linked successfully\n", peer.full().latin1(), sid.latin1());
	connected();
}

//----------------------------------------------------------------------------
// S5BConnector
//----------------------------------------------------------------------------
class S5BConnector::Item : public QObject
{
	Q_OBJECT
public:
	SocksClient *client;
	StreamHost host;
	QString key;

	Item(const StreamHost &_host, const QString &_key) : QObject(0)
	{
		host = _host;
		key = _key;
		client = new SocksClient;
		connect(client, SIGNAL(connected()), SLOT(sc_connected()));
		connect(client, SIGNAL(error(int)), SLOT(sc_error(int)));
	}

	~Item()
	{
		delete client;
	}

	void start()
	{
		client->connectToHost(host.host(), host.port(), key, 0);
	}

signals:
	void result(bool);

private slots:
	void sc_connected()
	{
		printf("S5BConnector[%s]: success\n", host.host().latin1());
		client->disconnect(this);
		result(true);
	}

	void sc_error(int)
	{
		printf("S5BConnector[%s]: error\n", host.host().latin1());
		delete client;
		client = 0;
		result(false);
	}
};

class S5BConnector::Private
{
public:
	SocksClient *active;
	QPtrList<Item> itemList;
	QString key;
	StreamHost activeHost;
	QTimer t;
};

S5BConnector::S5BConnector(QObject *parent)
:QObject(parent)
{
	d = new Private;
	d->active = 0;
	d->itemList.setAutoDelete(true);
	connect(&d->t, SIGNAL(timeout()), SLOT(t_timeout()));
}

S5BConnector::~S5BConnector()
{
	reset();
	delete d;
}

void S5BConnector::reset()
{
	d->t.stop();
	delete d->active;
	d->active = 0;
	d->itemList.clear();
}

void S5BConnector::start(const StreamHostList &hosts, const QString &key)
{
	reset();

	printf("S5BConnector: starting [%p]!\n", this);
	for(StreamHostList::ConstIterator it = hosts.begin(); it != hosts.end(); ++it) {
		Item *i = new Item(*it, key);
		connect(i, SIGNAL(result(bool)), SLOT(item_result(bool)));
		d->itemList.append(i);
		i->start();
	}
	d->t.start(30000);
}

SocksClient *S5BConnector::takeClient()
{
	SocksClient *c = d->active;
	d->active = 0;
	return c;
}

StreamHost S5BConnector::streamHostUsed() const
{
	return d->activeHost;
}

void S5BConnector::item_result(bool b)
{
	Item *i = (Item *)sender();
	if(b) {
		d->active = i->client;
		i->client = 0;
		d->activeHost = i->host;
		d->itemList.clear();
		d->t.stop();
		printf("S5BConnector: complete! [%p]\n", this);
		result(true);
	}
	else {
		d->itemList.removeRef(i);
		if(d->itemList.isEmpty()) {
			d->t.stop();
			printf("S5BConnector: failed! [%p]\n", this);
			result(false);
		}
	}
}

void S5BConnector::t_timeout()
{
	reset();
	printf("S5BConnector: failed! (timeout)\n");
	result(false);
}

//----------------------------------------------------------------------------
// S5BServer
//----------------------------------------------------------------------------
class S5BServer::Item : public QObject
{
	Q_OBJECT
public:
	SocksClient *client;
	QString host;
	QTimer expire;

	Item(SocksClient *c) : QObject(0)
	{
		client = c;
		connect(client, SIGNAL(incomingMethods(int)), SLOT(sc_incomingMethods(int)));
		connect(client, SIGNAL(incomingRequest(const QString &, int)), SLOT(sc_incomingRequest(const QString &, int)));
		connect(client, SIGNAL(error(int)), SLOT(sc_error(int)));

		connect(&expire, SIGNAL(timeout()), SLOT(doError()));
		resetExpiration();
	}

	~Item()
	{
		delete client;
	}

	void resetExpiration()
	{
		expire.start(30000);
	}

signals:
	void result(bool);

private slots:
	void doError()
	{
		expire.stop();
		delete client;
		client = 0;
		result(false);
	}

	void sc_incomingMethods(int m)
	{
		if(m & SocksClient::AuthNone)
			client->chooseMethod(SocksClient::AuthNone);
		else
			doError();
	}

	void sc_incomingRequest(const QString &_host, int port)
	{
		if(port == 0) {
			host = _host;
			client->disconnect(this);
			result(true);
		}
		else
			doError();
	}

	void sc_error(int)
	{
		doError();
	}
};

class S5BServer::Private
{
public:
	SocksServer serv;
	QStringList hostList;
	QPtrList<S5BManager> manList;
	QPtrList<Item> itemList;
};

S5BServer::S5BServer(QObject *parent)
:QObject(parent)
{
	d = new Private;
	d->itemList.setAutoDelete(true);
	connect(&d->serv, SIGNAL(incomingReady()), SLOT(ss_incomingReady()));
}

S5BServer::~S5BServer()
{
	delete d;
}

bool S5BServer::isActive() const
{
	return d->serv.isActive();
}

bool S5BServer::start(int port)
{
	d->serv.stop();
	return d->serv.listen(port);
}

void S5BServer::stop()
{
	d->serv.stop();
}

void S5BServer::setHostList(const QStringList &list)
{
	d->hostList = list;
}

QStringList S5BServer::hostList() const
{
	return d->hostList;
}

int S5BServer::port() const
{
	return d->serv.port();
}

void S5BServer::ss_incomingReady()
{
	Item *i = new Item(d->serv.takeIncoming());
	connect(i, SIGNAL(result(bool)), SLOT(item_result(bool)));
	d->itemList.append(i);
}

void S5BServer::item_result(bool b)
{
	Item *i = (Item *)sender();
	if(!b) {
		d->itemList.removeRef(i);
		return;
	}

	SocksClient *c = i->client;
	i->client = 0;
	QString key = i->host;
	d->itemList.removeRef(i);

	// find the appropriate manager for this incoming connection
	QPtrListIterator<S5BManager> it(d->manList);
	for(S5BManager *m; (m = it.current()); ++it) {
		if(m->srv_ownsHash(key)) {
			m->srv_incomingReady(c, key);
			return;
		}
	}

	// throw it away
	delete c;
}

void S5BServer::link(S5BManager *m)
{
	d->manList.append(m);
}

void S5BServer::unlink(S5BManager *m)
{
	d->manList.removeRef(m);
}

const QPtrList<S5BManager> & S5BServer::managerList() const
{
	return d->manList;
}

//----------------------------------------------------------------------------
// JT_S5B
//----------------------------------------------------------------------------
class JT_S5B::Private
{
public:
	QDomElement iq;
	Jid to;
	Jid streamHost;
	StreamHost proxyInfo;
	int mode;
	QTimer t;
};

JT_S5B::JT_S5B(Task *parent)
:Task(parent)
{
	d = new Private;
	d->mode = -1;
	connect(&d->t, SIGNAL(timeout()), SLOT(t_timeout()));
}

JT_S5B::~JT_S5B()
{
	delete d;
}

void JT_S5B::request(const Jid &to, const QString &sid, const StreamHostList &hosts, bool roleswitch, bool fast)
{
	d->mode = 0;

	QDomElement iq;
	d->to = to;
	iq = createIQ(doc(), "set", to.full(), id());
	QDomElement query = doc()->createElement("query");
	query.setAttribute("xmlns", "http://jabber.org/protocol/bytestreams");
	query.setAttribute("sid", sid);
	iq.appendChild(query);
	for(StreamHostList::ConstIterator it = hosts.begin(); it != hosts.end(); ++it) {
		QDomElement shost = doc()->createElement("streamhost");
		shost.setAttribute("jid", (*it).jid().full());
		shost.setAttribute("host", (*it).host());
		shost.setAttribute("port", QString::number((*it).port()));
		if((*it).isProxy()) {
			QDomElement p = doc()->createElement("proxy");
			p.setAttribute("xmlns", "http://affinix.com/jabber/stream");
			shost.appendChild(p);
		}
		query.appendChild(shost);
	}
	if(roleswitch) {
		QDomElement e = doc()->createElement("roleswitch");
		e.setAttribute("xmlns", "http://affinix.com/jabber/stream");
		query.appendChild(e);
	}
	if(fast) {
		QDomElement e = doc()->createElement("fast");
		e.setAttribute("xmlns", "http://affinix.com/jabber/stream");
		query.appendChild(e);
	}
	d->iq = iq;
}

void JT_S5B::requestProxyInfo(const Jid &to)
{
	d->mode = 1;

	QDomElement iq;
	d->to = to;
	iq = createIQ(doc(), "get", to.full(), id());
	QDomElement query = doc()->createElement("query");
	query.setAttribute("xmlns", "http://jabber.org/protocol/bytestreams");
	iq.appendChild(query);
	d->iq = iq;
}

void JT_S5B::requestActivation(const Jid &to, const QString &sid, const Jid &target)
{
	d->mode = 2;

	QDomElement iq;
	d->to = to;
	iq = createIQ(doc(), "set", to.full(), id());
	QDomElement query = doc()->createElement("query");
	query.setAttribute("xmlns", "http://jabber.org/protocol/bytestreams");
	query.setAttribute("sid", sid);
	iq.appendChild(query);
	QDomElement act = doc()->createElement("activate");
	act.appendChild(doc()->createTextNode(target.full()));
	query.appendChild(act);
	d->iq = iq;
}

void JT_S5B::onGo()
{
	if(d->mode == 1)
		d->t.start(15000, true);
	send(d->iq);
}

void JT_S5B::onDisconnect()
{
	d->t.stop();
}

bool JT_S5B::take(const QDomElement &x)
{
	if(d->mode == -1)
		return false;

	if(!iqVerify(x, d->to, id()))
		return false;

	d->t.stop();

	if(x.attribute("type") == "result") {
		QDomElement q = queryTag(x);
		if(d->mode == 0) {
			d->streamHost = "";
			if(!q.isNull()) {
				QDomElement shost = q.elementsByTagName("streamhost-used").item(0).toElement();
				if(!shost.isNull())
					d->streamHost = shost.attribute("jid");
			}

			setSuccess();
		}
		else if(d->mode == 1) {
			if(!q.isNull()) {
				QDomElement shost = q.elementsByTagName("streamhost").item(0).toElement();
				if(!shost.isNull()) {
					Jid j = shost.attribute("jid");
					if(j.isValid()) {
						QString host = shost.attribute("host");
						// TODO: support zeroconf
						if(!host.isEmpty()) {
							int port = shost.attribute("port").toInt();
							StreamHost h;
							h.setJid(j);
							h.setHost(host);
							h.setPort(port);
							h.setIsProxy(true);
							d->proxyInfo = h;
						}
					}
				}
			}

			setSuccess();
		}
		else {
			setSuccess();
		}
	}
	else {
		setError(x);
	}

	return true;
}

void JT_S5B::t_timeout()
{
	d->mode = -1;
	setError(500, "Timed out");
}

Jid JT_S5B::streamHostUsed() const
{
	return d->streamHost;
}

StreamHost JT_S5B::proxyInfo() const
{
	return d->proxyInfo;
}

//----------------------------------------------------------------------------
// JT_PushS5B
//----------------------------------------------------------------------------
JT_PushS5B::JT_PushS5B(Task *parent)
:Task(parent)
{
}

JT_PushS5B::~JT_PushS5B()
{
}

bool JT_PushS5B::take(const QDomElement &e)
{
	// must be an iq-set tag
	if(e.tagName() != "iq")
		return false;
	if(e.attribute("type") != "set")
		return false;
	if(queryNS(e) != "http://jabber.org/protocol/bytestreams")
		return false;

	Jid from(e.attribute("from"));
	QDomElement q = queryTag(e);
	QString sid = q.attribute("sid");

	StreamHostList hosts;
	QDomNodeList nl = q.elementsByTagName("streamhost");
	for(uint n = 0; n < nl.count(); ++n) {
		QDomElement shost = nl.item(n).toElement();
		if(hosts.count() < 3) {
			Jid j = shost.attribute("jid");
			if(!j.isValid())
				continue;
			QString host = shost.attribute("host");
			// TODO: support zeroconf
			if(host.isEmpty())
				continue;
			int port = shost.attribute("port").toInt();
			QDomElement p = shost.elementsByTagName("proxy").item(0).toElement();
			bool isProxy = false;
			if(!p.isNull() && p.attribute("xmlns") == "http://affinix.com/jabber/stream")
				isProxy = true;

			StreamHost h;
			h.setJid(j);
			h.setHost(host);
			h.setPort(port);
			h.setIsProxy(isProxy);
			hosts += h;
		}
	}
	bool roleswitch = false;
	bool fast = false;
	QDomElement t;
	t = q.elementsByTagName("roleswitch").item(0).toElement();
	if(!t.isNull() && t.attribute("xmlns") == "http://affinix.com/jabber/stream")
		roleswitch = true;
	t = q.elementsByTagName("fast").item(0).toElement();
	if(!t.isNull() && t.attribute("xmlns") == "http://affinix.com/jabber/stream")
		fast = true;

	S5BRequest r;
	r.from = from;
	r.id = e.attribute("id");
	r.sid = sid;
	r.hosts = hosts;
	r.roleswitch = roleswitch;
	r.fast = fast;

	incoming(r);
	return true;
}

void JT_PushS5B::respondSuccess(const Jid &to, const QString &id, const Jid &streamHost)
{
	QDomElement iq = createIQ(doc(), "result", to.full(), id);
	QDomElement query = doc()->createElement("query");
	query.setAttribute("xmlns", "http://jabber.org/protocol/bytestreams");
	iq.appendChild(query);
	QDomElement shost = doc()->createElement("streamhost-used");
	shost.setAttribute("jid", streamHost.full());
	query.appendChild(shost);
	send(iq);
}

void JT_PushS5B::respondError(const Jid &to, const QString &id, int code, const QString &str)
{
	QDomElement iq = createIQ(doc(), "error", to.full(), id);
	QDomElement err = textTag(doc(), "error", str);
	err.setAttribute("code", QString::number(code));
	iq.appendChild(err);
	send(iq);
}

//----------------------------------------------------------------------------
// StreamHost
//----------------------------------------------------------------------------
StreamHost::StreamHost()
{
	v_port = -1;
	proxy = false;
}

const Jid & StreamHost::jid() const
{
	return j;
}

const QString & StreamHost::host() const
{
	return v_host;
}

int StreamHost::port() const
{
	return v_port;
}

const QString & StreamHost::zeroconf() const
{
	return v_zeroconf;
}

bool StreamHost::isProxy() const
{
	return proxy;
}

void StreamHost::setJid(const Jid &_j)
{
	j = _j;
}

void StreamHost::setHost(const QString &host)
{
	v_host = host;
}

void StreamHost::setPort(int port)
{
	v_port = port;
}

void StreamHost::setZeroconf(const QString &s)
{
	v_zeroconf = s;
}

void StreamHost::setIsProxy(bool b)
{
	proxy = b;
}

}

#include"s5b.moc"
