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
#include"xmpp_client.h"
#include"xmpp_dtcp.h"
#include"xmpp_ibb.h"

using namespace Jabber;

//----------------------------------------------------------------------------
// JidLink
//----------------------------------------------------------------------------
class JidLink::Private
{
public:
	Private() {}

	Client *client;
	ByteStream *bs;
	int type;
	int state;
	Jid peer;
	QByteArray rest;
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
	reset();

	delete d;
}

void JidLink::reset()
{
	d->type = None;
	d->state = Idle;

	if(d->bs) {
		unlink();
		d->bs->close();
		if(d->bs->canRead())
			d->rest = d->bs->read();
		d->client->jidLinkManager()->takeOver(d->bs);
		d->bs = 0;
	}
}

void JidLink::connectToJid(const Jid &jid, int type, const QDomElement &comment)
{
	d->rest.resize(0);

	if(type == DTCP)
		d->bs = new DTCPConnection(d->client->dtcpManager());
	else if(type == IBB)
		d->bs = new IBBConnection(d->client->ibbManager());
	else
		return;

	d->type = type;
	d->peer = jid;
	d->state = Connecting;

	link();

	if(type == DTCP) {
		DTCPConnection *c = (DTCPConnection *)d->bs;
		status(StatDTCPRequesting);
		c->connectToJid(jid, comment);
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
		DTCPConnection *c = (DTCPConnection *)d->bs;
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
	if(d->type == DTCP) {
		DTCPConnection *c = (DTCPConnection *)d->bs;
		disconnect(c, SIGNAL(connected()), this, SLOT(dtcp_connected()));
		disconnect(c, SIGNAL(accepted()), this, SLOT(dtcp_accepted()));
	}
	else {
		IBBConnection *c = (IBBConnection *)d->bs;
		disconnect(c, SIGNAL(connected()), this, SLOT(ibb_connected()));
	}

	disconnect(d->bs, SIGNAL(connectionClosed()), this, SLOT(bs_connectionClosed()));
	disconnect(d->bs, SIGNAL(error(int)), this, SLOT(bs_error(int)));
	disconnect(d->bs, SIGNAL(bytesWritten(int)), this, SLOT(bs_bytesWritten(int)));
	disconnect(d->bs, SIGNAL(readyRead()), this, SLOT(bs_readyRead()));
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
		((DTCPConnection *)d->bs)->accept();
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
	int type = None;
	if(bs->inherits("Jabber::DTCPConnection"))
		type = DTCP;
	else if(bs->inherits("Jabber::IBBConnection"))
		type = IBB;

	if(type == None)
		return false;

	d->type = type;
	d->bs = bs;
	d->state = WaitingForAccept;

	link();

	if(d->type == DTCP)
		d->peer = ((DTCPConnection *)d->bs)->peer();
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

bool JidLink::isConnected() const
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

QByteArray JidLink::read()
{
	if(d->bs)
		return d->bs->read();
	else {
		QByteArray a = d->rest.copy();
		d->rest.resize(0);
		return a;
	}
}

bool JidLink::canRead() const
{
	if(d->bs)
		return d->bs->canRead();
	else
		return (!d->rest.isEmpty());
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
// JidLink
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

void JidLinkManager::takeOver(ByteStream *bs)
{
	// TODO handle shutdown of this object
	bs->deleteLater();
}

#include "xmpp_jidlink.moc"
