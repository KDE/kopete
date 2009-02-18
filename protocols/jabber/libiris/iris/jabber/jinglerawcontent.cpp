/*
 * jinglerawcontent.cpp - Jingle content using RAW-UDP transport method
 * Copyright (C) 2008-2009 - Detlev Casanova <detlev.casanova@gmail.com>
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

#include "jinglerawcontent.h"
#include "jinglesessionmanager.h"
#include "jinglesession.h"

#include <QDomElement>
#include <QUdpSocket>
#include <QNetworkInterface>

//----------------------
// JingleRawContent
//----------------------

using namespace XMPP;

class JingleRawContent::Private
{
public:
	QUdpSocket* inSocket[2];
	QUdpSocket* outSocket[2];
};

JingleRawContent::JingleRawContent(Mode mode, JingleSession *parent)
: JingleContent(mode, parent), d(new Private)
{
	qDebug() << "Creating JingleRawContent";
	if (mode == Initiator && parent != 0)
	{
		//We must add the local candidate
		findCandidate();
	}

	// Create transport element :
	QDomDocument doc("");
	QDomElement transport = doc.createElement("transport");
	transport.setAttribute("xmlns", NS_JINGLE_TRANSPORTS_RAW);
	setTransport(transport);

	d->inSocket[0] = 0;
	d->inSocket[1] = 0;
	d->outSocket[0] = 0;
	d->outSocket[1] = 0;
}

JingleRawContent::~JingleRawContent()
{
	delete d;
}

void JingleRawContent::addRemoteCandidate(const QDomElement& c)
{
	qDebug() << "Adding remote candidate (" << c.tagName() << ").";
	if (c.tagName() == "candidate")
		createUdpOutSocket(QHostAddress(c.attribute("ip")), c.attribute("port").toInt());

	JingleContent::addRemoteCandidate(c);
}

void JingleRawContent::addTransportInfo(const QDomElement& e)
{
	qDebug() << "Adding responder's candidates and connecting to it";
	
	QDomElement t = e.firstChildElement();
	
	addRemoteCandidate(t.firstChildElement());
}

void JingleRawContent::createUdpInSocket()
{
	
	setReceiving(true);
}

void JingleRawContent::slotReadyRead()
{
	if (sender() == d->inSocket[0])
		emit readyRead(0);
	else if (sender() == d->inSocket[1])
		emit readyRead(1);
}

void JingleRawContent::createUdpOutSocket(const QHostAddress& address, int port)
{
	qDebug() << "createUdpOutSocket()";
	
	if (!d->outSocket[0])
		d->outSocket[0] = new QUdpSocket();
	d->outSocket[0]->connectToHost(address, port);
	
	if (!d->outSocket[1])
		d->outSocket[1] = new QUdpSocket();
	d->outSocket[1]->connectToHost(address, port + 1);
	
	qDebug() << "Ok, we can start sending" << address.toString() << ":" << port;
	
	setSending(true);
}

void JingleRawContent::bind(const QHostAddress& address, int port)
{
	qDebug() << "Trying to bind socket to" << address.toString() << ":" << port;
	
	if (!d->inSocket[0])
		d->inSocket[0] = new QUdpSocket();
	
	if (!d->inSocket[1])
		d->inSocket[1] = new QUdpSocket();
	
	qDebug() << "Bind socket to" << address << ":" << port;
	
	if (d->inSocket[0]->bind(address, port))
		qDebug() << "Socket bound to" << address.toString() << ":" << port;
	else
	{
		qDebug() << "Unable to bind socket to" << address.toString() << ":" << port;
		return;
	}

	if (d->inSocket[1]->bind(address, port + 1))
		qDebug() << "Socket bound to" << address.toString() << ":" << port + 1;
	else
	{
		qDebug() << "Unable to bind socket to" << address.toString() << ":" << port + 1;
		return;
	}
	
	connect(d->inSocket[0], SIGNAL(readyRead()), this, SLOT(slotReadyRead()));
	connect(d->inSocket[1], SIGNAL(readyRead()), this, SLOT(slotReadyRead()));
	
	setReceiving(true);
}

static bool interfaceOrder(const QHostAddress& a1, const QHostAddress& a2)
{
	Q_UNUSED(a2)
	if ((a1 != QHostAddress::LocalHost) && (a1 != QHostAddress::Null) && (a1.protocol() != QAbstractSocket::IPv6Protocol))
		return true;
	return false;
}

QDomElement JingleRawContent::findCandidate()
{
	qDebug() << "JingleRawContent::findCandidate() called";
	QDomDocument doc("");
	//FIXME:Not sure about the definition of creator.

	QDomElement candidate = doc.createElement("candidate");
	QString ip;

	//Trying to get the address with the most chances to succeed.
	QNetworkInterface *interface = new QNetworkInterface();
	QList<QHostAddress> ips = interface->allAddresses();
	qSort(ips.begin(), ips.end(), interfaceOrder);

	if (ips.count() == 0)
	{
		qDebug() << "No Internet address found. Are you connected ?";
		//emit error(NoNetwork);
		return QDomElement();
	}
	ip = ips[0].toString();
	candidate.setAttribute("ip", ip); // ips[0] is not 127.0.0.1 if there is other adresses.
	int port = rootTask()->client()->jingleSessionManager()->nextUdpPort();
	candidate.setAttribute("port", QString("%1").arg(port));
	candidate.setAttribute("generation", QString("%1").arg(localCandidates().count()));
	
	addLocalCandidate(candidate);
	
	bind(QHostAddress(ip), port);

	return candidate;
}

void JingleRawContent::sendCandidates()
{
	QDomDocument doc("");
	QDomElement candidate = findCandidate();
	QDomElement transport = doc.createElement("transport");
	
	transport.setAttribute("xmlns", NS_JINGLE_TRANSPORTS_RAW);
	transport.appendChild(candidate);

	JT_JingleAction *tAction = new JT_JingleAction(rootTask());
	tAction->setSession(parentSession());
	qDebug() << "content name =" << name();
	tAction->transportInfo(name(), transport);
	tAction->go(true);
}

QString JingleRawContent::transportNS() const
{
	return NS_JINGLE_TRANSPORTS_RAW;
}

void JingleRawContent::writeDatagram(const QByteArray& ba, Channel channel)
{
	d->outSocket[(int)channel]->write(ba);
}

QByteArray JingleRawContent::readAll(Channel channel)
{
	QByteArray ret;

	ret.resize(d->inSocket[(int)channel]->pendingDatagramSize());
	d->inSocket[(int)channel]->readDatagram(ret.data(), d->inSocket[(int)channel]->pendingDatagramSize());

	return ret;
}

