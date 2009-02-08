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
	QUdpSocket *inSocket;
	QUdpSocket *outSocket;
};

JingleRawContent::JingleRawContent(Mode mode, JingleSession *parent, Task *rootTask)
: JingleContent(mode, parent, rootTask), d(new Private)
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

	d->inSocket = 0L;
	d->outSocket = 0L;
}

JingleRawContent::~JingleRawContent()
{
	delete d;
}

void JingleRawContent::setSession(JingleSession *sess)
{
	JingleContent::setSession(sess);

	if (mode() == Initiator)
		findCandidate();
}

void JingleRawContent::addCandidate(const QDomElement& c)
{
	JingleContent::addCandidate(c); //FIXME:addLocalCandidate() ??
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
	
	//createUdpOutSocket(QHostAddress(t.firstChildElement().attribute("ip")),
	//		   t.firstChildElement().attribute("port").toInt());
}

void JingleRawContent::createUdpInSocket()
{
	qDebug() << "JingleRawContent::createUdpInSocket()";

	if (transport().attribute("xmlns") != NS_JINGLE_TRANSPORTS_RAW)
		return;
	
	if (!d->inSocket)
		d->inSocket = new QUdpSocket();
	
	QHostAddress address(transport().firstChildElement().attribute("ip"));
	int port = transport().firstChildElement().attribute("port").toInt();
	qDebug() << "Bind socket to" << address << ":" << port;
	if (d->inSocket->bind(address, port))
		qDebug() << "Socket bound to" << address.toString() << ":" << port;
	else
	{
		qDebug() << "Unable to bind socket to" << address.toString() << ":" << port;
		return;
	}
	
	connect(d->inSocket, SIGNAL(readyRead()), this, SIGNAL(readyRead()));
	
	setReceiving(true);
}

void JingleRawContent::createUdpOutSocket(const QHostAddress& address, int port)
{
	qDebug() << "createUdpOutSocket()";
	
	if (!d->outSocket)
		d->outSocket = new QUdpSocket();
	d->outSocket->connectToHost(address, port);
	
	qDebug() << "Ok, we can start sending" << address.toString() << ":" << port;
	
	setSending(true);
}

void JingleRawContent::bind(const QHostAddress& address, int port)
{
	qDebug() << "Trying to bind socket to" << address.toString() << ":" << port;
	
	if (!d->inSocket)
		d->inSocket = new QUdpSocket();
	
	if (d->inSocket->bind(address, port))
		qDebug() << "Socket bound to" << address.toString() << ":" << port;
	
	connect(d->inSocket, SIGNAL(readyRead()), this, SIGNAL(readyRead()));
	
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
	int port = rootTask()->client()->jingleSessionManager()->nextRawUdpPort();
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

void JingleRawContent::writeDatagram(const QByteArray& ba)
{
	d->outSocket->write(ba);
}

QByteArray JingleRawContent::readAll()
{
	//qDebug() << "JingleRawContent::readAll()";
	QByteArray ret;
	ret.resize(d->inSocket->pendingDatagramSize());
	d->inSocket->readDatagram(ret.data(), d->inSocket->pendingDatagramSize());
	return ret;
}

