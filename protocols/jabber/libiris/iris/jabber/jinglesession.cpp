/*
 * jinglesession.cpp - Jingle session
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

#include <QString>
#include <QUdpSocket>

#include "jinglesession.h"

using namespace XMPP;

static QString genSid()
{
	QString s;
	int id_seed = rand() % 0xffff;
	s.sprintf("a%x", id_seed);
	return s;
}

class JingleSession::Private
{
public:
	Jid to;
	QList<JingleContent*> contents;
	Task *rootTask;
	QString sid;
	QString initiator;
	JT_JingleSession *jt;
	State state;
	QStringList contentsToRemove;
	QStringList transports;
	bool responderTrying;
};

JingleSession::JingleSession(Task *t, const Jid &j)
: d(new Private)
{
	d->to = j;
	d->rootTask = t;
	d->state = Pending;
	d->responderTrying = false;
}

JingleSession::~JingleSession()
{
	delete d;
}

void JingleSession::addContent(JingleContent *c)
{
	d->contents << c;
}

void JingleSession::addContents(const QList<JingleContent*>& l)
{
	d->contents << l;
}

Jid JingleSession::to() const
{
	return d->to;
}

QList<JingleContent*> JingleSession::contents() const
{
	return d->contents;
}

void JingleSession::start()
{
	// Generate session ID
	d->sid = genSid();

	/*qDebug() << "There are" << contents().count() << "contents : ";
	for (int i = 0; i < contents().count(); i++)
	{
		qDebug() << i << ":";
		qDebug() << d->rootTask->client()->stream().xmlToString(contents()[i]->contentElement(), true);
	}*/

	JT_JingleAction *iAction = new JT_JingleAction(d->rootTask);
	iAction->setSession(this);
	iAction->initiate();
	for (int i = 0; i < contents().count(); i++)
	{
		if (contents()[i]->transport().attribute("xmlns") == "urn:xmpp:tmp:jingle:transports:raw-udp")
		{
			qDebug() << "Create IN socket for" << contents()[i]->name();
			//qDebug("Content Adress : %x\n", (unsigned int) contents()[i]);
			contents()[i]->createUdpInSocket();
			//connect(contents()[i], SIGNAL(rawUdpDataReady()), this, SLOT(slotRawUdpDataReady()));
		}
	}
}

void JingleSession::slotRawUdpDataReady()
{/*
	JingleContent *content = (JingleContent*) sender();
	//qDebug("Content Adress : %x\n", (unsigned int) content);
	if (content == NULL)
	{
		qDebug() << "Null Jingle content, there is a problem";
		return;
	}
	if (d->state == Pending)
	{
		content->setReceiving(true);
		// We check if each contents is receiving AND sending.
		// In this case, the state can be changed to Active (should emit active() signal)
		bool changeState = true;
		for (int i = 0; i < contents().count(); i++)
		{
			if ((!contents()[i]->receiving()) || (!contents()[i]->sending()))
			{
				changeState = false;
				break;
			}
		}
		if (changeState)
			d->state = Active;
	}
	QByteArray datagram;
	QHostAddress address;
	quint16 port;
	datagram.resize(content->socket()->pendingDatagramSize());
	content->socket()->readDatagram(datagram.data(), datagram.size(), &address, &port);
	qDebug() << "Receiving data for content" << content->name() << "from" << address.toString() << ":" << port << ":";
	qDebug() << datagram;

	// Send "received" informationnal message. --> No, doesn't.
	//JT_JingleAction *rAction = new JT_JingleAction(d->rootTask);
	//connect(rAction, SIGNAL(acked()), this, SLOT(slotReceivedAcked()));
	//rAction->setSession(this);
	//rAction->received();
*/
}

void JingleSession::acceptSession()
{
	qDebug() << "Starting payloads and transport negotiation.";

/*
 * That won't be done here : the negotiation must be done while ringing for
 * example and even as soon as the session-initiate is received.
 */
	

	/*
	 * Here, it becomes a bit complicated (well, not so much) :
	 * 	If we are in pending state, we must start negotiation but *NOT* send a session-accept action now !
	 * 	Also, at this point, JingleSession MUST know what payloads we wanna use for what content
	 * 	and if the transport is supported. 
	 * 	Also, This should be known from the very start of the application : what transport are supported and
	 * 	what payloads to use for a particular content (RTP/AVP,...).
	 * 	That is done in the JingleSessionManager which knows what transports and contents are supported.
	 * 	Access it by rootTask->client()->jingleSessionManager()
	 */
	// RAW UDP :
	//	Why should I start sending data as soon as I received the session-initiate action ?
	//		That does not make sense with Audio via RTP specification (I won't "immediately negotiate connectivity over the ICE transport by exchanging XML-formatted candidate transports for the channel" as I first negotiate contents...)
	//	Seeing the specification (XEP-0177), I can't tell the responder which contents I accept and which I don't.
	//
	
	// ICE-UDP :
	//	Why should I start sending data as soon as I received the session-initiate action ?
	//	Seeing the specification (XEP-0176), I can't tell the responder which contents I accept and which I don't.
	
	/*Maybe yes : do both things at the same time...*/

}

