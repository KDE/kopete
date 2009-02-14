/*
 * jingleicecontent.cpp - Jingle Ice Content
 * Copyright (C) 2008 - Detlev Casanova <detlev.casanova@gmail.com>
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

#include "jingleicecontent.h"
#include "jinglesession.h"
#include "jinglesessionmanager.h"

#include <QDomElement>
#include <QTcpSocket>

//----------------------
// JingleIceContent
//----------------------

using namespace XMPP;

class JingleIceContent::Private
{
public:
	Private() :
//		rtpInSocket(0),
//		rtpOutSocket(0),
//		rtcpInSocket(0),
//		rtcpOutSocket(0),
		ice176(0)
		{}
//	QUdpSocket rtpInSocket;
//	QUdpSocket rtpOutSocket;
//	QUdpSocket rtcpInSocket;
//	QUdpSocket rtcpOutSocket;

	Ice176 *ice176;
	
	QString remoteUfrag;
	QString remotePassword;

	bool iceStarted;
	
	QList<Ice176::Candidate> pendingLocalCandidates;
	QList<Ice176::Candidate> pendingRemoteCandidates;

	QList<bool> channels;
};

JingleIceContent::JingleIceContent(Mode mode, JingleSession *parent)
: JingleContent(mode, parent), d(new Private())
{
	//TODO:check if hmac(sha1) is supported by QCA (qca-ossl)
	//if it is not, setting remote candidate will lead to a crash in QCA
	qDebug() << "Creating JingleIceContent";

	d->iceStarted = false;

	d->ice176 = new Ice176(this);

	connect(d->ice176, SIGNAL(started()), SLOT(slotIceStarted()));
	connect(d->ice176, SIGNAL(localCandidatesReady(const QList<XMPP::Ice176::Candidate>&)), SLOT(slotIceLocalCandidatesReady(const QList<XMPP::Ice176::Candidate>&)));
	connect(d->ice176, SIGNAL(componentReady(int)), SLOT(slotIceComponentReady(int)));
	connect(d->ice176, SIGNAL(readyRead(int)), SIGNAL(readyRead(int)));
	//connect(d->ice176, SIGNAL(datagramsWritten(int, int)), SLOT(slotIceDatagramsWritten(int, int)));

	d->ice176->setBasePort(rootTask()->client()->jingleSessionManager()->nextUdpPort());

	QList<Ice176::LocalAddress> addr = getAddresses();
	if (addr.count() == 0)
	{
		qDebug() << "No local addresses found. Unable to go further...";
		return;
	}

	d->ice176->setLocalAddresses(addr);

	d->channels << false << false;
	d->ice176->setComponentCount(2);//RTP + RTCP, I guess.

	JingleSessionManager *manager = rootTask()->client()->jingleSessionManager();
	
	if ((manager->stunPort() != -1) && !manager->stunAddress().isNull())
		d->ice176->setStunService(Ice176::Basic, manager->stunAddress(), manager->stunPort());

	switch (mode)
	{
	case Initiator:
		d->ice176->start(Ice176::Initiator);
		break;
	case Responder:
		d->ice176->start(Ice176::Responder);
		break;
	default :
		qDebug() << "JingleIceContent : unable to start Ice176.";
	return;
	}
	
	QDomDocument doc("");
	
	QDomElement transport = doc.createElement("transport");
	
	transport.setAttribute("xmlns", transportNS());
	transport.setAttribute("pwd", d->ice176->localPassword());
	transport.setAttribute("ufrag", d->ice176->localUfrag());
	
	setTransport(transport);
}

JingleIceContent::~JingleIceContent()
{
	delete d->ice176;
	delete d;
}

void JingleIceContent::slotIceComponentReady(int channel)
{
	qDebug() << "JingleIceContent::slotIceComponentReady";
	d->channels[channel] = true;

	// If not all channels are ready, we do nothing.
	for (int i = 0; i < d->channels.count(); i++)
		if (d->channels[i] == false)
			return;

	// All channels are ready, we are ready.
	setSending(true);
	setReceiving(true);
	//emit established();
}

QList<XMPP::Ice176::LocalAddress> JingleIceContent::getAddresses()
{
	//TODO:implement me
	QList<Ice176::LocalAddress> ret;
	Ice176::LocalAddress addr;
	
	JingleSessionManager *manager = rootTask()->client()->jingleSessionManager();

	if (!manager->selfAddr().isNull())
		addr.addr = manager->selfAddr();
	else
		addr.addr = "127.0.0.1";
	
	addr.network = 0;

	return ret << addr;
}

void JingleIceContent::slotIceLocalCandidatesReady(const QList<XMPP::Ice176::Candidate>& candidates)
{
	qDebug() << "slotIceLocalCandidatesReady called." << candidates.count() << "candidates ready.";

	if (!parentSession())
		d->pendingLocalCandidates += candidates;
	else
		sendCandidates(candidates);
}

void JingleIceContent::sendCandidates(const QList<XMPP::Ice176::Candidate>& candidates)
{
	if (!parentSession())
	{
		d->pendingLocalCandidates += candidates;
		return;
	}

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
		qDebug() << "content name =" << name();
		tAction->transportInfo(name(), transport);
		tAction->go(true);
	}
}

QDomElement JingleIceContent::candidateToXml(const Ice176::Candidate& candidate)
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

void JingleIceContent::slotIceStarted()
{
	qDebug() << "JingleIceContent::slotIceStarted() Called.";
	d->iceStarted = true;
	
	if (d->pendingRemoteCandidates.isEmpty())
		return;
	
	d->ice176->addRemoteCandidates(d->pendingRemoteCandidates);
}

void JingleIceContent::addCandidate(const QDomElement& elem)
{
	Q_UNUSED(elem)
}

void JingleIceContent::addTransportInfo(const QDomElement& elem)
{
	QDomElement t = elem.firstChildElement();

	if (d->remoteUfrag.isEmpty())
	{
		d->remoteUfrag = t.attribute("ufrag");
		d->ice176->setPeerUfrag(d->remoteUfrag);
	}
	if (d->remotePassword.isEmpty())
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

		cs << xmlToCandidate(c);
		
		c = c.nextSiblingElement();
	}

	qDebug() << "I have" << cs.count() << "remote candidates coming in.";

	if (d->iceStarted)
		d->ice176->addRemoteCandidates(cs);
	else
		d->pendingRemoteCandidates << cs;
}

Ice176::Candidate JingleIceContent::xmlToCandidate(const QDomElement& c)
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

QString JingleIceContent::transportNS() const
{
	return NS_JINGLE_TRANSPORTS_ICE;
}

void JingleIceContent::writeDatagram(const QByteArray& data, int channel)
{
	d->ice176->writeDatagram(channel, data);
}

QByteArray JingleIceContent::readAll(int channel)
{
	return d->ice176->readDatagram(channel);
}

