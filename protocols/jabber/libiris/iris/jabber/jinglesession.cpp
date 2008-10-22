/*
 * jinglesession.cpp - Jingle session
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

//FIXME:Must remove all JT_JingleAction as soon as they are acknowledged !!!!!!!
//	This bug makes kopete crash when a session is trminated and other sessions
//	are active because JT_JingleAction keeps a pointer on the JingleSession and
//	use it in the take() method

#include <QString>
#include <QUdpSocket>

#include "jinglesession.h"
#include "jinglesessionmanager.h"

//TODO:all JT_JingleActions pointers should be kept in a list so we can delete all non-acknowledged
//     actions when terminating the session.

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
	JingleSessionManager *jingleSessionManager;
	QString sid;
	QString initiator;
	State state;
	QStringList contentsToRemove;
	QStringList transports;
	bool responderTrying;
	QList<JT_JingleAction*> actions;
	bool allContentsConnected;
	bool userAcceptedSession;
};

JingleSession::JingleSession(Task *t, const Jid &j)
: d(new Private)
{
	qDebug() << "Creating XMPP::JingleSession";
	d->to = j;
	d->rootTask = t;
	d->jingleSessionManager = t->client()->jingleSessionManager();
	d->state = Pending;
	d->responderTrying = false;
	d->allContentsConnected = false;
	d->userAcceptedSession = false;
}

JingleSession::~JingleSession()
{
	delete d;
}

void JingleSession::addContent(JingleContent *c)
{
	d->contents << c;
	connect(c, SIGNAL(dataReceived()), this, SLOT(slotReceivingData()));
	if (initiator() != d->rootTask->client()->jid().full())
		connect(c, SIGNAL(established()), this, SLOT(slotContentConnected())); //Only do that if we are not the initiator.
}

void JingleSession::addContents(const QList<JingleContent*>& l)
{
	for (int i = 0; i < l.count(); i++)
	{
		d->contents << l[i];
		connect(l[i], SIGNAL(dataReceived()), this, SLOT(slotReceivingData()));
		if (initiator() != d->rootTask->client()->jid().full())
			connect(l[i], SIGNAL(established()), this, SLOT(slotContentConnected()));
	}
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
	d->actions << iAction;
	iAction->setSession(this);
	connect(iAction, SIGNAL(finished()), this, SLOT(slotAcked()));
	iAction->initiate();
	iAction->go(true);
	/*for (int i = 0; i < contents().count(); i++)
	{
		if (contents()[i]->transport().attribute("xmlns") == NS_JINGLE_TRANSPORTS_RAW)
		{
			qDebug() << "Create IN socket for" << contents()[i]->name();
			//qDebug("Content Adress : %x\n", (unsigned int) contents()[i]);
			//contents()[i]->createUdpInSocket(); --> should be done by the JT_JingleAction::initiate().
		}
	}*/
}

void JingleSession::slotAcked()
{
	//Should be autamtically deleted by Iris.
	/*if (!sender())
	{
		qDebug() << "Sender is NULL !";
		return;
	}
	deleteAction(static_cast<JT_JingleAction*>(sender()));*/
}

void JingleSession::deleteAction(JT_JingleAction* a)
{
	//Don't delete tasks, iris will take care of it.
	/*for (int i = 0; i < d->actions.count(); i++)
	{
		if (d->actions[i] == a)
		{
			delete d->actions.takeAt(i);
			break;
		}
	}*/
}

