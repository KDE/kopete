/*
 * dtcp.cpp - direct connection protocol via tcp
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

#include"xmpp_dtcp.h"

#define DEBUG_DSH

#include<qcstring.h>
#include<qsocket.h>
#include<qtimer.h>
#include"ndns.h"
#include"xmpp_xmlcommon.h"

#include<stdlib.h>

using namespace Jabber;

static int num_dsh = 0;
static int id_dsh = 0;
static int num_conn = 0;
static int id_conn = 0;


//----------------------------------------------------------------------------
// DTCPConnection
//----------------------------------------------------------------------------
class DTCPConnection::Private
{
public:
	Private() {}

	DTCPManager *m;
	DTCPSocketHandler *sock;
	int state;
	Jid peer;
	QString keyA, keyB;
	DTCPOutgoing *out;
	JT_DTCP *j;
	QDomElement comment;
	QString iq_id;
	bool hasKeyB;
	bool remote;
	HostPortList hosts;
	bool localGaveUp, remoteGaveUp;
	int id;
	QTimer *t;
};

DTCPConnection::DTCPConnection(DTCPManager *m)
:ByteStream(m)
{
	d = new Private;
	d->m = m;
	d->j = 0;
	d->out = 0;
	d->sock = 0;
	d->remote = false;

	d->t = new QTimer;
	connect(d->t, SIGNAL(timeout()), SLOT(t_timeout()));

	reset();

	++num_conn;
	d->id = id_conn++;
	QString dstr; dstr.sprintf("DTCPConnection[%d]: constructing, count=%d\n", d->id, num_conn);
	d->m->client()->debug(dstr);
}

void DTCPConnection::reset(bool clear)
{
	d->m->unlink(this);

	d->t->stop();

	delete d->j;
	d->j = 0;
	delete d->out;
	d->out = 0;
	if(d->sock) {
		d->sock->close();
		if(clear) {
			delete d->sock;
			d->sock = 0;
		}
	}

	d->state = Idle;
	d->hasKeyB = false;
	d->localGaveUp = false;
	d->remoteGaveUp = false;
}

DTCPConnection::~DTCPConnection()
{
	reset(true);

	--num_conn;
	QString dstr; dstr.sprintf("DTCPConnection[%d]: destructing, count=%d\n", d->id, num_conn);
	d->m->client()->debug(dstr);

	delete d->t;
	delete d;
}

void DTCPConnection::connectToJid(const Jid &peer, const QDomElement &comment)
{
	close();
	delete d->sock;
	d->sock = 0;

	d->state = Requesting;
	d->peer = peer;
	d->keyA = d->m->genUniqueKey();
	d->comment = comment;
	d->remote = false;

	d->m->link(this);

	QString dstr; dstr.sprintf("DTCPConnection[%d]: initiating request %s [%s]\n", d->id, peer.full().latin1(), d->keyA.latin1());
	d->m->client()->debug(dstr);

	d->j = new JT_DTCP(d->m->client()->rootTask());
	connect(d->j, SIGNAL(finished()), SLOT(dtcp_finished()));

	d->hosts.clear();
	DTCPServer *serv = d->m->server();
	if(serv && serv->isActive()) {
		QStringList hostList = serv->hostList();
		for(QStringList::ConstIterator it = hostList.begin(); it != hostList.end(); ++it)
			d->hosts += HostPort(*it, serv->port());
	}

	// if we don't send hosts, assume the remote gives up connecting
	if(d->hosts.isEmpty())
		d->remoteGaveUp = true;

	d->j->request(d->peer, d->keyA, d->hosts, comment);
	d->j->go(true);
}

void DTCPConnection::accept()
{
	if(d->state != WaitingForAccept)
		return;

	d->state = Connecting;

	// connect in 60 seconds
	d->t->start(60000, true);

	QString dstr; dstr.sprintf("DTCPConnection[%d]: accepting %s [%s]\n", d->id, d->peer.full().latin1(), d->keyA.latin1());
	d->m->client()->debug(dstr);

	HostPortList hl;
	DTCPServer *serv = d->m->server();
	if(serv && serv->isActive()) {
		QStringList hostList = serv->hostList();
		for(QStringList::ConstIterator it = hostList.begin(); it != hostList.end(); ++it)
			hl += HostPort(*it, serv->port());
	}
	if(hl.isEmpty())
		d->remoteGaveUp = true;

	d->m->doAccept(this, d->iq_id, hl);

	QTimer::singleShot(0, this, SLOT(postAccept()));
}

void DTCPConnection::postAccept()
{
	if(!d->hosts.isEmpty()) {
		d->out = new DTCPOutgoing(d->m);
		connect(d->out, SIGNAL(result(bool)), SLOT(out_result(bool)));
		d->out->start(d->hosts, d->peer, d->keyA, d->keyB, !d->remote);
	}
	else {
		d->localGaveUp = true;
		checkGaveUp();
	}
}

void DTCPConnection::close()
{
	if(d->state == Idle)
		return;

	if(d->state == Connecting)
		d->m->sendError(d->peer, d->keyB, 500, "Closed");
	else if(d->state == WaitingForAccept)
		d->m->doReject(this, d->iq_id, 403, "Rejected");

	reset();

	QString dstr; dstr.sprintf("DTCPConnection[%d]: closing %s [%s]\n", d->id, d->peer.full().latin1(), d->keyA.latin1());
	d->m->client()->debug(dstr);
}

void DTCPConnection::setSocketHandler(DTCPSocketHandler *h)
{
	d->sock = h;
	connect(d->sock, SIGNAL(connectionClosed()), SLOT(dsh_connectionClosed()));
	connect(d->sock, SIGNAL(delayedCloseFinished()), SLOT(dsh_delayedCloseFinished()));
	connect(d->sock, SIGNAL(readyRead()), SLOT(dsh_readyRead()));
	connect(d->sock, SIGNAL(bytesWritten(int)), SLOT(dsh_bytesWritten(int)));
	connect(d->sock, SIGNAL(error(int)), SLOT(dsh_error(int)));
}

void DTCPConnection::setIncomingHandler(DTCPSocketHandler *h)
{
	// stop any outgoing attempts
	delete d->out;
	d->out = 0;

	d->state = Active;
	d->peer = h->peer();
	d->keyA = h->localKey();

	setSocketHandler(h);

	QString dstr; dstr.sprintf("DTCPConnection[%d]: %s [%s] received successfully\n", d->id, d->peer.full().latin1(), d->keyA.latin1());
	d->m->client()->debug(dstr);

	d->t->stop();
	QTimer::singleShot(0, this, SLOT(postConnect()));
	connected();
}

void DTCPConnection::waitForAccept(const Jid &peer, const QString &localKey, const QString &remoteKey, const HostPortList &hosts, const QDomElement &comment, const QString &iq_id)
{
	close();
	delete d->sock;
	d->sock = 0;

	d->state = WaitingForAccept;
	d->peer = peer;
	d->keyA = localKey;
	d->keyB = remoteKey;
	d->hosts = hosts;
	d->comment = comment;
	d->iq_id = iq_id;

	d->hasKeyB = true;
	d->remote = true;

	d->m->link(this);
}

bool DTCPConnection::isRemote() const
{
	return d->remote;
}

int DTCPConnection::state() const
{
	return d->state;
}

Jid DTCPConnection::peer() const
{
	return d->peer;
}

QString DTCPConnection::localKey() const
{
	return d->keyA;
}

QString DTCPConnection::remoteKey() const
{
	return d->keyB;
}

bool DTCPConnection::hasRemoteKey() const
{
	return d->hasKeyB;
}

QDomElement DTCPConnection::comment() const
{
	return d->comment;
}

bool DTCPConnection::isConnected() const
{
	if(d->state == Active)
		return true;
	else
		return false;
}

void DTCPConnection::write(const QByteArray &buf)
{
	if(d->state == Active)
		d->sock->write(buf);
}

QByteArray DTCPConnection::read()
{
	if(d->sock)
		return d->sock->read();
	else
		return QByteArray();
}

bool DTCPConnection::canRead() const
{
	if(d->sock)
		return d->sock->canRead();
	else
		return false;
}

int DTCPConnection::bytesToWrite() const
{
	if(d->state == Active)
		return d->sock->bytesToWrite();
	else
		return 0;
}

void DTCPConnection::dsh_connectionClosed()
{
	reset();
	connectionClosed();
}

void DTCPConnection::dsh_delayedCloseFinished()
{
	// echo
	delayedCloseFinished();
}

void DTCPConnection::dsh_readyRead()
{
	// echo
	readyRead();
}

void DTCPConnection::dsh_bytesWritten(int x)
{
	// echo
	bytesWritten(x);
}

void DTCPConnection::dsh_error(int)
{
	reset();
	error(ErrSocket);
}

void DTCPConnection::dtcp_finished()
{
	JT_DTCP *j = d->j;
	d->j = 0;

	if(j->success()) {
		d->keyB = j->key();
		d->hosts = j->hostList();
		d->hasKeyB = true;

		QString dstr; dstr.sprintf("DTCPConnection[%d]: %s [%s] accepted. (%s)\n", d->id, d->peer.full().latin1(), d->keyA.latin1(), d->keyB.latin1());
		d->m->client()->debug(dstr);

		d->state = Connecting;
		accepted();

		// connect in 60 seconds
		d->t->start(60000, true);

		// there might be waiting connections that need to be processed
		QTimer::singleShot(0, this, SLOT(postContinue()));
		d->m->continueAfterWait(d->keyA);
	}
	else {
		QString dstr; dstr.sprintf("DTCPConnection[%d]: %s [%s] refused.\n", d->id, d->peer.full().latin1(), d->keyA.latin1());
		d->m->client()->debug(dstr);

		reset(true);
		error(ErrRequest);
	}
}

void DTCPConnection::out_result(bool b)
{
	if(b) {
		d->state = Active;
		DTCPSocketHandler *h = d->out->takeHandler();
		delete d->out;
		d->out = 0;

		setSocketHandler(h);

		QString dstr; dstr.sprintf("DTCPConnection[%d]: %s [%s] connected successfully.\n", d->id, d->peer.full().latin1(), d->keyA.latin1());
		d->m->client()->debug(dstr);

		d->t->stop();
		QTimer::singleShot(0, this, SLOT(postConnect()));
		connected();
	}
	else {
		d->m->sendError(d->peer, d->keyB, 502, "Could not connect to given hosts");
		d->localGaveUp = true;
		checkGaveUp();
	}
}

void DTCPConnection::postConnect()
{
	if(d->sock->canRead())
		readyRead();
}

void DTCPConnection::postContinue()
{
	if(!isConnected()) {
		if(!d->hosts.isEmpty()) {
			d->out = new DTCPOutgoing(d->m);
			connect(d->out, SIGNAL(result(bool)), SLOT(out_result(bool)));
			d->out->start(d->hosts, d->peer, d->keyA, d->keyB, !d->remote);
		}
		else {
			d->localGaveUp = true;
			checkGaveUp();
		}
	}
}

void DTCPConnection::onError(int, const QString &str)
{
	// don't care about errors unless we are connecting
	if(d->state == Connecting) {
		QString dstr; dstr.sprintf("DTCPConnection[%d]: %s - remote gave up.  Reason: [%s]\n", d->id, d->peer.full().latin1(), str.latin1());
		d->m->client()->debug(dstr);

		d->remoteGaveUp = true;
		checkGaveUp();
	}
}

void DTCPConnection::checkGaveUp()
{
	if(!d->localGaveUp || !d->remoteGaveUp)
		return;

	QString dstr; dstr.sprintf("DTCPConnection[%d]: %s [%s] link failed.\n", d->id, d->peer.full().latin1(), d->keyA.latin1());
	d->m->client()->debug(dstr);

	reset(true);
	error(ErrConnect);
}

void DTCPConnection::t_timeout()
{
	reset(true);
	error(ErrConnect);
}


//----------------------------------------------------------------------------
// DTCPManager
//----------------------------------------------------------------------------
class DTCPManager::Private
{
public:
	Private() {}

	Client *client;
	DTCPServer *serv;
	DTCPConnectionList activeConns;
	DTCPConnectionList incomingConns;
	JT_PushDTCP *pdtcp;
};

DTCPManager::DTCPManager(Client *parent)
:QObject(parent)
{
	d = new Private;
	d->client = parent;
	d->serv = 0;

	d->pdtcp = new JT_PushDTCP(d->client->rootTask());
	connect(d->pdtcp, SIGNAL(incoming(const Jid &, const QString &, const QString &, const HostPortList &, const QDomElement &)), SLOT(pdtcp_incoming(const Jid &, const QString &, const QString &, const HostPortList &, const QDomElement &)));
	connect(d->pdtcp, SIGNAL(error(const Jid &, const QString &, int, const QString &)), SLOT(pdtcp_error(const Jid &, const QString &, int, const QString &)));
}

DTCPManager::~DTCPManager()
{
	setServer(0);
	d->incomingConns.setAutoDelete(true);
	d->incomingConns.clear();
	delete d->pdtcp;
	delete d;
}

Client *DTCPManager::client() const
{
	return d->client;
}

DTCPServer *DTCPManager::server() const
{
	return d->serv;
}

void DTCPManager::setServer(DTCPServer *serv)
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

DTCPConnection *DTCPManager::takeIncoming()
{
	if(d->incomingConns.isEmpty())
		return 0;

	DTCPConnection *c = d->incomingConns.getFirst();
	d->incomingConns.removeRef(c);
	return c;
}

void DTCPManager::pdtcp_incoming(const Jid &from, const QString &id, const QString &key, const HostPortList &hosts, const QDomElement &comment)
{
	DTCPConnection *c = findRemoteConnection(from, key);
	if(c) {
		d->pdtcp->respondError(from, id, 403, "Key in use");
	}
	else {
		QString keyA = genUniqueKey();

		// create a "waiting" connection
		DTCPConnection *c = new DTCPConnection(this);
		c->waitForAccept(from, keyA, key, hosts, comment, id);
		d->incomingConns.append(c);
		incomingReady();
	}
}

void DTCPManager::pdtcp_error(const Jid &, const QString &key, int code, const QString &str)
{
	DTCPConnection *c = findConnection(key);
	if(!c)
		return;
	c->onError(code, str);
}

void DTCPManager::doAccept(DTCPConnection *c, const QString &id, const HostPortList &hosts)
{
	d->pdtcp->respondSuccess(c->peer(), id, c->localKey(), hosts);
}

void DTCPManager::doReject(DTCPConnection *c, const QString &id, int code, const QString &str)
{
	d->pdtcp->respondError(c->peer(), id, code, str);
}

DTCPConnection *DTCPManager::findConnection(const QString &key) const
{
	DTCPConnectionListIt it(d->activeConns);
	for(DTCPConnection *c; (c = it.current()); ++it) {
		if(c->localKey() == key)
			return c;
	}
	return 0;
}

DTCPConnection *DTCPManager::findRemoteConnection(const Jid &peer, const QString &key) const
{
	DTCPConnectionListIt it(d->activeConns);
	for(DTCPConnection *c; (c = it.current()); ++it) {
		if(c->peer().compare(peer) && c->remoteKey() == key)
			return c;
	}
	return 0;
}

QString DTCPManager::genKey() const
{
	QString key = "dtcp_";

	for(int i = 0; i < 4; ++i) {
		int word = rand() & 0xffff;
		for(int n = 0; n < 4; ++n) {
			QString s;
			s.sprintf("%x", (word >> (n * 4)) & 0xf);
			key.append(s);
		}
	}

	return key;
}

QString DTCPManager::genUniqueKey() const
{
	// get unused key
	QString key;
	while(1) {
		key = genKey();

		// if we have a server, then check through it
		if(d->serv) {
			if(!d->serv->findConnection(key))
				break;
		}
		else {
			if(!findConnection(key))
				break;
		}
	}

	return key;
}

void DTCPManager::link(DTCPConnection *c)
{
	d->activeConns.append(c);
}

void DTCPManager::unlink(DTCPConnection *c)
{
	d->activeConns.removeRef(c);
}

void DTCPManager::sendError(const Jid &to, const QString &key, int code, const QString &str)
{
	QDomElement iq = createIQ(d->client->doc(), "error", to.full(), "");
	QDomElement query = d->client->doc()->createElement("query");
	query.setAttribute("xmlns", "http://jabber.org/protocol/dtcp");
	iq.appendChild(query);
	query.appendChild(textTag(d->client->doc(), "key", key));
	QDomElement err = textTag(d->client->doc(), "error", str);
	err.setAttribute("code", QString::number(code));
	iq.appendChild(err);
	d->client->send(iq);
}

void DTCPManager::continueAfterWait(const QString &key)
{
	if(d->serv)
		d->serv->continueAfterWait(key);
}


//----------------------------------------------------------------------------
// DTCPOutgoing
//----------------------------------------------------------------------------
class DTCPOutgoing::Private
{
public:
	Private() {}

	DTCPManager *m;
	HostPortList hosts;
	Jid peer;
	QString keyA, keyB;
	QPtrList<DTCPSocketHandler> socks;
	DTCPSocketHandler *act;
	bool requestor;
};

DTCPOutgoing::DTCPOutgoing(DTCPManager *m)
:QObject(0)
{
	d = new Private;
	d->m = m;
	d->act = 0;
}

DTCPOutgoing::~DTCPOutgoing()
{
	reset();
	delete d;
}

void DTCPOutgoing::reset()
{
	stop();
	delete d->act;
	d->act = 0;
}

void DTCPOutgoing::start(const HostPortList &hosts, const Jid &peer, const QString &keyA, const QString &keyB, bool requestor)
{
	reset();

	d->hosts = hosts;
	d->peer = peer;
	d->keyA = keyA;
	d->keyB = keyB;
	d->requestor = requestor;

	QString dstr = "DTCPOutgoing: trying ";
	bool first = true;
	HostPortList::ConstIterator it;
	for(it = d->hosts.begin(); it != d->hosts.end(); ++it) {
		const HostPort &hp = *it;
		if(!first)
			dstr += ", ";
		dstr += hp.host() + ':' + QString::number(hp.port());
		first = false;
	}
	dstr += '\n';
	d->m->client()->debug(dstr);

	// connect to all hosts at the same time
	for(it = d->hosts.begin(); it != d->hosts.end(); ++it) {
		const HostPort &hp = *it;
		DTCPSocketHandler *s = new DTCPSocketHandler(d->m);
		d->socks.append(s);
		connect(s, SIGNAL(connected()), SLOT(dsh_connected()));
		connect(s, SIGNAL(error(int)), SLOT(dsh_error(int)));
		s->handle(hp.host(), hp.port(), d->peer, d->keyA, d->keyB, d->requestor);
	}
}

void DTCPOutgoing::stop()
{
	d->socks.setAutoDelete(true);
	d->socks.clear();
	d->socks.setAutoDelete(false);
	d->hosts.clear();
}

DTCPSocketHandler *DTCPOutgoing::takeHandler() const
{
	DTCPSocketHandler *h = d->act;
	d->act = 0;
	return h;
}

void DTCPOutgoing::conn()
{
	if(d->hosts.isEmpty())
		return;
}

void DTCPOutgoing::dsh_connected()
{
	DTCPSocketHandler *s = (DTCPSocketHandler *)sender();
	d->socks.removeRef(s);

	stop();

	QString dstr; dstr.sprintf("DTCPOutgoing: success with %s:%d\n", s->host().latin1(), s->port());
	d->m->client()->debug(dstr);

	d->act = s;
	result(true);
}

void DTCPOutgoing::dsh_error(int)
{
	DTCPSocketHandler *s = (DTCPSocketHandler *)sender();
	d->socks.removeRef(s);
	delete s;

	// if no more sockets then we bail
	if(d->socks.isEmpty())
		result(false);
}


//----------------------------------------------------------------------------
// DTCPServer
//----------------------------------------------------------------------------
class DTCPServer::Private
{
public:
	Private() {}

	ServSock *serv;
	QStringList hostList;
	QPtrList<DTCPManager> manList;
	QPtrList<DTCPSocketHandler> dshList;
};

DTCPServer::DTCPServer(QObject *parent)
:QObject(parent)
{
	d = new Private;
	d->serv = 0;
}

DTCPServer::~DTCPServer()
{
	d->dshList.setAutoDelete(true);
	d->dshList.clear();
	delete d->serv;
	delete d;
}

bool DTCPServer::isActive() const
{
	return (d->serv ? true: false);
}

bool DTCPServer::listen(int port)
{
	delete d->serv;
	d->serv = 0;

	if(port == -1)
		return false;

	d->serv = new ServSock(port);
	if(!d->serv->ok()) {
		delete d->serv;
		d->serv = 0;
		return false;
	}
	connect(d->serv, SIGNAL(connectionReady(int)), SLOT(connectionReady(int)));

	return true;
}

void DTCPServer::setHostList(const QStringList &list)
{
	d->hostList = list;
}

QStringList DTCPServer::hostList() const
{
	return d->hostList;
}

int DTCPServer::port() const
{
	if(d->serv)
		return d->serv->port();
	else
		return -1;
}

void DTCPServer::connectionReady(int s)
{
	DTCPSocketHandler *h = new DTCPSocketHandler(this);
	connect(h, SIGNAL(connected()), SLOT(dsh_connected()));
	connect(h, SIGNAL(error(int)), SLOT(dsh_error(int)));
	d->dshList.append(h);
	h->handle(s);
}

void DTCPServer::dsh_connected()
{
	DTCPSocketHandler *h = (DTCPSocketHandler *)sender();
	d->dshList.removeRef(h);

	DTCPConnection *c = findConnection(h->localKey());
	if(c && !c->isConnected())
		c->setIncomingHandler(h);
	else
		delete h;
}

void DTCPServer::dsh_error(int)
{
	DTCPSocketHandler *h = (DTCPSocketHandler *)sender();
	d->dshList.removeRef(h);
	delete h;
}

void DTCPServer::link(DTCPManager *m)
{
	d->manList.append(m);
}

void DTCPServer::unlink(DTCPManager *m)
{
	d->manList.removeRef(m);
}

DTCPConnection *DTCPServer::findConnection(const QString &key) const
{
	QPtrListIterator<DTCPManager> it(d->manList);
	for(DTCPManager *m; (m = it.current()); ++it) {
		DTCPConnection *c = m->findConnection(key);
		if(c)
			return c;
	}
	return 0;
}

void DTCPServer::continueAfterWait(const QString &key)
{
	QPtrListIterator<DTCPSocketHandler> it(d->dshList);
	for(DTCPSocketHandler *dsh; (dsh = it.current()); ++it) {
		if(dsh->isWaiting() && dsh->localKey() == key)
			dsh->continueAfterWait();
	}
}


//----------------------------------------------------------------------------
// DTCPSocketHandler
//----------------------------------------------------------------------------
class DTCPSocketHandler::Private
{
public:
	Private() {}

	DTCPManager *m;
	DTCPServer *s;
	int mode;
	QSocket *sock;
	Jid peer;
	QString keyA, keyB;
	NDns ndns;
	QString host;
	int port;
	bool established;
	QByteArray recvbuf;
	bool requestor;
	bool waiting;
	int step;
	int id;
	QTimer *t;
};

DTCPSocketHandler::DTCPSocketHandler(DTCPManager *m)
:QObject(0)
{
	init();
	d->m = m;
	d->mode = Client;

#ifdef DEBUG_DSH
	printf("DSH[%d] - constructing Client, count=%d\n", d->id, num_dsh);
#endif
}

DTCPSocketHandler::DTCPSocketHandler(DTCPServer *s)
:QObject(0)
{
	init();
	d->s = s;
	d->mode = Server;

#ifdef DEBUG_DSH
	printf("DSH[%d] - constructing Server, count=%d\n", d->id, num_dsh);
#endif
}

void DTCPSocketHandler::init()
{
	++num_dsh;
	d = new Private;
	d->m = 0;
	d->s = 0;
	d->id = id_dsh++;

	connect(&d->ndns, SIGNAL(resultsReady()), SLOT(ndns_done()));

	d->sock = new QSocket;
	connect(d->sock, SIGNAL(connected()), SLOT(sock_connected()));
	connect(d->sock, SIGNAL(connectionClosed()), SLOT(sock_connectionClosed()));
	connect(d->sock, SIGNAL(delayedCloseFinished()), SLOT(sock_delayedCloseFinished()));
	connect(d->sock, SIGNAL(readyRead()), SLOT(sock_readyRead()));
	connect(d->sock, SIGNAL(bytesWritten(int)), SLOT(sock_bytesWritten(int)));
	connect(d->sock, SIGNAL(error(int)), SLOT(sock_error(int)));

	d->t = new QTimer;
	connect(d->t, SIGNAL(timeout()), SLOT(t_timeout()));

	reset(true);
}

void DTCPSocketHandler::reset(bool clear)
{
	if(d->sock->state() != QSocket::Idle)
		d->sock->close();
	d->t->stop();
	d->ndns.stop();
	d->established = false;
	d->requestor = false;
	d->waiting = false;
	d->step = 0;

	if(clear)
		d->recvbuf.resize(0);
}

void DTCPSocketHandler::serverReset()
{
	// reset important variables, but keep the connection alive
	d->step = 0;
	d->keyA = "";
	d->keyB = "";
	d->peer = "";
	d->waiting = false;
}

DTCPSocketHandler::~DTCPSocketHandler()
{
	delete d->t;
	delete d->sock;

	--num_dsh;
#ifdef DEBUG_DSH
	printf("DSH[%d] - destructing, count=%d\n", d->id, num_dsh);
#endif
	delete d;
}

void DTCPSocketHandler::handle(const QString &host, int port, const Jid &peer, const QString &keyA, const QString &keyB, bool requestor)
{
	reset(true);
	d->host = host;
	d->port = port;
	d->peer = peer;
	d->keyA = keyA;
	d->keyB = keyB;
	d->requestor = requestor;

	d->t->start(30000, true);
#ifdef DEBUG_DSH
	printf("DSH[%d] - connecting...\n", d->id);
#endif
	d->ndns.resolve(d->host.latin1());
}

void DTCPSocketHandler::handle(int s)
{
	reset(true);

	d->t->start(30000, true);
#ifdef DEBUG_DSH
	printf("DSH[%d] - serving...\n", d->id);
#endif
	d->sock->setSocket(s);
	if(d->sock->bytesAvailable() > 0)
		sock_readyRead();
}

void DTCPSocketHandler::close()
{
	if(d->sock->bytesToWrite() == 0)
		reset();
	else
		d->sock->close();
}

bool DTCPSocketHandler::isWaiting() const
{
	return d->waiting;
}

QString DTCPSocketHandler::localKey() const
{
	return d->keyA;
}

QString DTCPSocketHandler::host() const
{
	return d->host;
}

int DTCPSocketHandler::port() const
{
	return d->port;
}

int DTCPSocketHandler::mode() const
{
	return d->mode;
}

Jid DTCPSocketHandler::peer() const
{
	return d->peer;
}

bool DTCPSocketHandler::isConnected() const
{
	return d->established;
}

void DTCPSocketHandler::write(const QByteArray &buf)
{
	if(d->established)
		d->sock->writeBlock(buf.data(), buf.size());
}

QByteArray DTCPSocketHandler::read()
{
	QByteArray a;

	if(canRead()) {
		a = d->recvbuf;
		a.detach();
		d->recvbuf.resize(0);
	}

	return a;
}

bool DTCPSocketHandler::canRead() const
{
	return (d->recvbuf.size() > 0 ? true: false);
}

int DTCPSocketHandler::bytesToWrite() const
{
	if(d->established)
		return d->sock->bytesToWrite();
	else
		return 0;
}

void DTCPSocketHandler::ndns_done()
{
	if(d->ndns.result() == 0) {
		doError(ErrConnect);
		return;
	}

	d->sock->connectToHost(d->ndns.resultString(), d->port);
}

void DTCPSocketHandler::sock_connected()
{
#ifdef DEBUG_DSH
	printf("DSH[%d] - connected\n", d->id);
#endif
	writeLine(QString("key:") + d->keyB);
}

void DTCPSocketHandler::sock_connectionClosed()
{
	if(d->established) {
		reset();
		connectionClosed();
	}
	else {
		doError(ErrHandshake);
	}
}

void DTCPSocketHandler::sock_delayedCloseFinished()
{
	if(d->established) {
		reset();
		delayedCloseFinished();
	}
}

void DTCPSocketHandler::sock_readyRead()
{
	// read in the block
	QByteArray block;
	int len = d->sock->bytesAvailable();
	if(len < 1)
		len = 1024; // zero bytes available?  we'll assume a bogus value and default to 1024
	block.resize(len);
	int actual = d->sock->readBlock(block.data(), len);
	if(actual < 1)
		return;
	block.resize(actual);

#ifdef DEBUG_DSH
	printf("DSH[%d] - read %d byte(s)\n", d->id, block.size());
#endif

	int oldsize = d->recvbuf.size();
	d->recvbuf.resize(oldsize + block.size());
	memcpy(d->recvbuf.data() + oldsize, block.data(), block.size());

	if(d->established)
		readyRead();
	else {
		// process incoming lines
		while(1) {
			bool found;
			QString line = extractLine(&d->recvbuf, &found);
			if(!found)
				break;

			if(!processLine(line))
				break;
		}
	}
}

void DTCPSocketHandler::sock_bytesWritten(int x)
{
	if(d->established) {
		// echo
		bytesWritten(x);
	}
}

void DTCPSocketHandler::sock_error(int x)
{
	if(x == QSocket::ErrSocketRead)
		doError(ErrSocket);
	else
		doError(ErrConnect);
}

void DTCPSocketHandler::doSuccess()
{
#ifdef DEBUG_DSH
	printf("DSH[%d] - *** established *** !\n", d->id);
#endif

	d->t->stop();
	d->established = true;
	QTimer::singleShot(0, this, SLOT(postConnect()));
	connected();
}

void DTCPSocketHandler::postConnect()
{
	if(!d->recvbuf.isEmpty())
		readyRead();
}

void DTCPSocketHandler::doError(int err)
{
#ifdef DEBUG_DSH
	printf("DSH[%d] - error [%d]\n", d->id, err);
#endif

	reset();
	error(err);
}

void DTCPSocketHandler::writeLine(const QString &str)
{
	if(d->sock->state() != QSocket::Connected)
		return;

	QCString cstr = str.utf8() + '\n';
	d->sock->writeBlock(cstr.data(), cstr.length());
#ifdef DEBUG_DSH
	printf("DSH[%d] - write [%s]\n", d->id, str.latin1());
#endif
}

QString DTCPSocketHandler::extractLine(QByteArray *buf, bool *found) const
{
	// scan for newline
	int n;
	for(n = 0; n < (int)buf->size(); ++n) {
		if(buf->at(n) == '\n') {
			QCString cstr;
			cstr.resize(n+1);
			memcpy(cstr.data(), buf->data(), n);
			++n; // hack off LF

			memmove(buf->data(), buf->data() + n, buf->size() - n);
			buf->resize(buf->size() - n);
			QString s = QString::fromUtf8(cstr);

			if(found)
				*found = true;
			return s;
		}
	}

	if(found)
		*found = false;
	return "";
}

bool DTCPSocketHandler::processLine(const QString &line)
{
#ifdef DEBUG_DSH
	printf("DSH[%d] - read [%s]\n", d->id, line.latin1());
#endif

	// separate into cmd:str
	QString cmd, str;
	int n = line.find(':');
	if(n == -1) {
		cmd = line;
		str = "";
	}
	else {
		cmd = line.mid(0, n);
		str = line.mid(n+1);
	}

	// client
	if(d->mode == Client) {
		if(cmd != "ok" || str != d->keyA) {
			doError(ErrHandshake);
			return false;
		}

		DTCPConnection *c = d->m->findConnection(d->keyA);
		if(!c || c->isConnected()) {
			doError(ErrHandshake);
			return false;
		}

		if(d->requestor)
			writeLine("ok");

		doSuccess();
		return false;
	}
	// server
	else {
		// step 0 -- get 'key'
		if(d->step == 0) {
			if(cmd == "key") {
				if(validate(str))
					return false;
			}
			else
				writeLine("error:bad input");
		}
		// step 1 -- get 'ok'
		else {
			if(cmd == "ok") {
				doSuccess();
				return false;
			}
			else {
				serverReset();
				writeLine("error:bad input");
			}
		}
	}

	return true;
}

bool DTCPSocketHandler::validate(const QString &keyA)
{
#ifdef DEBUG_DSH
	printf("DSH[%d] - validating [%s]\n", d->id, keyA.latin1());
#endif
	DTCPConnection *c = d->s->findConnection(keyA);
	if(!c || c->isConnected()) {
		serverReset();
		writeLine("error:no such key or key active");
		return false;
	}

	// record some known information
	d->peer = c->peer();
	d->keyA = keyA;
	d->requestor = !c->isRemote();

	// no remote key yet?
	if(!c->hasRemoteKey()) {
#ifdef DEBUG_DSH
		printf("DSH[%d] - no remote key yet.  waiting ...\n", d->id);
#endif
		d->waiting = true;
		return false;
	}
	d->waiting = false;

	// take the key!
	d->keyB = c->remoteKey();

	writeLine(QString("ok:") + d->keyB);

	if(d->requestor)
		doSuccess();
	else
		++d->step;

	return true;
}

void DTCPSocketHandler::continueAfterWait()
{
	if(!d->waiting)
		return;

	validate(d->keyA);
}

void DTCPSocketHandler::t_timeout()
{
	doError(ErrTimeout);
}


//----------------------------------------------------------------------------
// JT_DTCP
//----------------------------------------------------------------------------
class JT_DTCP::Private
{
public:
	Private() {}

	QDomElement iq;
	Jid to;
	HostPortList hostList;
	QString key;
};

JT_DTCP::JT_DTCP(Task *parent)
:Task(parent)
{
	d = new Private;
}

JT_DTCP::~JT_DTCP()
{
	delete d;
}

void JT_DTCP::request(const Jid &to, const QString &key, const HostPortList &hosts, const QDomElement &comment)
{
	QDomElement iq;
	d->to = to;
	iq = createIQ(doc(), "set", to.full(), id());
	QDomElement query = doc()->createElement("query");
	query.setAttribute("xmlns", "http://jabber.org/protocol/dtcp");
	iq.appendChild(query);
	query.appendChild(textTag(doc(), "key", key));
	for(HostPortList::ConstIterator it = hosts.begin(); it != hosts.end(); ++it)
		query.appendChild( textTag(doc(), "host", (*it).host() + ':' + QString::number((*it).port())) );
	query.appendChild(comment);
	d->iq = iq;
}

void JT_DTCP::onGo()
{
	send(d->iq);
}

bool JT_DTCP::take(const QDomElement &x)
{
	Jid from(x.attribute("from"));
	if(x.attribute("id") != id() || !d->to.compare(from))
		return false;

	if(x.attribute("type") == "result") {
		QDomElement q = queryTag(x);
		bool found;
		QDomElement k = findSubTag(q, "key", &found);
		if(found)
			d->key = tagContent(k);
		d->hostList.clear();
		for(QDomNode n = q.firstChild(); !n.isNull(); n = n.nextSibling()) {
			QDomElement i = n.toElement();
			if(i.isNull())
				continue;

			if(i.tagName() == "host" && d->hostList.count() < 3) {
				QString str = tagContent(i);
				int n = str.find(':');
				QString host;
				int port;
				if(n == -1) {
					host = str;
					port = 0;
				}
				else {
					host = str.mid(0, n);
					port = str.mid(n+1).toInt();
				}
				d->hostList += HostPort(host, port);
			}
		}

		setSuccess();
	}
	else {
		setError(x);
	}

	return true;
}

HostPortList JT_DTCP::hostList() const
{
	return d->hostList;
}

QString JT_DTCP::key() const
{
	return d->key;
}

Jid JT_DTCP::jid() const
{
	return d->to;
}


//----------------------------------------------------------------------------
// JT_PushDTCP
//----------------------------------------------------------------------------
JT_PushDTCP::JT_PushDTCP(Task *parent)
:Task(parent)
{
}

JT_PushDTCP::~JT_PushDTCP()
{
}

bool JT_PushDTCP::take(const QDomElement &e)
{
	// must be an iq-set tag
	if(e.tagName() != "iq")
		return false;

	bool ok = false;
	if(e.attribute("type") == "set")
		ok = true;
	else if(e.attribute("type") == "error" && !e.hasAttribute("id"))
		ok = true;
	if(!ok)
		return false;
	if(queryNS(e) != "http://jabber.org/protocol/dtcp")
		return false;

	Jid from(e.attribute("from"));
	QDomElement q = queryTag(e);
	bool found;
	QDomElement k = findSubTag(q, "key", &found);
	if(!found)
		return true;
	QString key = tagContent(k);

	QString type = e.attribute("type");
	if(type == "set") {
		HostPortList hosts;
		for(QDomNode n = q.firstChild(); !n.isNull(); n = n.nextSibling()) {
			QDomElement i = n.toElement();
			if(i.isNull())
				continue;

			if(i.tagName() == "host" && hosts.count() < 3) {
				QString str = tagContent(i);
				int n = str.find(':');
				QString host;
				int port;
				if(n == -1) {
					host = str;
					port = 0;
				}
				else {
					host = str.mid(0, n);
					port = str.mid(n+1).toInt();
				}
				hosts += HostPort(host, port);
			}
		}
		QDomElement comment = findSubTag(q, "comment", &found);

		incoming(from, e.attribute("id"), key, hosts, comment);
	}
	else if(type == "error") {
		QString str = "";
		int code = 0;
		QDomElement err = findSubTag(e, "error", &found);
		if(found) {
			str = tagContent(err);
			code = err.attribute("code").toInt();
		}
		error(from, key, code, str);
	}

	return true;
}

void JT_PushDTCP::respondSuccess(const Jid &to, const QString &id, const QString &key, const HostPortList &hosts)
{
	QDomElement iq = createIQ(doc(), "result", to.full(), id);
	QDomElement query = doc()->createElement("query");
	query.setAttribute("xmlns", "http://jabber.org/protocol/dtcp");
	iq.appendChild(query);
	query.appendChild(textTag(doc(), "key", key));
	for(HostPortList::ConstIterator it = hosts.begin(); it != hosts.end(); ++it)
		query.appendChild( textTag(doc(), "host", (*it).host() + ':' + QString::number((*it).port())) );
	send(iq);
}

void JT_PushDTCP::respondError(const Jid &to, const QString &id, int code, const QString &str)
{
	QDomElement iq = createIQ(doc(), "error", to.full(), id);
	QDomElement err = textTag(doc(), "error", str);
	err.setAttribute("code", QString::number(code));
	iq.appendChild(err);
	send(iq);
}


//----------------------------------------------------------------------------
// HostPort
//----------------------------------------------------------------------------
HostPort::HostPort(const QString &host, int port)
{
	v_host = host;
	v_port = port;
}

const QString & HostPort::host() const
{
	return v_host;
}

int HostPort::port() const
{
	return v_port;
}

void HostPort::setHost(const QString &host)
{
	v_host = host;
}

void HostPort::setPort(int port)
{
	v_port = port;
}


//----------------------------------------------------------------------------
// ServSocket
//----------------------------------------------------------------------------
ServSock::ServSock(int port)
:QServerSocket(port, 16)
{
}

void ServSock::newConnection(int x)
{
	connectionReady(x);
}