void JingleSession::acceptContent()
{
	/* FIXME:
	 * 	* The JT_JingleAction task should be in Private.
	 * 	* More than 1 JT_JingleAction could be present at the same time.
	 */
	JT_JingleAction *tAction = new JT_JingleAction(d->rootTask);
	tAction->setSession(this);
	tAction->contentAccept();
}

void JingleSession::removeContent(const QStringList& c)
{
	// Removing only existing contents.
	for (int i = 0; i < c.count(); i++)
	{
		for (int j = 0; j < contents().count(); j++)
		{
			if (c.at(i) == contents()[j]->name())
			{
				d->contentsToRemove << c[i];
			}
		}
	}
	if (d->contentsToRemove.count() <= 0)
		return;
	
	//d->contents.removeAt(i); //Or do it in the slotRemoveAcked() ?? --> Yes, we will remove all contents in d->contentsToRemove from d->contents when acked.
	JT_JingleAction *rAction = new JT_JingleAction(d->rootTask);
	connect(rAction, SIGNAL(acked()), this, SLOT(slotRemoveAcked()));
	rAction->setSession(this);
	rAction->removeContents(d->contentsToRemove);
}

void JingleSession::removeContent(const QString& c) // Provided for convenience, may disappear.
{
/*
 * From Jingle Specification : 
 * A client MUST NOT return an error upon receipt of a 'content-remove' action for a content
 * type that is received after a 'content-remove' action has been sent but before the action
 * has been acknowledged by the peer. 
 * 
 * If the content-remove results in zero content types for the session, the entity that receives
 * the content-remove SHOULD send a session-terminate action to the other party (since a session
 * with no content types is void).
 */
	bool found = false;
	//if (d->state != Active) /*FIXME:Should be if (d->state == Pending)*/
	{
		// FIXME:whatever the state is, the same thing will be done here...
		// Checking if that content exists.
		int i = 0;
		for ( ; i < contents().count(); i++)
		{
			if (contents()[i]->name() == c)
			{
				found = true;
				break;
			}
		}
		if (!found)
		{
			qDebug() << "This content does not exists for this session (" << c << ")";
			return;
		}
		JT_JingleAction *rAction = new JT_JingleAction(d->rootTask);
		connect(rAction, SIGNAL(acked()), this, SLOT(slotRemoveAcked()));
		rAction->setSession(this);
		d->contentsToRemove << c;
		rAction->removeContents(d->contentsToRemove);
	}
}

void JingleSession::slotRemoveAcked()
{
	JT_JingleAction *rAction = (JT_JingleAction*) sender();
	if (rAction != 0)
		delete rAction;
	else
		return;
	// Remove contents from the d->contents
	for (int i = 0; i < d->contentsToRemove.count(); i++)
	{
		for (int j = 0; j < contents().count(); j++)
		{
			if (d->contentsToRemove[i] == contents()[j]->name())
			{
				d->contents.removeAt(j);
				break;
			}
		}
	}
	d->contentsToRemove.clear();

	if (d->state == Pending)
		acceptSession(); //FIXME:Should be contentAccept...
	//else if (d->state == Active)
	//	emit stopSending(d->contentToRemove);
}

void JingleSession::ring()
{
	JT_JingleAction *jtRing = new JT_JingleAction(d->rootTask);
	jtRing->setSession(this);
	jtRing->ringing();
}

void JingleSession::setSid(const QString& s)
{
	d->sid = s;
}

QString JingleSession::sid() const
{
	return d->sid;
}

void JingleSession::setInitiator(const QString& init)
{
	d->initiator = init;
}

void JingleSession::addContent(const QDomElement& content)
{
	JingleContent *c = new JingleContent();
	c->fromElement(content);
	d->contents << c;
}

void JingleSession::terminate(const JingleReason& r)
{
/*
 * FIXME: Reason should be a class so we can add informations like the new
 * 	  sid for an alternative-session condition.
 */
	JT_JingleAction *tAction = new JT_JingleAction(d->rootTask);
	tAction->setSession(this);
	tAction->terminate(r);

	connect(tAction, SIGNAL(finished()), this, SLOT(slotSessTerminated()));
	/* TODO:
	 * 	tAction will send a signal when aknowledged by the recipient.
	 * 	it MUST be deleted at that moment.
	 */
}

QString JingleSession::initiator() const
{
	return d->initiator;
}

