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
	QList<QDomElement> rPayloads; // Remote's payloads.
	
	QDomElement bestPayload;
	
	QList<QDomElement> localCandidates; // Local candidates.
	QList<QDomElement> remoteCandidates; // Remote's candidates.

	Task *rootTask;
	QDomElement transport;
	QList<QDomElement> candidates;
	QString creator;
	QString name;
	QString descriptionNS;
	bool sending;
	bool receiving;
	Type type;
	Mode mode;
	JingleSession *parent;
};

JingleContent::JingleContent(Mode mode, JingleSession *parent)
: d(new Private())
{
	if (mode == Initiator)
		qDebug() << "Creating a JingleContent as initiator.";
	else if (mode == Responder)
		qDebug() << "Creating a JingleContent as responder.";
	else
		qDebug() << "Creating an unknown JingleContent.";

	d->parent = parent;
	d->mode = mode;
	d->rootTask = parent->rootTask();
	d->sending = false;
	d->receiving = false;
}

JingleContent::~JingleContent()
{
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

void JingleContent::addCandidate(const QDomElement& c) //Weird..., that's only stored, never used...
{
	d->candidates << c;
}

void JingleContent::addLocalPayload(const QDomElement& pl)
{
	d->payloads << pl;
}

void JingleContent::addLocalPayloads(const QList<QDomElement>& pl)
{
	d->payloads << pl;
}

void JingleContent::setLocalPayloads(const QList<QDomElement>& pl)
{
	d->payloads.clear();
	d->payloads = pl;
}

void JingleContent::setTransport(const QDomElement& t)
{
	d->transport = t;
}

QList<QDomElement> JingleContent::localPayloads() const
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
	qDebug() << "name set :" << name();
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
	
	setRemotePayloads(payloads);

	QDomElement transport = desc.nextSiblingElement();
	d->transport = transport;

	if (d->transport.hasChildNodes())
	addRemoteCandidate(d->transport.firstChildElement());

	// Send candidates as soon as possible, so it's now.
	// FIXME: is that true with all transport methods ?
	sendCandidates();
}

void JingleContent::sendCandidates()
{

}

QDomElement JingleContent::contentElement(JingleContent::CandidateType cType, JingleContent::PayloadType pType)
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
	
	switch (pType)
	{
	case LocalPayloads :
		qDebug() << "adding local payloads.";
		for (int i = 0; i < localPayloads().count(); i++)
		{
			description.appendChild(localPayloads().at(i));
		}
		break;
	case RemotePayloads :
		qDebug() << "adding remote payloads.";
		for (int i = 0; i < remotePayloads().count(); i++)
		{
			description.appendChild(remotePayloads().at(i));
		}
		break;
	case UsedPayload :
		qDebug() << "adding best payload.";
		description.appendChild(d->bestPayload);
	default :
		break;
	}

	QDomElement t = transport();
	//doc.createElement("transport");
	//transport.setAttribute("xmlns", transportNS());

	switch (cType)
	{
	case LocalCandidates :
		for (int i = 0; i < localCandidates().count(); i++)
			t.appendChild(localCandidates().at(i));
		break;
	case RemoteCandidates :
		for (int i = 0; i < remoteCandidates().count(); i++)
			t.appendChild(remoteCandidates().at(i));
		break;
	case LocalCandidate :
		if (localCandidates().count() > 0)
			t.appendChild(localCandidates().at(0)); //FIXME:that should be the used candidate.
		break;
	case RemoteCandidate :
		if (remoteCandidates().count() > 0)
			t.appendChild(remoteCandidates().at(0)); //FIXME:idem
		break;
	case NoCandidate :
	default :
		break;
	}
	
	content.appendChild(description);
	content.appendChild(t);

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

void JingleContent::addTransportInfo(const QDomElement& e) //virtual maybe ?
{
	QDomElement transport = e.firstChildElement();
	
/*	if (transport.attribute("xmlns") == NS_JINGLE_TRANSPORTS_ICE)
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
		
		startSending(QHostAddress(transport.firstChildElement().attribute("ip")),
			     transport.firstChildElement().attribute("port").toInt());
		
	}
	*/
	// That's exactly the kind of things to avoid.
}

/*void JingleContent::createUdpInSocket()
{
	qDebug() << "JingleContent::createUdpInSocket()";

	if (d->transport.attribute("xmlns") != NS_JINGLE_TRANSPORTS_RAW)
		return;
	
	if (!d->inSocket)
		d->inSocket = new QUdpSocket();
	
	QHostAddress address(d->transport.firstChildElement().attribute("ip"));
	int port = d->transport.firstChildElement().attribute("port").toInt();
	qDebug() << "Bind socket to" << address << ":" << port;
	if (d->inSocket->bind(address, port))
		qDebug() << "Socket bound to" << address.toString() << ":" << port;
	else
	{
		qDebug() << "Unable to bind socket to" << address.toString() << ":" << port;
		return;
	}
	
	setReceiving(true);
}*/

/*QUdpSocket *JingleContent::inSocket() //Not used anymore, use encapsulation.
{
	qDebug() << "Getting IN socket from content" << name();
	return d->inSocket;
}*/

