/*
 * jinglecontent.cpp - Jingle content
 * Copyright (C) 2004  Justin Karneges
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

#include "jinglecontent.h"

//----------------------
// JingleContent
//----------------------

using namespace XMPP;

class JingleContent::Private
{
public:
	QList<QDomElement> payloads;
	QDomElement transport;
	QList<QDomElement> candidates;
	QString creator;
	QString name;
	QString descriptionNS;
	//The application will access this socket directly, Iris has not to deal with RTP.
	QUdpSocket *inSocket; //Currently, this is the IN raw-udp socket for this content.
	QUdpSocket *outSocket; //Currently, this is the OUT raw-udp socket for this content.
	bool sending;
	bool receiving;
	Type type;
	bool isInitiator;
};

JingleContent::JingleContent()
: d(new Private())
{
	d->sending = false;
	d->receiving = false;
	d->inSocket = 0L;
	d->outSocket = 0L;
	d->isInitiator = false;
}

JingleContent::~JingleContent()
{
	delete d->inSocket;
	delete d->outSocket;
	delete d;
}

void JingleContent::addCandidate(const QDomElement& c)
{
	d->candidates << c;
}

void JingleContent::addPayloadType(const QDomElement& pl)
{
	d->payloads << pl;
}

void JingleContent::addPayloadTypes(const QList<QDomElement>& pl)
{
	d->payloads << pl;
}

void JingleContent::setPayloadTypes(const QList<QDomElement>& pl)
{
	d->payloads.clear();
	d->payloads = pl;
}

void JingleContent::setTransport(const QDomElement& t)
{
	d->transport = t;
}

QList<QDomElement> JingleContent::payloadTypes() const
{
	return d->payloads;
}

QDomElement JingleContent::transport() const
{
	return d->transport;
}

void JingleContent::setCreator(const QString& c)
{
	d->creator = c;
}

void JingleContent::setName(const QString& n)
{
	d->name = n;
}

void JingleContent::setDescriptionNS(const QString& desc)
{
	d->descriptionNS = desc;
}

void JingleContent::fromElement(const QDomElement& e)
{
	// FIXME:tag order may not always be the same !!!
	if (e.tagName() != "content")
		return;
	d->creator = e.attribute("creator");
	d->name = e.attribute("name");
	QDomElement desc = e.firstChildElement();
	d->descriptionNS = desc.attribute("xmlns");
	d->type = stringToType(desc.attribute("media"));
	QDomElement payload = desc.firstChildElement();
	while (!payload.isNull())
	{
		d->payloads << payload;
		payload = payload.nextSiblingElement();
	}
	QDomElement transport = desc.nextSiblingElement();
	d->transport = transport;
}

QDomElement JingleContent::contentElement()
{
	// Create the QDomElement which has to be returned.
	QDomDocument doc("");
	
	QDomElement content = doc.createElement("content");
	content.setAttribute("creator", d->creator);
	content.setAttribute("name", d->name);
	content.setAttribute("sender", "both"); //Setting to default currently, change it !
	
	QDomElement description = doc.createElement("description");
	description.setAttribute("xmlns", d->descriptionNS);
	description.setAttribute("media", typeToString(d->type));

	for (int i = 0; i < d->payloads.count(); i++)
	{
		description.appendChild(d->payloads.at(i));
	}
	content.appendChild(description);
	content.appendChild(d->transport);

	return content;
}

QString JingleContent::name() const
{
	return d->name;
}

QString JingleContent::descriptionNS() const
{
	return d->descriptionNS;
}

void JingleContent::addTransportInfo(const QDomElement& e)
{
	QDomElement transport = e.firstChildElement();
	if (transport.attribute("xmlns") == "urn:xmpp:tmp:jingle:transports:ice-udp")
	{
		if (d->transport.attribute("pwd") != transport.attribute("pwd"))
		{
			qDebug() << "Bad ICE Password !";
			return;
		}
		
		if (d->transport.attribute("ufrag") != transport.attribute("ufrag"))
		{
			qDebug() << "Bad ICE User Fragment !";
			return;
		}
		QDomElement child = transport.firstChildElement();
		//FIXME:Is it possible to have more than one candidate per transport-info ?
		//	See Thread "Jingle: multiple candidates per transport-info?" on xmpp-standards.
		if (child.tagName() == "candidate")
		{
			// Just adding the Xml Element.
			d->candidates << child;
		}
	}
	else if (transport.attribute("xmlns") == "urn:xmpp:tmp:jingle:transports:raw-udp")
	{
		qDebug() << "Adding responder's candidates and connecting to it";
		d->candidates << transport.firstChildElement();
		//TODO : Start connection to this candidate.
		//WARNING : as Jingle specification is not clear,
		//	    the connexion will be considered as established
		//	    even without receiving the "received" informational
		//	    message.
		startSending(QHostAddress(transport.firstChildElement().attribute("ip")),
			     transport.firstChildElement().attribute("port").toInt());
		
	}
}

QString JingleContent::iceUdpPassword()
{
	if (d->transport.attribute("xmlns") == "urn:xmpp:tmp:jingle:transports:ice-udp")
		return d->transport.attribute("pwd");
	return "";
}

QString JingleContent::iceUdpUFrag()
{
	if (d->transport.attribute("xmlns") == "urn:xmpp:tmp:jingle:transports:ice-udp")
		return d->transport.attribute("ufrag");
	return "";
}

void JingleContent::createUdpInSocket()
{
	if (d->transport.attribute("xmlns") != "urn:xmpp:tmp:jingle:transports:raw-udp")
		return;
	qDebug() << "JingleContent::createUdpInSocket()";
	
	if (!d->inSocket)
		d->inSocket = new QUdpSocket();
	
	//The problem is the fact that a socket cannot be Bound _and_ connected at the same time --> there is now a IN socket an a OUT socket
	QHostAddress address(d->transport.firstChildElement().attribute("ip"));
	int port = d->transport.firstChildElement().attribute("port").toInt();
	qDebug() << "Bind socket to" << address << ":" << port;
	if (d->inSocket->bind(address, port))
		qDebug() << "Socket bound to" << address.toString() << ":" << port;
	
	connect(d->inSocket, SIGNAL(readyRead()), this, SLOT(slotRawUdpDataReady()));
	emit inSocketReady();
}

void JingleContent::slotRawUdpDataReady()
{
	qDebug() << "Data arrived on the socket.";
	setReceiving(true);
	disconnect(sender(), 0, this, 0);
}

QUdpSocket *JingleContent::inSocket()
{
	qDebug() << "Getting IN socket from content" << name();
	return d->inSocket;
}

QUdpSocket *JingleContent::outSocket()
{
	qDebug() << "Getting OUT socket from content" << name();
	return d->outSocket;
}

bool JingleContent::sending()
{
	return d->sending;
}

void JingleContent::setSending(bool s)
{
	if (d->sending == s)
		return;
	d->sending = s;
	if (d->sending && d->receiving)
	{
		qDebug() << "setSending : emit established() SIGNAL";
		emit established();
	}
}

bool JingleContent::receiving()
{
	return d->receiving;
}

void JingleContent::setReceiving(bool r)
{
	if (d->receiving == r)
		return;
	d->receiving = r;
	if (d->sending && d->receiving)
	{
		qDebug() << "setReceiving : emit established() SIGNAL";
		emit established();
	}
}

void JingleContent::startSending()
{
	QHostAddress address(transport().firstChildElement().attribute("ip"));
	int port = transport().firstChildElement().attribute("port").toInt();
	startSending(address, port);
}

void JingleContent::startSending(const QHostAddress& address, int port)
{
	//Create udp OUT socket
	if (!d->outSocket)
		d->outSocket = new QUdpSocket();
	d->outSocket->connectToHost(address, port);
	emit outSocketReady();
	qDebug() << "Sending data to" << address.toString() << ":" << port;
	qDebug() << "EMIT needData(c) SIGNAL";
	setSending(true);
	emit needData(this); //FIXME:We can Use sender to know which content sent the signal.
}

QList<QDomElement> JingleContent::candidates() const
{
	return d->candidates;
}

QString JingleContent::creator() const
{
	return d->creator;
}

void JingleContent::bind(const QHostAddress& address, int port)
{
	qDebug() << "Trying to bind socket to" << address.toString() << ":" << port;
	if (!d->inSocket)
		d->inSocket = new QUdpSocket();
	if (d->inSocket->bind(address, port))
		qDebug() << "Socket bound to" << address.toString() << ":" << port;
	
	connect(d->inSocket, SIGNAL(readyRead()), this, SLOT(slotRawUdpDataReady()));
	
	emit inSocketReady();
}



JingleContent& JingleContent::operator=(const JingleContent &other)
{
	d->payloads = other.payloadTypes();
	d->transport = other.transport();
	d->candidates = other.candidates();
	d->creator = other.creator();
	d->name = other.name();
	d->descriptionNS = other.descriptionNS();
	
	return *this;
}

void JingleContent::setType(JingleContent::Type t)
{
	d->type = t;
}

JingleContent::Type JingleContent::type() const
{
	return d->type;
}

QString JingleContent::typeToString(JingleContent::Type t)
{
	switch(t)
	{
	case Video :
		return "video";
	case Audio :
		return "audio";
	case FileTransfer :
		return "file transfer";
	default:
		return "unknown";
	}
}

JingleContent::Type JingleContent::stringToType(const QString& s)
{
	if (s == "video")
		return Video;
	else if (s == "audio")
		return Audio;
	else if (s == "file transfer")
		return FileTransfer;
	else
		return Unknown;
}