//TODO:maybe change the name of this function.
void JingleSession::startNegotiation()
{
	/* TODO:
	 * 	For each transport in each contents, I must send all possible candidates.
	 * 	Those candidates can be found without the help of the application.
	 */
	qDebug() << "Start Negotiation : ";
	for (int i = 0; i < d->contents.count(); i++)
	{
		if (d->contents[i]->transport().attribute("xmlns") == "urn:xmpp:tmp:jingle:transports:ice-udp")
		{
			qDebug() << "    ICE-UDP";
			sendIceUdpCandidates();
		}
		else if (d->contents[i]->transport().attribute("xmlns") == "urn:xmpp:tmp:jingle:transports:raw-udp")
		{
			qDebug() << d->contents[i]->name() << "    RAW-UDP";
			startRawUdpConnection(d->contents[i]);
		}
	}
}

JingleContent *JingleSession::contentWithName(const QString& n)
{
	qDebug() << "There are" << d->contents.count() << "contents";
	for (int i = 0; i < d->contents.count(); i++)
	{
		if (d->contents.at(i)->name() == n)
			return d->contents[i];
	}
	return 0;
}

void JingleSession::setTo(const Jid& to)
{
	d->to = to;
}

void JingleSession::sendIceUdpCandidates()
{
	qDebug() << "Sending ice-udp candidates (Not Implemented Yet)";
	/*JT_JingleAction *cAction = new JT_JingleAction(d->rootTask);
	cAction->setSession(this);
	QDomDocument doc("");
	QDomElement candidate = doc.createElement("candidate");
	candidate.setAttribute("foo", "bar");
	//cAction->sendCandidate(candidate);
	// --> Or better : sendTransportInfo(QDomElement transport);*/
}

void JingleSession::startRawUdpConnection(JingleContent *c)
{
	QDomElement e = c->transport();
	qDebug() << "Start raw-udp connection (still 'TODO') for content" << c->name();
	
	connect(c, SIGNAL(needData(XMPP::JingleContent*)), this, SIGNAL(needData(XMPP::JingleContent*)));
	//FIXME:This signal should not go trough JingleSession and be used directly with the JingleContent by the application.
	c->startSending();

	//Sending my own candidate:
	JT_JingleAction *cAction = new JT_JingleAction(d->rootTask);
	cAction->setSession(this);
	cAction->transportInfo(c);
	//TODO:after sending this, this content must be ready to receive data.
	
	//Sending "trying" stanzas
	JT_JingleAction *tAction = new JT_JingleAction(d->rootTask);
	tAction->setSession(this);
	tAction->trying(*c);
}

void JingleSession::slotSessTerminated()
{
	emit terminated();
}

void JingleSession::addSessionInfo(const QDomElement& e)
{
	QString info = e.tagName();
	if (info == "trying")
	{
		d->responderTrying = true;
	}
	else if (info == "received")
	{
		//stopTrying();
		//sessionAccept();
		//FIXME:For what Content do we receive that info ?
		//How do I know all content's connections are established, in both directions ?
		//What if it isn't the case ?
		//d->state = Active; not now, wait for the ack of session-accept.
	}
}

void JingleSession::addTransportInfo(const QDomElement& e)
{
	// this should really depend on the transport used...
	qDebug() << "Transport info for content named" << e.attribute("name");
	
	JingleContent *content = contentWithName(e.attribute("name"));
	
	qDebug() << "Found content with address" << (int) content;
	
	connect(content, SIGNAL(needData(XMPP::JingleContent*)), this, SIGNAL(needData(XMPP::JingleContent*)));
	content->addTransportInfo(e);
	
	//If it is a candidate, we try to connect.
	//FIXME:is a transport-info always a candidate ? --> Currently, we consider this can only a candidate.
	//TODO:There should be a JingleTransport Class as the transport will be used everywhere
	//	Also, we could manipulate the QDomElement
	QDomElement candidate = e.firstChildElement().firstChildElement(); //This is the candidate.
}

JingleSession::State JingleSession::state() const
{
	return d->state;
}


//--------------------------
// JingleReason
//--------------------------


class JingleReason::Private
{
public:
	QString reasonText;
	Type type;
};

JingleReason::JingleReason()
: d(new Private)
{
	d->reasonText = "";
	d->type = NoReason;
}

JingleReason::JingleReason(JingleReason::Type type, const QString& text)
: d(new Private)
{
	d->reasonText = text;
	d->type = type;
}

JingleReason::~JingleReason()
{

}

void JingleReason::setText(const QString& r)
{
	d->reasonText = r;
}

void JingleReason::setType(JingleReason::Type t)
{
	d->type = t;
}

QString JingleReason::text() const
{
	return d->reasonText;
}

JingleReason::Type JingleReason::type() const
{
	return d->type;
}