/*QUdpSocket *JingleContent::outSocket() //Not used anymore, use encapsulation.
{
	qDebug() << "Getting OUT socket from content" << name();
	return d->outSocket;
}*/

bool JingleContent::sending()
{
	return d->sending;
}

void JingleContent::setSending(bool s)
{
	if (d->sending == s)
		return;
	d->sending = s;
	
	// If we are also receiving, that's ok, this content's connection is established.
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

/*void JingleContent::startSending() //Not used anymore.
{
	QHostAddress address(transport().firstChildElement().attribute("ip"));
	int port = transport().firstChildElement().attribute("port").toInt();
	
	//startSending(address, port);
}

void JingleContent::startSending(const QHostAddress& address, int port) //idem
{
	qDebug() << "startSending()";
	
	if (!d->outSocket)
		d->outSocket = new QUdpSocket();
	d->outSocket->connectToHost(address, port);
	
	qDebug() << "Ok, we can start sending" << address.toString() << ":" << port;
	
	setSending(true);
}*/

QList<QDomElement> JingleContent::candidates() const
{
	return d->candidates;
}

QString JingleContent::creator() const
{
	return d->creator;
}

/*void JingleContent::bind(const QHostAddress& address, int port)
{
	qDebug() << "Trying to bind socket to" << address.toString() << ":" << port;
	
	if (!d->inSocket)
		d->inSocket = new QUdpSocket();
	
	if (d->inSocket->bind(address, port))
		qDebug() << "Socket bound to" << address.toString() << ":" << port;
	
	setReceiving(true);
}*/

JingleContent& JingleContent::operator=(const JingleContent &other)
{
	d->payloads = other.localPayloads();
	d->transport = other.transport();
	//d->candidates = other.candidates();
	d->localCandidates = other.localCandidates();
	d->remoteCandidates = other.remoteCandidates();

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

void JingleContent::setRemotePayloads(const QList<QDomElement>& payloads)
{
	qDebug() << "Setting responder payloads.";
	
	d->rPayloads = payloads;
	
	if (d->payloads.count() != 0)
	{
		//Store the best payload to use for this content.
		//The application will just have to get it from this content.
		d->bestPayload = bestPayload(d->rPayloads, d->payloads);
	}
}

QList<QDomElement> JingleContent::remotePayloads() const
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
		return NoType;
}

QDomElement JingleContent::bestPayload(const QList<QDomElement>& payload1, const QList<QDomElement>& payload2)
{
	/*
	 * FIXME : this is not the best algorithm to determine which one is the better.
	 * |-------|
	 * | a | c |
	 * +---+---+
	 * | b | b |
	 * +---+---+
	 * | d | e |
	 * +---+---+
	 * | c | a |
	 * |-------|
	 *  --> In that case, payload a will be chosen but payload b would be the best choice.
	 */
	
	for (int i = 0; i < payload1.count(); i++)
	{
		for (int j = 0; j < payload2.count(); j++)
		{
			if (samePayload(payload1[i], payload2[j]))
			{
				qDebug() << "Best payload name :" << payload1[i].attribute("name");
				return payload1[i];
			}
		}
	}

	qDebug() << "No best payload found, maybe responder and/or local payloads are no set yet.";
	
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
	
	// Parameters are informative, even if they differ, the payload is still the same.
	// FIXME: is that statement true ?
	qDebug() << "Payloads are the same.";

	return true;
}

bool JingleContent::isReady() const
{
	return d->sending && d->receiving;
}

QString JingleContent::transportNS() const
{
	return "";
}

QString JingleContent::transportNS(const QDomElement& c)
{
	QString ret;
	QDomElement content = c;

	if (content.tagName() != "content")
	{
		qDebug() << "This is not a content xml element.";
		return ret;
	}

	QDomElement transport = content.firstChildElement();
	qDebug() << transport.tagName();
	while (!transport.isNull())
	{
		qDebug() << transport.tagName();
		if (transport.tagName() == "transport")
		{
			ret = transport.attribute("xmlns");
		}
		transport = transport.nextSiblingElement();
	}
	qDebug() << "Found transportNS :" << ret;

	return ret;
}

JingleSession *JingleContent::parentSession() const
{
	return d->parent;
}

QList<QDomElement> JingleContent::localCandidates() const
{
	qDebug() << "return" << d->localCandidates.count() << "candidates";
	return d->localCandidates;
}

void JingleContent::addLocalCandidate(const QDomElement& c)
{
	qDebug() << "Adding local candidate.";
	d->localCandidates << c;
}

QList<QDomElement> JingleContent::remoteCandidates() const
{
	qDebug() << "return" << d->remoteCandidates.count() << "candidates";
	return d->remoteCandidates;
}

void JingleContent::addRemoteCandidate(const QDomElement& c)
{
	d->remoteCandidates << c;
}

void JingleContent::setRootTask(Task *rt)
{
	d->rootTask = rt;
}

Task *JingleContent::rootTask() const
{
	return d->rootTask;
}

JingleContent::Mode JingleContent::mode() const
{
	return d->mode;
}

void JingleContent::writeDatagram(const QByteArray& ba, int)
{
	Q_UNUSED(ba)
}

QByteArray JingleContent::readAll(int)
{
	return QByteArray();
}
