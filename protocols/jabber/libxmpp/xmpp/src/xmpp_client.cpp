/*
 * client.cpp - main library class
 * Copyright (C) 2001, 2002  Justin Karneges
 *                           Akito Nozaki
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

//! \class Client client.h
//! \brief Communicates with the Jabber network.  Start here.
//!
//!  Client controls an active Jabber connection.  It allows you to connect,
//!  authenticate, manipulate the roster, and send / receive messages and
//!  presence.  It is the centerpiece of this library, and all Tasks must pass
//!  through it.
//!
//!  For convenience, many Tasks are handled internally to Client (such as
//!  JT_Auth).  However, for accessing features beyond the basics provided by
//!  Client, you will need to manually invoke Tasks.  Fortunately, the
//!  process is very simple.
//!
//!  The entire Task system is heavily founded on Qt.  All Tasks have a parent,
//!  except for the root Task, and are considered QObjects.  By using Qt's RTTI
//!  facilities (QObject::sender(), QObject::isA(), etc), you can use a
//!  "fire and forget" approach with Tasks.
//!
//!  \code
//!  #include "client.h"
//!  using namespace Jabber;
//!
//!  ...
//!
//!  Client *client;
//!
//!  Session::Session()
//!  {
//!    client = new Client;
//!    connect(client, SIGNAL(handshaken()), SLOT(clientHandshaken()));
//!    connect(client, SIGNAL(authFinished(bool, int, const QString &)), SLOT(authFinished(bool, int, const QString &)));
//!    client->connectToHost("jabber.org");
//!  }
//!
//!  void Session::clientHandshaken()
//!  {
//!    client->authDigest("jabtest", "12345", "Psi");
//!  }
//!
//!  void Session::authFinished(bool success, int, const QString &err)
//!  {
//!    if(success)
//!      printf("Login success!");
//!    else
//!      printf("Login failed.  Here's why: %s\n", err.latin1());
//!  }
//!  \endcode

#include"xmpp_client.h"

#include<stdio.h>
#include<stdarg.h>
#include<qstring.h>
#include<qdom.h>
#include<qobjectlist.h>
#include<qtimer.h>
#include"xmpp_stream.h"
#include"xmpp_tasks.h"
#include"xmpp_xmlcommon.h"
#include"xmpp_dtcp.h"
#include"xmpp_ibb.h"
#include"xmpp_jidlink.h"

using namespace Jabber;

#ifdef Q_WS_WIN
#define vsnprintf _vsnprintf
#endif


//----------------------------------------------------------------------------
// Client
//----------------------------------------------------------------------------
class Client::GroupChat
{
public:
	enum { Connecting, Connected, Closing };
	GroupChat() {}

	Jid j;
	int status;
};

class Client::ClientPrivate
{
public:
	ClientPrivate() {}

	Stream *stream;
	QDomDocument doc;
	int id_seed;
	Task *root;
	QString host, user, pass, resource;
	QString osname, tzname, clientName, clientVersion;
	int tzoffset;
	bool authed;

	LiveRoster roster;
	ResourceList resourceList;
	DTCPManager *dtcpman;
	IBBManager *ibbman;
	JidLinkManager *jlman;
	QValueList<GroupChat> groupChatList;
};


Client::Client(QObject *par)
:QObject(par)
{
	d = new ClientPrivate;
	d->tzoffset = 0;
	d->authed = false;
	d->osname = "N/A";
	d->clientName = "N/A";
	d->clientVersion = "0.0";

	d->stream = new Stream;
	connect(d->stream, SIGNAL(connected()), SLOT(streamConnected()));
	connect(d->stream, SIGNAL(handshaken()), SLOT(streamHandshaken()));
	connect(d->stream, SIGNAL(error(const StreamError &)), SLOT(streamError(const StreamError &)));
	connect(d->stream, SIGNAL(sslCertificateReady(const QSSLCert &)), SLOT(streamSSLCertificateReady(const QSSLCert &)));
	connect(d->stream, SIGNAL(receivePacket(const QDomElement &)), SLOT(streamReceivePacket(const QDomElement &)));
	connect(d->stream, SIGNAL(closeFinished()), SLOT(streamCloseFinished()));

	d->id_seed = 0xaaaa;
	d->root = new Task(this, true);

	d->dtcpman = new DTCPManager(this);
	connect(d->dtcpman, SIGNAL(incomingReady()), SLOT(dtcp_incomingReady()));

	d->ibbman = new IBBManager(this);
	connect(d->ibbman, SIGNAL(incomingReady()), SLOT(ibb_incomingReady()));

	d->jlman = new JidLinkManager(this);
}

Client::~Client()
{
	close(true);

	delete d->jlman;
	delete d->ibbman;
	delete d->dtcpman;
	delete d->root;
	delete d->stream;
	delete d;
}

JidLinkManager *Client::jidLinkManager() const
{
	return d->jlman;
}

DTCPManager *Client::dtcpManager() const
{
	return d->dtcpman;
}

IBBManager *Client::ibbManager() const
{
	return d->ibbman;
}

bool Client::isActive() const
{
	return d->stream->isActive();
}

bool Client::isConnected() const
{
	return d->stream->isConnected();
}

bool Client::isHandshaken() const
{
	return d->stream->isHandshaken();
}

void Client::setNoopTime(int mills)
{
	d->stream->setNoopTime(mills);
}

bool Client::isSSLEnabled() const
{
	return d->stream->isSSLEnabled();
}

bool Client::setSSLEnabled(bool b)
{
	if(b && !Stream::isSSLSupported())
		return false;

	d->stream->setSSLEnabled(b);
	return true;
}

void Client::setSSLTrustedCertStoreDir(const QString &certStoreDir)
{
	d->stream->setSSLTrustedCertStoreDir(certStoreDir);
}

void Client::setProxy(const StreamProxy &p)
{
	d->stream->setProxy(p);
}

void Client::groupChatChangeNick(const QString &host, const QString &room, const QString &nick, const Status &_s)
{
	Jid jid(room + "@" + host + "/" + nick);
	for(QValueList<GroupChat>::ConstIterator it = d->groupChatList.begin(); it != d->groupChatList.end(); it++) {
		const GroupChat &i = *it;
		if(i.j.compare(jid, false)) {
			Status s = _s;
			s.setIsAvailable(true);

			JT_Presence *j = new JT_Presence(rootTask());
			j->pres(jid, s);
			j->go(true);
			
			break;
		}
	}
}

bool Client::groupChatJoin(const QString &host, const QString &room, const QString &nick)
{
	Jid jid(room + "@" + host + "/" + nick);
	for(QValueList<GroupChat>::Iterator it = d->groupChatList.begin(); it != d->groupChatList.end();) {
		GroupChat &i = *it;
		if(i.j.compare(jid, false)) {
			// if this room is shutting down, then free it up
			if(i.status == GroupChat::Closing)
				it = d->groupChatList.remove(it);
			else
				return false;
		}
		else
			++it;
	}

	debug(QString("Client: Joined: [%1]\n").arg(jid.full()));
	GroupChat i;
	i.j = jid;
	i.status = GroupChat::Connecting;
	d->groupChatList += i;

	JT_Presence *j = new JT_Presence(rootTask());
	j->pres(jid, Status());
	j->go(true);

	return true;
}

void Client::groupChatSetStatus(const QString &host, const QString &room, const Status &_s)
{
	Jid jid(room + "@" + host);
	bool found = false;
	for(QValueList<GroupChat>::ConstIterator it = d->groupChatList.begin(); it != d->groupChatList.end(); it++) {
		const GroupChat &i = *it;
		if(i.j.compare(jid, false)) {
			found = true;
			jid = i.j;
			break;
		}
	}
	if(!found)
		return;

	Status s = _s;
	s.setIsAvailable(true);

	JT_Presence *j = new JT_Presence(rootTask());
	j->pres(jid, s);
	j->go(true);
}

void Client::groupChatLeave(const QString &host, const QString &room)
{
	Jid jid(room + "@" + host);
	for(QValueList<GroupChat>::Iterator it = d->groupChatList.begin(); it != d->groupChatList.end(); it++) {
		GroupChat &i = *it;

		if(!i.j.compare(jid, false))
			continue;

		i.status = GroupChat::Closing;
		debug(QString("Client: Leaving: [%1]\n").arg(i.j.full()));

		JT_Presence *j = new JT_Presence(rootTask());
		Status s;
		s.setIsAvailable(false);
		j->pres(i.j, s);
		j->go(true);
	}
}

void Client::connectToHost(const QString &host, int port, const QString &virtualHost)
{
	if(virtualHost.isEmpty())
		d->host = host;
	else
		d->host = virtualHost;

	d->stream->connectToHost(host, port, virtualHost);
}

// TODO: fast close
void Client::close(bool)
{
	for(QValueList<GroupChat>::Iterator it = d->groupChatList.begin(); it != d->groupChatList.end(); it++) {
		GroupChat &i = *it;
		i.status = GroupChat::Closing;

		JT_Presence *j = new JT_Presence(rootTask());
		Status s;
		s.setIsAvailable(false);
		j->pres(i.j, s);
		j->go(true);
	}
	d->stream->close();
	disconnected();
	cleanup();
}

void Client::cleanup()
{
	d->authed = false;
	d->groupChatList.clear();
}

void Client::continueAfterCert()
{
	d->stream->continueAfterCert();
}

void Client::streamConnected()
{
	connected();
}

void Client::streamHandshaken()
{
	handshaken();
}

void Client::streamError(const StreamError &err)
{
	StreamError e = err;
	error(e);

	if(!e.isWarning()) {
		disconnected();
		cleanup();
	}
}

void Client::streamSSLCertificateReady(const QSSLCert &cert)
{
	sslCertReady(cert);
}

void Client::streamCloseFinished()
{
	closeFinished();
}

void Client::streamReceivePacket(const QDomElement &x)
{
	QString out;
	QTextStream ts(&out, IO_WriteOnly);
	x.save(ts, 0);

	debug(QString("Client: incoming: [\n%1]\n").arg(out));

	xmlIncoming(out);
	distribute(x);
}

void Client::debug(const QString &str)
{
	debugText(str);
}

QString Client::genUniqueId()
{
	QString s;
	s.sprintf("a%x", d->id_seed);
	d->id_seed += 0x10;
	return s;
}

Task *Client::rootTask()
{
	return d->root;
}

QDomDocument *Client::doc() const
{
	return &d->doc;
}

void Client::distribute(const QDomElement &x)
{
	if(x.hasAttribute("from")) {
		Jid j(x.attribute("from"));
		if(!j.isValid()) {
			debug("Client: bad 'from' JID\n");
			return;
		}
	}

	if(!rootTask()->take(x)) {
		debug("Client: packet was ignored.\n");
	}
}

void Client::send(const QDomElement &x)
{
	QString out;
	QTextStream ts(&out, IO_WriteOnly);
	x.save(ts, 0);

	debug(QString("Client: outgoing: [\n%1]\n").arg(out));
	xmlOutgoing(out);
	d->stream->sendPacket(x);
}

void Client::send(const QString &str)
{
	debug(QString("Client: outgoing: [\n%1]\n").arg(str));
	QCString cs = str.utf8();
	xmlOutgoing(str);
	d->stream->sendString(cs);
}

Stream & Client::stream()
{
	return *d->stream;
}

const LiveRoster & Client::roster() const
{
	return d->roster;
}

const ResourceList & Client::resourceList() const
{
	return d->resourceList;
}

QString Client::host() const
{
	return d->host;
}

QString Client::user() const
{
	return d->user;
}

QString Client::pass() const
{
	return d->pass;
}

QString Client::resource() const
{
	return d->resource;
}

Jid Client::jid() const
{
	QString s;
	if(!d->user.isEmpty())
		s += d->user + '@';
	s += d->host;
	if(!d->resource.isEmpty()) {
		s += '/';
		s += d->resource;
	}

	return Jid(s);
}

bool Client::isAuthenticated() const
{
	return d->authed;
}

void Client::authPlain(const QString &user, const QString &pass, const QString &resource)
{
	d->user = user;
	d->pass = pass;
	d->resource = resource;

	JT_Auth *a = new JT_Auth(rootTask());
	connect(a, SIGNAL(finished()), SLOT(slotAuthFinished()));
	a->plain(user, pass, resource);
	a->go(true);
}

void Client::authDigest(const QString &user, const QString &pass, const QString &resource)
{
	d->user = user;
	d->pass = pass;
	d->resource = resource;

	JT_Auth *a = new JT_Auth(rootTask());
	connect(a, SIGNAL(finished()), SLOT(slotAuthFinished()));
	a->digest(user, pass, resource);
	a->go(true);
}

void Client::slotAuthFinished()
{
	JT_Auth *a = (JT_Auth *)sender();

	// on success, load up some daemons
	if(a->success()) {
		d->authed = true;

		Status s;
		s.setIsAvailable(false);
		d->resourceList += Resource(resource(), s);

		JT_PushPresence *pp = new JT_PushPresence(rootTask());
		connect(pp, SIGNAL(subscription(const Jid &, const QString &)), SLOT(ppSubscription(const Jid &, const QString &)));
		connect(pp, SIGNAL(presence(const Jid &, const Status &)), SLOT(ppPresence(const Jid &, const Status &)));

		JT_PushMessage *pm = new JT_PushMessage(rootTask());
		connect(pm, SIGNAL(message(const Message &)), SLOT(pmMessage(const Message &)));

		JT_PushRoster *pr = new JT_PushRoster(rootTask());
		connect(pr, SIGNAL(roster(const Roster &)), SLOT(prRoster(const Roster &)));

		new JT_ServInfo(rootTask());
	}
	else {
		// don't report a disconnect.  Client::error() will do that.
		if(a->statusCode() == Task::ErrDisc)
			return;
	}

	// report success / fail
	authFinished(a->success(), a->statusCode(), a->statusString());
}

void Client::ppSubscription(const Jid &j, const QString &s)
{
	subscription(j, s);
}

void Client::ppPresence(const Jid &j, const Status &s)
{
	if(s.isAvailable())
		debug(QString("Client: %1 is available.\n").arg(j.full()));
	else
		debug(QString("Client: %1 is unavailable.\n").arg(j.full()));

	for(QValueList<GroupChat>::Iterator it = d->groupChatList.begin(); it != d->groupChatList.end(); it++) {
		GroupChat &i = *it;

		if(i.j.compare(j, false)) {
			bool us = (i.j.resource() == j.resource() || j.resource().isEmpty()) ? true: false;

			debug(QString("for groupchat i=[%1] pres=[%2], [us=%3].\n").arg(i.j.full()).arg(j.full()).arg(us));
			switch(i.status) {
				case GroupChat::Connecting:
					if(us && s.hasError()) {
						Jid j = i.j;
						d->groupChatList.remove(it);
						groupChatError(j, s.errorCode(), s.errorString());
					}
					else {
						// don't signal success unless it is a non-error presence
						if(!s.hasError()) {
							i.status = GroupChat::Connected;
							groupChatJoined(i.j);
						}
						groupChatPresence(j, s);
					}
					break;
				case GroupChat::Connected:
					groupChatPresence(j, s);
					break;
				case GroupChat::Closing:
					if(us && !s.isAvailable()) {
						Jid j = i.j;
						d->groupChatList.remove(it);
						groupChatLeft(j);
					}
					break;
				default:
					break;
			}

			return;
		}
	}

	if(s.hasError()) {
		presenceError(j, s.errorCode(), s.errorString());
		return;
	}

	// is it me?
	if(j.compare(jid(), false)) {
		updateSelfPresence(j, s);
	}
	else {
		// update all relavent roster entries
		for(LiveRoster::Iterator it = d->roster.begin(); it != d->roster.end(); ++it) {
			LiveRosterItem &i = *it;

			if(!i.jid().compare(j, false))
				continue;

			// roster item has its own resource?
			if(!i.jid().resource().isEmpty()) {
				if(i.jid().resource() != j.resource())
					continue;
			}

			updatePresence(&i, j, s);
		}
	}
}

void Client::updateSelfPresence(const Jid &j, const Status &s)
{
	ResourceList::Iterator rit = d->resourceList.find(j.resource());
	bool found = (rit == d->resourceList.end()) ? false: true;

	// unavailable?  remove the resource
	if(!s.isAvailable()) {
		if(found) {
			debug(QString("Client: Removing self resource: name=[%1]\n").arg(j.resource()));
			(*rit).setStatus(s);
			resourceUnavailable(j, *rit);
			d->resourceList.remove(rit);
		}
	}
	// available?  add/update the resource
	else {
		Resource r;
		if(!found) {
			r = Resource(j.resource(), s);
			d->resourceList += r;
			debug(QString("Client: Adding self resource: name=[%1]\n").arg(j.resource()));
		}
		else {
			(*rit).setStatus(s);
			r = *rit;
			debug(QString("Client: Updating self resource: name=[%1]\n").arg(j.resource()));
		}

		resourceAvailable(j, r);
	}
}

void Client::updatePresence(LiveRosterItem *i, const Jid &j, const Status &s)
{
	ResourceList::Iterator rit = i->resourceList().find(j.resource());
	bool found = (rit == i->resourceList().end()) ? false: true;

	// unavailable?  remove the resource
	if(!s.isAvailable()) {
		if(found) {
			(*rit).setStatus(s);
			debug(QString("Client: Removing resource from [%1]: name=[%2]\n").arg(i->jid().full()).arg(j.resource()));
			resourceUnavailable(j, *rit);
			i->resourceList().remove(rit);
			i->setLastUnavailableStatus(s);
		}
	}
	// available?  add/update the resource
	else {
		Resource r;
		if(!found) {
			r = Resource(j.resource(), s);
			i->resourceList() += r;
			debug(QString("Client: Adding resource to [%1]: name=[%2]\n").arg(i->jid().full()).arg(j.resource()));
		}
		else {
			(*rit).setStatus(s);
			r = *rit;
			debug(QString("Client: Updating resource to [%1]: name=[%2]\n").arg(i->jid().full()).arg(j.resource()));
		}

		resourceAvailable(j, r);
	}
}

void Client::pmMessage(const Message &m)
{
	debug(QString("Client: Message from %1\n").arg(m.from().full()));

	if(m.type() == "groupchat") {
		for(QValueList<GroupChat>::Iterator it = d->groupChatList.begin(); it != d->groupChatList.end(); it++) {
			const GroupChat &i = *it;

			if(!i.j.compare(m.from(), false))
				continue;

			if(i.status == GroupChat::Connected)
				messageReceived(m);
		}
	}
	else
		messageReceived(m);
}

void Client::prRoster(const Roster &r)
{
	importRoster(r);
}

void Client::rosterRequest()
{
	if(!isAuthenticated())
		return;

	JT_Roster *r = new JT_Roster(rootTask());
	connect(r, SIGNAL(finished()), SLOT(slotRosterRequestFinished()));
	r->get();
	d->roster.flagAllForDelete(); // mod_groups patch
	r->go(true);
}

void Client::slotRosterRequestFinished()
{
	JT_Roster *r = (JT_Roster *)sender();
	// on success, let's take it
	if(r->success()) {
		//d->roster.flagAllForDelete(); // mod_groups patch

		importRoster(r->roster());

		for(LiveRoster::Iterator it = d->roster.begin(); it != d->roster.end();) {
			LiveRosterItem &i = *it;
			if(i.flagForDelete()) {
				rosterItemRemoved(i);
				it = d->roster.remove(it);
			}
			else
				++it;
		}
	}
	else {
		// don't report a disconnect.  Client::error() will do that.
		if(r->statusCode() == Task::ErrDisc)
			return;
	}

	// report success / fail
	rosterRequestFinished(r->success(), r->statusCode(), r->statusString());
}

void Client::importRoster(const Roster &r)
{
	for(Roster::ConstIterator it = r.begin(); it != r.end(); ++it) {
		importRosterItem(*it);
	}
}

void Client::importRosterItem(const RosterItem &item)
{
	QString substr;
	switch(item.subscription().type()) {
		case Subscription::Both:
			substr = "<-->";  break;
		case Subscription::From:
			substr = "  ->";  break;
		case Subscription::To:
			substr = "<-  ";  break;
		case Subscription::Remove:
			substr = "xxxx";  break;
		case Subscription::None:
		default:
			substr = "----";  break;
	}

	QString dstr, str;
	str.sprintf("  %s %-32s", substr.latin1(), item.jid().full().latin1());
	if(!item.name().isEmpty())
		str += QString(" [") + item.name() + "]";
	str += '\n';

	// Remove
	if(item.subscription().type() == Subscription::Remove) {
		LiveRoster::Iterator it = d->roster.find(item.jid());
		if(it != d->roster.end()) {
			rosterItemRemoved(*it);
			d->roster.remove(it);
		}
		dstr = "Client: (Removed) ";
	}
	// Add/Update
	else {
		LiveRoster::Iterator it = d->roster.find(item.jid());
		if(it != d->roster.end()) {
			LiveRosterItem &i = *it;
			i.setFlagForDelete(false);
			i.setRosterItem(item);
			rosterItemUpdated(i);
			dstr = "Client: (Updated) ";
                }
		else {
			LiveRosterItem i(item);
			d->roster += i;

			// signal it
			rosterItemAdded(i);
			dstr = "Client: (Added)   ";
		}
	}

	debug(dstr + str);
}

void Client::sendMessage(const Message &m)
{
	JT_Message *j = new JT_Message(rootTask(), m);
	j->go(true);
}

void Client::sendSubscription(const Jid &jid, const QString &type)
{
	JT_Presence *j = new JT_Presence(rootTask());
	j->sub(jid, type);
	j->go(true);
}

void Client::setPresence(const Status &s)
{
	JT_Presence *j = new JT_Presence(rootTask());
	j->pres(s);
	j->go(true);

	// update our resourceList
	ppPresence(jid(), s);
	//ResourceList::Iterator rit = d->resourceList.find(resource());
	//Resource &r = *rit;
	//r.setStatus(s);
}

QString Client::OSName() const
{
	return d->osname;
}

QString Client::timeZone() const
{
	return d->tzname;
}

int Client::timeZoneOffset() const
{
	return d->tzoffset;
}

QString Client::clientName() const
{
	return d->clientName;
}

QString Client::clientVersion() const
{
	return d->clientVersion;
}

void Client::setOSName(const QString &name)
{
	d->osname = name;
}

void Client::setTimeZone(const QString &name, int offset)
{
	d->tzname = name;
	d->tzoffset = offset;
}

void Client::setClientName(const QString &s)
{
	d->clientName = s;
}

void Client::setClientVersion(const QString &s)
{
	d->clientVersion = s;
}

void Client::dtcp_incomingReady()
{
	DTCPConnection *c = d->dtcpman->takeIncoming();
	if(!c)
		return;
	d->jlman->insertStream(c);
	incomingJidLink();
}

void Client::ibb_incomingReady()
{
	IBBConnection *c = d->ibbman->takeIncoming();
	if(!c)
		return;
	d->jlman->insertStream(c);
	incomingJidLink();
}


//----------------------------------------------------------------------------
// Task
//----------------------------------------------------------------------------
class Task::TaskPrivate
{
public:
	TaskPrivate() {}

	QString id;
	bool success;
	int statusCode;
	QString statusString;
	Client *client;
	bool insig, deleteme, autoDelete;
	bool done;
};

Task::Task(Task *parent)
:QObject(parent)
{
	init();

	d->client = parent->client();
	d->id = client()->genUniqueId();
	connect(d->client, SIGNAL(disconnected()), SLOT(clientDisconnected()));
}

Task::Task(Client *parent, bool)
:QObject(0)
{
	init();

	d->client = parent;
	connect(d->client, SIGNAL(disconnected()), SLOT(clientDisconnected()));
}

Task::~Task()
{
	delete d;
}

void Task::init()
{
	d = new TaskPrivate;
	d->success = false;
	d->insig = false;
	d->deleteme = false;
	d->autoDelete = false;
	d->done = false;
}

Task *Task::parent() const
{
	return (Task *)QObject::parent();
}

Client *Task::client() const
{
	return d->client;
}

QDomDocument *Task::doc() const
{
	return client()->doc();
}

QString Task::id() const
{
	return d->id;
}

bool Task::success() const
{
	return d->success;
}

int Task::statusCode() const
{
	return d->statusCode;
}

const QString & Task::statusString() const
{
	return d->statusString;
}

void Task::go(bool autoDelete)
{
	d->autoDelete = autoDelete;

	onGo();
}

bool Task::take(const QDomElement &x)
{
	const QObjectList *p = children();
	if(!p)
		return false;

	// pass along the xml
	QObjectListIt it(*p);
	Task *t;
	for(; it.current(); ++it) {
		QObject *obj = it.current();
		if(!obj->inherits("Jabber::Task"))
			continue;

		t = static_cast<Task*>(obj);
		if(t->take(x))
			return true;
	}

	return false;
}

void Task::safeDelete()
{
	if(d->deleteme)
		return;

	d->deleteme = true;
	if(!d->insig)
		deleteLater();
}

void Task::onGo()
{
}

void Task::onDisconnect()
{
	if(!d->done) {
		d->success = false;
		d->statusCode = ErrDisc;
		d->statusString = tr("Disconnected");

		// delay this so that tasks that react don't block the shutdown
		QTimer::singleShot(0, this, SLOT(done()));
	}
}

void Task::send(const QDomElement &x)
{
	client()->send(x);
}

void Task::setSuccess(int code, const QString &str)
{
	if(!d->done) {
		d->success = true;
		d->statusCode = code;
		d->statusString = str;
		done();
	}
}

void Task::setError(const QDomElement &e)
{
	if(!d->done) {
		d->success = false;
		getErrorFromElement(e, &d->statusCode, &d->statusString);
		done();
	}
}

void Task::setError(int code, const QString &str)
{
	if(!d->done) {
		d->success = false;
		d->statusCode = code;
		d->statusString = str;
		done();
	}
}

void Task::done()
{
	if(d->done || d->insig)
		return;
	d->done = true;

	if(d->deleteme || d->autoDelete)
		d->deleteme = true;

	d->insig = true;
	finished();
	d->insig = false;

	if(d->deleteme)
		deleteLater();
}

void Task::clientDisconnected()
{
	onDisconnect();
}

void Task::debug(const char *fmt, ...)
{
	char *buf;
	QString str;
	int size = 1024;
	int r;

	do {
		buf = new char[size];
		va_list ap;
		va_start(ap, fmt);
		r = vsnprintf(buf, size, fmt, ap);
		va_end(ap);

		if(r != -1)
			str = QString(buf);

		delete [] buf;

		size *= 2;
	} while(r == -1);

	debug(str);
}

void Task::debug(const QString &str)
{
	client()->debug(QString("%1: ").arg(className()) + str);
}

bool Task::iqVerify(const QDomElement &x, const Jid &to, const QString &id, const QString &xmlns)
{
	Jid from(x.attribute("from"));
	Jid local = client()->jid();
	Jid server = client()->host();

	// empty 'from' ?
	if(from.isEmpty()) {
		// allowed if we are querying the server
		if(!to.isEmpty() && !to.compare(server))
			return false;
	}
	// from ourself?
	else if(from.compare(local)) {
		// allowed if we are querying ourself or the server
		if(!to.isEmpty() && !to.compare(local, false) && !to.compare(server))
			return false;
	}
	// from anywhere else?
	else {
		if(!from.compare(to))
			return false;
	}

	if(!id.isEmpty()) {
		if(x.attribute("id") != id)
			return false;
	}

	if(!xmlns.isEmpty()) {
		if(queryNS(x) != xmlns)
			return false;
	}

	return true;
}


//---------------------------------------------------------------------------
// LiveRosterItem
//---------------------------------------------------------------------------
LiveRosterItem::LiveRosterItem(const Jid &jid)
:RosterItem(jid)
{
	setFlagForDelete(false);
}

LiveRosterItem::LiveRosterItem(const RosterItem &i)
{
	setRosterItem(i);
	setFlagForDelete(false);
}

LiveRosterItem::~LiveRosterItem()
{
}

void LiveRosterItem::setRosterItem(const RosterItem &i)
{
	setJid(i.jid());
	setName(i.name());
	setGroups(i.groups());
	setSubscription(i.subscription());
	setAsk(i.ask());
	setIsPush(i.isPush());
}

ResourceList & LiveRosterItem::resourceList()
{
	return v_resourceList;
}

ResourceList::Iterator LiveRosterItem::priority()
{
	return v_resourceList.priority();
}

const ResourceList & LiveRosterItem::resourceList() const
{
	return v_resourceList;
}

ResourceList::ConstIterator LiveRosterItem::priority() const
{
	return v_resourceList.priority();
}

bool LiveRosterItem::isAvailable() const
{
	if(v_resourceList.count() > 0)
		return true;
	return false;
}

const Status & LiveRosterItem::lastUnavailableStatus() const
{
	return v_lastUnavailableStatus;
}

bool LiveRosterItem::flagForDelete() const
{
	return v_flagForDelete;
}

void LiveRosterItem::setLastUnavailableStatus(const Status &s)
{
	v_lastUnavailableStatus = s;
}

void LiveRosterItem::setFlagForDelete(bool b)
{
	v_flagForDelete = b;
}


//---------------------------------------------------------------------------
// LiveRoster
//---------------------------------------------------------------------------
LiveRoster::LiveRoster()
:QValueList<LiveRosterItem>()
{
}

LiveRoster::~LiveRoster()
{
}

void LiveRoster::flagAllForDelete()
{
	for(Iterator it = begin(); it != end(); ++it)
		(*it).setFlagForDelete(true);
}

LiveRoster::Iterator LiveRoster::find(const Jid &j, bool compareRes)
{
	Iterator it;
	for(it = begin(); it != end(); ++it) {
		if((*it).jid().compare(j, compareRes))
			break;
	}
	return it;
}

LiveRoster::ConstIterator LiveRoster::find(const Jid &j, bool compareRes) const
{
	ConstIterator it;
	for(it = begin(); it != end(); ++it) {
		if((*it).jid().compare(j, compareRes))
			break;
	}
	return it;
}
