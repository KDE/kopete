/*
 * jinglecontent.cpp - Jingle content
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

#include "jinglecontent.h"

#include "jinglesession.h"

#include <QTimer>
#include <QDomElement>
#include <QUdpSocket>

//----------------------
// JingleContent
//----------------------

using namespace XMPP;


class JingleContent::Private
{
public:
	QList<QDomElement> payloads; // My payloads.
	QList<QDomElement> rPayloads; // Responder's payloads.
	QDomElement bestPayload;

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
	QTimer *outTimer;
	int tries;
};

JingleContent::JingleContent()
: d(new Private())
{
	qDebug() << "Creating JingleContent";
	d->sending = false;
	d->receiving = false;
	d->inSocket = 0L;
	d->outSocket = 0L;
	d->isInitiator = false;
	d->tries = 0;
}

JingleContent::~JingleContent()
{
	//delete d->inSocket;
	//delete d->outSocket;
	delete d;
}

QDomElement JingleContent::bestPayload()
{
	if (d->bestPayload.isNull())
	{
		//Trying to update the best payload.
		d->bestPayload = bestPayload(d->rPayloads, d->payloads);
	}
	return d->bestPayload;
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
	// This content is created from XML data, that means that it comes from the outside.
	// So, pyloads are added as responder payloads
	QList<QDomElement> payloads;
	while (!payload.isNull())
	{
		payloads << payload;
		payload = payload.nextSiblingElement();
	}
	setResponderPayloads(payloads);

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
	if (transport.attribute("xmlns") == NS_JINGLE_TRANSPORTS_ICE)
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
	else if (transport.attribute("xmlns") == NS_JINGLE_TRANSPORTS_RAW)
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

/*FIXME:this as no sense, this content is for RAW UDP only*/
QString JingleContent::iceUdpPassword()
{
	if (d->transport.attribute("xmlns") == NS_JINGLE_TRANSPORTS_ICE)
		return d->transport.attribute("pwd");
	return "";
}

/*FIXME:this as no sense, this content is for RAW UDP only*/
QString JingleContent::iceUdpUFrag()
{
	if (d->transport.attribute("xmlns") == NS_JINGLE_TRANSPORTS_ICE)
		return d->transport.attribute("ufrag");
	return "";
}

void JingleContent::createUdpInSocket()
{
	if (d->transport.attribute("xmlns") != NS_JINGLE_TRANSPORTS_RAW)
		return;
	qDebug() << "JingleContent::createUdpInSocket()";
	
	if (!d->inSocket)
		d->inSocket = new QUdpSocket();
	
	QHostAddress address(d->transport.firstChildElement().attribute("ip"));
	int port = d->transport.firstChildElement().attribute("port").toInt();
	qDebug() << "Bind socket to" << address << ":" << port;
	if (d->inSocket->bind(/*address, */port))
		qDebug() << "Socket bound to" << /*address.toString() << ":" <<*/ port;
	
	connect(d->inSocket, SIGNAL(readyRead()), this, SLOT(slotRawUdpDataReady()));
	//emit inSocketReady(); --> also no need of this.
}

void JingleContent::slotRawUdpDataReady()
{
	qDebug() << "slotRawUdpDataReady() :: Data arrived on the socket.";
	emit dataReceived();
	setReceiving(true);
	disconnect(sender(), SIGNAL(readyRead()), this, 0);
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
	
	// We do not need to try sending anymore, we have proof that data sending is OK.
	d->outTimer->stop();
	delete d->outTimer;

	// If we are also receiving, that's ok, this content is established.
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
	//This correspond to the trying phase.
	//Create udp OUT socket
	if (!d->outSocket)
		d->outSocket = new QUdpSocket();
	d->outSocket->connectToHost(address, port);
	//emit outSocketReady(); --> This signal has no sense anymore, we must prepare rtp sessions when the sockets are both ready.
	
	qDebug() << "Sending data to" << address.toString() << ":" << port;
	//We will start sending "SYN" every 5 seconds for 1 minute until we receive a received informationnal message.
	slotTrySending();
	d->outTimer = new QTimer();
	d->outTimer->setInterval(5000);
	connect(d->outTimer, SIGNAL(timeout()), this, SLOT(slotTrySending()));
	//setSending(true); --> set it when the received informationnal message has been received.
}

void JingleContent::slotTrySending()
{
	d->tries++;
	if (d->tries == 13)
	{
		//This content cannot connect, what do we do ?
		d->outTimer->stop();
		qDebug() << "JingleContent::slotTrySending : Unable to establish the connection for content" << name();
	}

	d->outSocket->write(QByteArray("SYN"));
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
	
	//emit inSocketReady();
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

void JingleContent::setResponderPayloads(const QList<QDomElement>& payloads)
{
	qDebug() << "*******Setting responder payloads**********";
	d->rPayloads = payloads;
	if (d->payloads.count() != 0) //No, if payloads is empty, we should get the list from the supported payloads. Actually, those payloads should be always set when creating the content.
	{
		//Store the best payload to use for this content.
		//The application will just have to get it from this content.
		d->bestPayload = bestPayload(d->rPayloads, d->payloads);
	}
}

QList<QDomElement> JingleContent::responderPayloads() const
{
	return d->rPayloads;
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

QDomElement JingleContent::bestPayload(const QList<QDomElement>& payload1, const QList<QDomElement>& payload2)
{
	//FIXME : this is not the best algorithm to determine which one is the better.
	// |-------|
	// | a | c |
	// +---+---+
	// | b | b |
	// +---+---+
	// | d | e |
	// +---+---+
	// | c | a |
	// |-------|
	//  --> In that case, payload a will be chosen but payload b would be the best choice.
	for (int i = 0; i < payload1.count(); i++)
	{
		for (int j = 0; j < payload2.count(); j++)
		{
			if (samePayload(payload1[i], payload2[j]))
				return payload1[i];
		}
	}
	qDebug() << "Returns QDomElement !";
	return QDomElement();
}

bool JingleContent::samePayload(const QDomElement& p1, const QDomElement& p2)
{
	// Checking payload-type attributes
	if (!p1.hasAttribute("id") || !p2.hasAttribute("id"))
		return false;
	if (p1.attribute("id") != p2.attribute("id"))
		return false;
	int id = p1.attribute("id").toInt();
	if ((id >= 96) && (id <= 127)) //dynamic payloads, "name" attribute must be there
	{
		if (!p1.hasAttribute("name") || !p2.hasAttribute("name"))
			return false;
		if (p1.attribute("name") != p2.attribute("name"))
			return false;
	}
	if (p1.hasAttribute("channels") && p2.hasAttribute("channels"))
		if (p1.attribute("channels") != p2.attribute("channels"))
			return false;
	if (p1.hasAttribute("clockrate") && p2.hasAttribute("clockrate"))
		if (p1.attribute("clockrate") != p2.attribute("clockrate"))
			return false;
	// Parameters are informative, even if they differ, the payload is stil the same.
	qDebug() << "Payloads are the same.";
	return true;
}