void JingleSession::slotContentConnected()
{
	qDebug() << "---------------- void JingleSession::slotContentConnected() : called";
	bool allOk = true;
	// Checking if all contents are connected.
	for (int i = 0; i < contents().count(); i++)
	{
		if (!contents()[i]->sending() || !contents()[i]->receiving())
		{
			allOk = false;
			break;
		}
	}
	
	if (!allOk)
	{
		qDebug() << "Not All ok !!! --> Not switching to ACTIVE state.";
		disconnect(sender(), 0, this, 0);
		return;
	}
	else
		d->allContentsConnected = true;
	
	if (!d->userAcceptedSession)
	{
		qDebug() << "User did not accept the session yet.";
		disconnect(sender(), 0, this, 0);
		return;
	}
	
	/*qDebug() << initiator() << "=?=" << d->rootTask->client()->jid().full();
	if (initiator() == d->rootTask->client()->jid().full())
	{
		// In this case, we must not send session-accept and wait for it.
		qDebug() << "I'm the initiator, it's not me who must accept the session.";
		disconnect(sender(), 0, this, 0);
		return;
	}*/

	acceptSession();

	disconnect(sender(), 0, this, 0);
}

void JingleSession::sessionAccepted(const QDomElement& x)
{
	qDebug() << "void JingleSession::sessionAccepted(const QDomElement& x) called";
	QDomElement content = x.firstChildElement();
	
	while (!content.isNull())
	{
		JingleContent *c = contentWithName(content.attribute("name"));
		QList<QDomElement> payloads;
		QDomElement pType = content.firstChildElement().firstChildElement();
		//		    content    description         payload-type
		while (!pType.isNull())
		{
			payloads << pType;
			pType = pType.nextSiblingElement();
		}
		c->setResponderPayloads(payloads);
		qDebug() << "Best payload name :" << c->bestPayload().attribute("name");
		content = content.nextSiblingElement();
	}
	d->state = Active;

	qDebug() << "Ok, we switched to ACTIVE state, starting to stream.";
	
	emit stateChanged();
}

void JingleSession::slotSessionAcceptAcked()
{
	d->state = Active;

	if (sender())
		deleteAction(static_cast<JT_JingleAction*>(sender()));
	qDebug() << "Ok, we switched to ACTIVE state, starting to stream.";
	emit stateChanged();
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
	//connect(rAction, SIGNAL(finished()), this, SLOT(slotReceivedAcked()));
	//rAction->setSession(this);
	//rAction->received();
*/
}

void JingleSession::acceptSession()
{
	qDebug() << "JingleSession::acceptSession() : called";

	if (d->allContentsConnected)
	{
		//Accept session.
		qDebug() << "Ok, all contents connected and session accepted by the user ! let's accept the session.";
		// Create all contents to send in the session-accept.
		QList<JingleContent*> contentList;
		for (int i = 0; i < contents().count(); i++)
		{
			qDebug() << "setting right informations in the content" << contents()[i]->name();
			// First, set our supported payload-types.
			JingleContent *c = new JingleContent();
			c->setType(contents()[i]->type());
			c->setPayloadTypes(c->type() == JingleContent::Audio ?
						d->jingleSessionManager->supportedAudioPayloads() :
						d->jingleSessionManager->supportedVideoPayloads());
			c->setDescriptionNS(contents()[i]->descriptionNS());
			c->setName(contents()[i]->name());
			c->setCreator(contents()[i]->creator());
			c->setTransport(contents()[i]->transport().cloneNode(false).toElement()); //We don't need the child nodes.
			contentList << c;
	
			// Then, set the corresponding candidate for ICE-UDP (let's see later)
			// TODO:implement me !
		}
	
		qDebug() << "Sending session-accept action.";
		JT_JingleAction *sAction = new JT_JingleAction(d->rootTask);
		d->actions << sAction;
		sAction->setSession(this);
		connect(sAction, SIGNAL(finished()), this, SLOT(slotSessionAcceptAcked()));
		sAction->sessionAccept(contentList);
		sAction->go(true);
	}
	else
	{
		qDebug() << "d->allContentsConnected is FALSE !!!";
	}

	d->userAcceptedSession = true;
}

