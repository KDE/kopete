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
#include "ice176.h"

#include <QDomElement>
#include <QUdpSocket>

//----------------------
// JingleIceContent
//----------------------

using namespace XMPP;


class JingleIceContent::Private
{
public:
	Private() :
		rtpInSocket(0),
		rtpOutSocket(0),
		rtcpInSocket(0),
		rtcpOutSocket(0),
		ice176(0)
		{}
	QUdpSocket rtpInSocket;
	QUdpSocket rtpOutSocket;
	QUdpSocket rtcpInSocket;
	QUdpSocket rtcpOutSocket;

	Ice176 *ice176;

	int nothing;
};

JingleIceContent::JingleIceContent(Mode mode, JingleSession *parent, Task *rootTask)
: JingleContent(mode, parent, rootTask), d(new Private())
{
	qDebug() << "Creating JingleIceContent";

	d->ice176 = new Ice176(this);

	connect(d->ice176, SIGNAL(started()), SLOT(slotIceStarted()));
	connect(d->ice176, SIGNAL(localCandidatesReady(const QList<XMPP::Ice176::Candidate> &)), SLOT(slotIceLocalCandidatesReady(const QList<XMPP::Ice176::Candidate> &)));
	connect(d->ice176, SIGNAL(componentReady(int)), SLOT(slotIceComponentReady(int)));
	connect(d->ice176, SIGNAL(readyRead(int)), SLOT(slotIceReadyRead(int)));
	connect(d->ice176, SIGNAL(datagramsWritten(int, int)), SLOT(slotIceDatagramsWritten(int, int)));

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
	
	//Creating transport element.
	QDomDocument doc("");
	QDomElement transport = doc.createElement("transport");
	transport.setAttribute("xmlns", transportNS());
	qDebug() << "Local password =" << d->ice176->localPassword();
	transport.setAttribute("pwd", d->ice176->localPassword());
	qDebug() << "Local ufrag =" << d->ice176->localUfrag();
	transport.setAttribute("ufrag", d->ice176->localUfrag());
	setTransport(transport);
}

JingleIceContent::~JingleIceContent()
{
	delete d;
}


void JingleIceContent::addCandidate(const QDomElement& elem)
{
	Q_UNUSED(elem)
}

void JingleIceContent::addTransportInfo(const QDomElement& elem)
{
	QDomElement t = elem.firstChildElement();

	d->ice176->setPeerUfrag(t.attribute("ufrag"));
	d->ice176->setPeerPassword(t.attribute("pwd"));
}

QString JingleIceContent::transportNS() const
{
	return NS_JINGLE_TRANSPORTS_ICE;
}

void JingleIceContent::setSession(JingleSession *sess)
{
	Q_UNUSED(sess)
}

void JingleIceContent::writeDatagram(const QByteArray& data)
{
	Q_UNUSED(data)
}

QByteArray JingleIceContent::readAll()
{

}
