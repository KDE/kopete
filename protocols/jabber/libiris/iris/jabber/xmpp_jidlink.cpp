/*
 * jidlink.cpp - establish a link between Jabber IDs
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

#include"xmpp_jidlink.h"

#include<qdom.h>
#include<qtimer.h>
#include"im.h"
#include"s5b.h"
#include"xmpp_ibb.h"

using namespace XMPP;

//----------------------------------------------------------------------------
// JidLink
//----------------------------------------------------------------------------
class JidLink::Private
{
public:
	Client *client;
	ByteStream *bs;
	int type;
	int state;
	Jid peer;
};

JidLink::JidLink(Client *client)
:QObject(client->jidLinkManager())
{
	d = new Private;
	d->client = client;
	d->bs = 0;

	reset();
}

JidLink::~JidLink()
{
	reset(true);

	delete d;
}

void JidLink::reset(bool clear)
{
	d->type = None;
	d->state = Idle;

	if(d->bs) {
		unlink();
		d->bs->close();
		if(clear) {
			delete d->bs;
			d->bs = 0;
		}
	}
}

void JidLink::connectToJid(const Jid &jid, int type, const QDomElement &comment)
{
	reset(true);
	if(type == DTCP)
		d->bs = d->client->s5bManager()->createConnection();
	else if(type == IBB)
		d->bs = new IBBConnection(d->client->ibbManager());
	else
		return;

	d->type = type;
	d->peer = jid;
	d->state = Connecting;

	link();

	if(type == DTCP) {
		S5BConnection *c = (S5BConnection *)d->bs;
		status(StatDTCPRequesting);
		c->connectToJid(jid, d->client->s5bManager()->genUniqueSID(jid));
	}
	else {
		IBBConnection *c = (IBBConnection *)d->bs;
		status(StatIBBRequesting);
		c->connectToJid(jid, comment);
	}
}

void JidLink::link()
{
	if(d->type == DTCP) {
		S5BConnection *c = (S5BConnection *)d->bs;
		connect(c, SIGNAL(connected()), SLOT(dtcp_connected()));
		connect(c, SIGNAL(accepted()), SLOT(dtcp_accepted()));
	}
	else {
		IBBConnection *c = (IBBConnection *)d->bs;
		connect(c, SIGNAL(connected()), SLOT(ibb_connected()));
	}

	connect(d->bs, SIGNAL(connectionClosed()), SLOT(bs_connectionClosed()));
	connect(d->bs, SIGNAL(error(int)), SLOT(bs_error(int)));
	connect(d->bs, SIGNAL(bytesWritten(int)), SLOT(bs_bytesWritten(int)));
	connect(d->bs, SIGNAL(readyRead()), SLOT(bs_readyRead()));
}

void JidLink::unlink()
{
	d->bs->disconnect(this);
}

void JidLink::accept()
{
	if(d->state != WaitingForAccept)
		return;

	QTimer::singleShot(0, this, SLOT(doRealAccept()));
}

void JidLink::doRealAccept()
{
	if(d->type == DTCP) {
		((S5BConnection *)d->bs)->accept();
		d->state = Connecting;
		dtcp_accepted();
	}
	else {
		((IBBConnection *)d->bs)->accept();
		d->state = Active;
		connected();
	}
}

bool JidLink::setStream(ByteStream *bs)
{
	reset(true);
	int type = None;
	if(bs->inherits("XMPP::S5BConnection"))
		type = DTCP;
	else if(bs->inherits("XMPP::IBBConnection"))
		type = IBB;

	if(type == None)
		return false;

	d->type = type;
	d->bs = bs;
	d->state = WaitingForAccept;

	link();

	if(d->type == DTCP)
		d->peer = ((S5BConnection *)d->bs)->peer();
	else
		d->peer = ((IBBConnection *)d->bs)->peer();

	return true;
}

int JidLink::type() const
{
	return d->type;
}

Jid JidLink::peer() const
{
	return d->peer;
}

int JidLink::state() const
{
	return d->state;
}

bool JidLink::isOpen() const
{
	if(d->state == Active)
		return true;
	else
		return false;
}

void JidLink::close()
{
	if(d->state == Idle)
		return;
	reset();
}

void JidLink::write(const QByteArray &a)
{
	if(d->state == Active)
		d->bs->write(a);
}

QByteArray JidLink::read(int bytes)
{
	if(d->bs)
		return d->bs->read(bytes);
	else
		return QByteArray();
}

int JidLink::bytesAvailable() const
{
	if(d->bs)
		return d->bs->bytesAvailable();
	else
		return 0;
}

int JidLink::bytesToWrite() const
{
	if(d->state == Active)
		return d->bs->bytesToWrite();
	else
		return 0;
}

void JidLink::dtcp_accepted()
{
	status(StatDTCPAccepted);
}

void JidLink::dtcp_connected()
{
	d->state = Active;
	status(StatDTCPConnected);
	connected();
}

void JidLink::ibb_connected()
{
	d->state = Active;
	status(StatIBBConnected);
	connected();
}

void JidLink::bs_connectionClosed()
{
	reset();
	connectionClosed();
}

void JidLink::bs_error(int)
{
	reset();
	error(ErrConnect);
}

void JidLink::bs_readyRead()
{
	readyRead();
}

void JidLink::bs_bytesWritten(int x)
{
	bytesWritten(x);
}


//----------------------------------------------------------------------------
// JidLinkManager
//----------------------------------------------------------------------------
class JidLinkManager::Private
{
public:
	Private() {}

	Client *client;
	QPtrList<JidLink> incomingList;
};

JidLinkManager::JidLinkManager(Client *par)
:QObject(par)
{
	d = new Private;
	d->client = par;
}

JidLinkManager::~JidLinkManager()
{
	d->incomingList.setAutoDelete(true);
	d->incomingList.clear();
	delete d;
}

JidLink *JidLinkManager::takeIncoming()
{
	if(d->incomingList.isEmpty())
		return 0;

	JidLink *j = d->incomingList.getFirst();
	d->incomingList.removeRef(j);
	return j;
}

void JidLinkManager::insertStream(ByteStream *bs)
{
	JidLink *j = new JidLink(d->client);
	if(j->setStream(bs))
		d->incomingList.append(j);
}