void JingleSession::acceptContent()
{
	//TODO:Implement me !
	//JT_JingleAction *tAction = new JT_JingleAction(d->rootTask);
	//tAction->setSession(this);
	//tAction->contentAccept();
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
	d->actions << rAction;
	rAction->setSession(this);
	connect(rAction, SIGNAL(finished()), this, SLOT(slotRemoveAcked()));
	rAction->removeContents(d->contentsToRemove);
	rAction->go(true);
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
		d->actions << rAction;
		connect(rAction, SIGNAL(finished()), this, SLOT(slotRemoveAcked()));
		rAction->setSession(this);
		d->contentsToRemove << c;
		rAction->removeContents(d->contentsToRemove);
		rAction->go(true);
	}
}

void JingleSession::slotRemoveAcked()
{
	JT_JingleAction *rAction = (JT_JingleAction*) sender();
	if (rAction != 0)
		deleteAction(static_cast<JT_JingleAction*>(sender()));
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

	//else if (d->state == Active)
	//	emit stopSending(d->contentsToRemove);
}

void JingleSession::ring()
{
	JT_JingleAction *rAction = new JT_JingleAction(d->rootTask);
	d->actions << rAction;
	connect(rAction, SIGNAL(finished()), this, SLOT(slotAcked()));
	rAction->setSession(this);
	rAction->ringing();
	rAction->go(true);
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
	
	if (initiator() != d->rootTask->client()->jid().full())
		connect(c, SIGNAL(established()), this, SLOT(slotContentConnected()));
	connect(c, SIGNAL(dataReceived()), this, SLOT(slotReceivingData()));
}

void JingleSession::terminate(const JingleReason& r)
{
//FIXME:should Take an QDomElement as argument, the application should implement this
//class itself and be able to return the right QDomElement when calling this method
	JT_JingleAction *tAction = new JT_JingleAction(d->rootTask);
	d->actions << tAction;
	tAction->setSession(this);
	connect(tAction, SIGNAL(finished()), this, SLOT(slotSessTerminated()));
	tAction->terminate(r);
	tAction->go(true);
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
		if (d->contents[i]->transport().attribute("xmlns") == NS_JINGLE_TRANSPORTS_ICE)
		{
			qDebug() << "    ICE-UDP";
			sendIceUdpCandidates();
		}
		else if (d->contents[i]->transport().attribute("xmlns") == NS_JINGLE_TRANSPORTS_RAW)
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
	d->actions << cAction;
	connect(cAction, SIGNAL(finished()), this, SLOT(slotAcked()));
	cAction->setSession(this);
	cAction->transportInfo(c);
	cAction->go(true);
}

void JingleSession::slotSessTerminated()
{
	qDebug() << "JingleSession::slotSessTerminated() called !";
	if (sender())
		deleteAction(static_cast<JT_JingleAction*>(sender()));
	qDebug() << "Emit terminated() signal";
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
		//FIXME:For what Content do we receive that info ?
		//How do I know all content's connections are established, in both directions ?
		//What if it isn't the case ?

		// We consider every ports are opened, no firewall (that's the specification that tells us that.)
		for (int i = 0; i < contents().count(); i++)
		{
			//We tell the content that it is able to send data.
			contents()[i]->setSending(true);
		}
	}
}

void JingleSession::addTransportInfo(const QDomElement& e)
{
	// this should really depend on the transport used...
	qDebug() << "Transport info for content named" << e.attribute("name");
	
	JingleContent *content = contentWithName(e.attribute("name"));
	
	qDebug() << "Found content with address" << (int*) content;
	
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

void JingleSession::slotReceivingData()
{
	// Whatever the sender content is, we send the same received informational message.
	// That's from the raw-udp specification, later, we will have to do fifferent things
	// here depending on the transport method used in the sender content.
	
	JT_JingleAction *rAction = new JT_JingleAction(d->rootTask);
	d->actions << rAction;
	connect(rAction, SIGNAL(finished()), this, SLOT(slotAcked()));
	rAction->setSession(this);
	rAction->received();
	rAction->go(true);
}

