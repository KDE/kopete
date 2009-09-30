/*
 * jingleicetransport.cpp - Jingle Ice Transport
 * Copyright (C) 2009 - Detlev Casanova <detlev.casanova@gmail.com>
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

#include "jinglesession.h"
#include "jinglesessionmanager.h"
#include "jingleicetransport.h"

#include <QDomElement>

//----------------------
// JingleIceTransport
//----------------------

using namespace XMPP;

class JingleIceTransport::Private
{
public:
	Private() :
		ice176(0),
		iceStarted(false),
		iceStarting(false),
		manager(0),
		componentCount(0),
		sentCandidates(0)
		{}

	Ice176 *ice176;
	
	QString remoteUfrag;
	QString remotePassword;

	bool iceStarted;
	bool iceStarting;
	
	QList<Ice176::Candidate> pendingLocalCandidates;
	QList<Ice176::Candidate> pendingRemoteCandidates;

	QList<bool> channels;

	JingleSessionManager *manager;
	
	int componentCount;
	int sentCandidates;

	QDomElement elem;
};

JingleIceTransport::JingleIceTransport(Mode mode, JingleContent *parent, const QDomElement& elem)
: JingleTransport(mode, parent), d(new Private())
{
	d->elem = elem;
	
	if (parent != 0)
		init();
}

void JingleIceTransport::init()
{
	if (d->ice176 != 0)
		return;

	qDebug() << "JingleTransport::init() called";
	d->ice176 = new Ice176(this);

	if (!d->elem.isNull())
	{
		// elem is a transport element.
		d->remoteUfrag = d->elem.attribute("ufrag");
		d->remotePassword = d->elem.attribute("pwd");
		d->ice176->setPeerUfrag(d->remoteUfrag);
		d->ice176->setPeerPassword(d->remotePassword);
		QDomElement cand = d->elem.firstChildElement();
		while (!cand.isNull())
		{
			if (cand.tagName() == "candidate")
			{
				Ice176::Candidate iceCand = xmlToCandidate(cand);
				d->pendingRemoteCandidates << iceCand;
			}

			cand = cand.nextSiblingElement();
		}
	}

	connect(d->ice176, SIGNAL(started()), SLOT(slotIceStarted()));
	connect(d->ice176, SIGNAL(localCandidatesReady(const QList<XMPP::Ice176::Candidate>&)), SLOT(slotIceLocalCandidatesReady(const QList<XMPP::Ice176::Candidate>&)));
	connect(d->ice176, SIGNAL(componentReady(int)), SLOT(slotIceComponentReady(int)));
	connect(d->ice176, SIGNAL(readyRead(int)), SIGNAL(readyRead(int)));
	//connect(d->ice176, SIGNAL(datagramsWritten(int, int)), SLOT(slotIceDatagramsWritten(int, int)));

	d->manager = JingleSessionManager::manager();

	if (d->manager && (d->manager->stunPort() != -1) && !d->manager->stunAddress().isNull())
		d->ice176->setStunService(Ice176::Basic, d->manager->stunAddress(), d->manager->stunPort());

	QList<Ice176::LocalAddress> addr = getAddresses();
	if (addr.count() == 0)
		return;
	
	d->ice176->setLocalAddresses(addr);
}

JingleIceTransport::~JingleIceTransport()
{
	delete d->ice176;
	delete d;
}

void JingleIceTransport::start()
{
	if (d->iceStarting)
		return;

	if (d->componentCount <= 0)
		return;

	switch (mode())
	{
	case Initiator:
	{
		d->ice176->start(Ice176::Initiator);
		break;
	}
	case Responder:
	{
		d->ice176->start(Ice176::Responder);
		break;
	}
	default :
		qDebug() << "JingleIceTransport : unable to start Ice176.";
		return;
	}

	QDomDocument doc("");

	QDomElement transport = doc.createElement("transport");

	transport.setAttribute("xmlns", transportNS());
	transport.setAttribute("pwd", d->ice176->localPassword());
	transport.setAttribute("ufrag", d->ice176->localUfrag());

	setTransport(transport);

	if (parentSession())
		sendLocalCandidates(d->pendingLocalCandidates);

	d->iceStarting = true;
}

void JingleIceTransport::setComponentCount(int c)
{
	d->channels.clear();
	for (int i = 0; i < c; ++i)
		d->channels << false;
	
	d->componentCount = c;
	
	if (d->manager)
		d->ice176->setBasePort(d->manager->nextUdpPort(c)); //FIXME:How does the next session to be started know which base port to set ?

	d->ice176->setComponentCount(c);
}

void JingleIceTransport::slotIceComponentReady(int channel)
{
	d->channels[channel] = true;

	// If not all channels are ready, we do nothing.
	for (int i = 0; i < d->channels.count(); i++)
	{
		if (d->channels[i] == false)
			return;
	}

	// All channels are ready, we are ready.
	setConnected(true);
	
	emit success(); //FIXME:Should be in JingleTransport and emitted when setConnected(true); is called.
}

QList<XMPP::Ice176::LocalAddress> JingleIceTransport::getAddresses()
{
	QList<Ice176::LocalAddress> ret;
	Ice176::LocalAddress addr;
	
	if (!d->manager->selfAddr().isNull())
		addr.addr = d->manager->selfAddr();
	else
		addr.addr = "127.0.0.1";
	
	addr.network = 0;

	return ret << addr;
}

void JingleIceTransport::slotIceLocalCandidatesReady(const QList<XMPP::Ice176::Candidate>& candidates)
{
	// Using mid to avoid sending local candidates already sent.
	if (!parentSession() || !parentSession()->isStarted())
		d->pendingLocalCandidates += candidates.mid(d->sentCandidates);
	else
		sendLocalCandidates(candidates.mid(d->sentCandidates));
}

void JingleIceTransport::sendLocalCandidates(const QList<XMPP::Ice176::Candidate>& candidates)
{
	foreach(Ice176::Candidate c, candidates)
	{
		//It is recommended to send each candidate in a separate iq.
		QDomDocument doc("");
		QDomElement transport = doc.createElement("transport");
		transport.setAttribute("xmlns", transportNS());
		transport.setAttribute("ufrag", d->ice176->localUfrag());
		transport.setAttribute("pwd", d->ice176->localPassword());
		transport.appendChild(candidateToXml(c));

		JT_JingleAction *tAction = new JT_JingleAction(rootTask());
		tAction->setSession(parentSession());
		tAction->transportInfo(parent()->name(), transport);
		tAction->go(true);
	}

	d->sentCandidates += candidates.count();
}

QDomElement JingleIceTransport::candidateToXml(const Ice176::Candidate& candidate)
{
	QDomDocument doc("");
	QDomElement c = doc.createElement("candidate");

	c.setAttribute("component", candidate.component);
	c.setAttribute("generation", candidate.generation);
	c.setAttribute("network", candidate.network);
	c.setAttribute("port", candidate.port);
	c.setAttribute("priority", candidate.priority);
	c.setAttribute("foundation", candidate.foundation);
	c.setAttribute("id", candidate.id);
	c.setAttribute("protocol", candidate.protocol);
	c.setAttribute("type", candidate.type);
	c.setAttribute("ip", candidate.ip.toString());

	if (candidate.rel_port != -1)
		c.setAttribute("rel-port", candidate.rel_port);
	if (candidate.rem_port != -1)
		c.setAttribute("rem-port", candidate.rem_port);
	if (candidate.rel_addr.toString() != "")
		c.setAttribute("rel-addr", candidate.rel_addr.toString());
	if (candidate.rem_addr.toString() != "")
		c.setAttribute("rem-addr", candidate.rem_addr.toString());

	return c;
}

void JingleIceTransport::slotIceStarted()
{
	d->iceStarted = true;
	
	if (!d->pendingRemoteCandidates.isEmpty())
	{
		d->ice176->addRemoteCandidates(d->pendingRemoteCandidates);
		d->pendingRemoteCandidates.clear();
	}
	
	emit started();
}

void JingleIceTransport::addTransportInfo(const QDomElement& elem)
{
	QDomElement t = elem.firstChildElement();

	if (d->remoteUfrag.isNull())
	{
		d->remoteUfrag = t.attribute("ufrag");
		d->ice176->setPeerUfrag(d->remoteUfrag);
	}
	if (d->remotePassword.isNull())
	{
		d->remotePassword = t.attribute("pwd");
		d->ice176->setPeerPassword(d->remotePassword);
	}
	
	QDomElement c = t.firstChildElement();
	QList<Ice176::Candidate> cs;
	
	while (!c.isNull())
	{
		if (c.tagName() != "candidate")
		{
			qDebug() << "This is not a candidate";
			c = c.nextSiblingElement();
			continue;
		}

		//addRemoteCandidate(c); FIXME:Do we have to keep a list of candidates ?

		Ice176::Candidate candidate = xmlToCandidate(c);
		
		if (candidate.component > d->componentCount)
		{
			qDebug() << "Component out of bound (component" << candidate.component << "but only" << d->componentCount << "available). Ignoring this candidate.";
			c = c.nextSiblingElement();
			continue;
		}
		
		cs << candidate;

		c = c.nextSiblingElement();
	}
	
	if (d->iceStarted)
		d->ice176->addRemoteCandidates(cs);
	else
		d->pendingRemoteCandidates << cs;
}

Ice176::Candidate JingleIceTransport::xmlToCandidate(const QDomElement& c)
{
	//FIXME:which ones are optionnal ?
	Ice176::Candidate ret;
	bool ok;

	ret.component = c.attribute("component").toInt(&ok);
	ret.generation = c.attribute("generation").toInt(&ok);
	ret.network = c.attribute("network").toInt(&ok);
	ret.port = c.attribute("port").toInt(&ok);
	ret.priority = c.attribute("priority").toInt(&ok);
	ret.foundation = c.attribute("foundation");
	ret.id = c.attribute("id");
	ret.protocol = c.attribute("protocol");
	ret.type = c.attribute("type");
	ret.ip = c.attribute("ip");
	if (c.hasAttribute("rel-port"))
		ret.rel_port = c.attribute("rel-port").toInt(&ok);
	if (c.hasAttribute("rem-port"))
		ret.rem_port = c.attribute("rem-port").toInt(&ok);
	if (c.hasAttribute("rel-addr"))
		ret.rel_addr = c.attribute("rel-addr");
	if (c.hasAttribute("rem-addr"))
		ret.rem_addr = c.attribute("rem-addr");

	if (!ok)
		qDebug() << "error parsing candidate";
	
	return ret;
}

QString JingleIceTransport::transportNS() const
{
	return NS_JINGLE_TRANSPORTS_ICE;
}

void JingleIceTransport::writeDatagram(const QByteArray& data, Channel channel)
{
	d->ice176->writeDatagram(static_cast<int>(channel), data);
}

QByteArray JingleIceTransport::readAll(Channel channel)
{
	return d->ice176->readDatagram(static_cast<int>(channel));
}

QDomElement JingleIceTransport::toXml(TransportType type)
{
	switch (type)
	{
	case NoCandidate :
		return transport();
	case LocalCandidates :
		//candidates are sent as soon as they are ready.
		/*Fall through*/
	default:
		return transport();
	}
}

