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

#include <QString>
#include <QUdpSocket>

#include "jinglesession.h"
#include "jingleaction.h"
#include "jinglesessionmanager.h"
#include "jinglerawcontent.h"
#include "jingleicecontent.h"

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
	int contentsStarted;
	bool userAcceptedSession;
	bool contentsReady;
	bool canStart;
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
	d->contentsReady = false;
	d->contentsStarted = 0;
	d->canStart = false;
}

JingleSession::~JingleSession()
{
	for (int i = 0; i < d->contents.count(); i++)
	{
		delete d->contents[i];
	}
	delete d;
}

Task *JingleSession::rootTask() const
{
	return d->rootTask;
}

void JingleSession::addContent(JingleContent *c)
{
	qDebug() << "addContent" << c->name();
	d->contents << c;
	connect(c, SIGNAL(started()), this, SLOT(slotContentReady()));
	connect(c, SIGNAL(established()), this, SLOT(slotContentConnected())); //Only do that if we are not the initiator.
}

void JingleSession::addContents(const QList<JingleContent*>& l)
{
	qDebug() << "addContent : adding" << l.count() << "contents.";
	
	for (int i = 0; i < l.count(); i++)
	{
		d->contents << l[i];
		connect(l[i], SIGNAL(started()), this, SLOT(slotContentReady()));
		connect(l[i], SIGNAL(established()), this, SLOT(slotContentConnected()));
	}
}

Jid JingleSession::to() const
{
	return d->to;
}

QList<JingleContent*> JingleSession::contents() const
{
	qDebug() << "JingleSession::contents() returns a list of" << d->contents.count() << "contents.";
	return d->contents;
}

void JingleSession::slotContentReady()
{
	/*disconnect(sender(), SIGNAL(started()), this, SLOT(slotContentReady()));
	if (++d->contentsStarted == contents().count())
	{
		d->contentsReady = true;
		if (d->canStart)
			start();
	}*/
}

void JingleSession::start()
{
	/*d->canStart = true;
	if (!d->contentsReady)
		return;*/
	// Generate session ID
	d->sid = genSid();

	JT_JingleAction *iAction = new JT_JingleAction(d->rootTask);
	d->actions << iAction;
	iAction->setSession(this);
	connect(iAction, SIGNAL(finished()), this, SLOT(slotAcked()));
	iAction->initiate();
	iAction->go(true);
}

void JingleSession::slotContentConnected()
{
	qDebug() << "void JingleSession::slotContentConnected() called.";
	
	//No need for informations from this content anymore.
	disconnect(sender(), 0, this, 0);
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
		return;
	}
	else
		d->allContentsConnected = true;

	// If we are the initiator, we must not accept the session.
	if (isInitiator())
	{
		qDebug() << "We are the initiator, it's not our to accept the session.";
		return;
	}

	if (!d->userAcceptedSession)
	{
		qDebug() << "User did not accept the session yet.";
		return;
	}
	
	acceptSession();
}

bool JingleSession::isInitiator() const
{
	return initiator() == d->rootTask->client()->jid().full();
}

/*void JingleSession::sessionAccepted(const QDomElement& x)
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
}*/

void JingleSession::slotSessionAcceptAcked()
{
	d->state = Active;

	qDebug() << "Ok, we switched to ACTIVE state, starting to stream.";
	emit stateChanged();
}

void JingleSession::acceptSession()
{
	qDebug() << "JingleSession::acceptSession() : called";

	if (d->allContentsConnected)
	{
		//Accept session.
		qDebug() << "Ok, all contents connected and session accepted by the user ! let's accept the session.";
		
		for (int i = 0; i < contents().count(); i++)
		{
			qDebug() << "setting right information in the content" << contents()[i]->name();
			// First, set our supported payload-types.
			JingleContent *c = contents()[i];
			c->setLocalPayloads(c->type() == JingleContent::Audio ?
						d->jingleSessionManager->supportedAudioPayloads() :
						d->jingleSessionManager->supportedVideoPayloads());
		}
	
		qDebug() << "Sending session-accept action.";

		JT_JingleAction *sAction = new JT_JingleAction(d->rootTask);
		d->actions << sAction;
		sAction->setSession(this);
		connect(sAction, SIGNAL(finished()), this, SLOT(slotSessionAcceptAcked()));
		sAction->sessionAccept(contents());
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
	if (rAction == 0)
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
	qDebug() << "addContent" << content.attribute("name");

	// If the content is added from XML data, chances are it is a remote content.
	JingleContent *c;
	if (JingleContent::transportNS(content) == NS_JINGLE_TRANSPORTS_RAW)
		c = new JingleRawContent(JingleContent::Responder, this);
	else if (JingleContent::transportNS(content) == NS_JINGLE_TRANSPORTS_ICE)
		c = new JingleIceContent(JingleContent::Responder, this);
	else
	{
		qDebug() << "Unsupported Content. What do I do now ?";
		return;
	}
	
	connect(c, SIGNAL(started()), this, SLOT(slotContentReady()));
	connect(c, SIGNAL(established()), this, SLOT(slotContentConnected()));
	
	c->fromElement(content);
	d->contents << c;
}

void JingleSession::sessionTerminate(const JingleReason& r)
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

void JingleSession::slotSessTerminated()
{
	qDebug() << "JingleSession::slotSessTerminated() called !";
	qDebug() << "Emit terminated() signal";
	emit terminated();
}

void JingleSession::addSessionInfo(const QDomElement& e)
{
	//FIXME:
	//This is in a session info ? 
	//Well, this should be managed by the JingleContent too.
	//Anyway, protocol has changed, this does not exist anymore
	//	1) for Raw-Udp
	//	2) it's not a general use-case, maybe used for ICE-UDP only, but not even sure, too lazy to check now.
	//Uncomment if necessary
	/*QString info = e.tagName();
	if (info == "trying")
	{
		d->responderTrying = true;
	}
	else if (info == "received")

		// We consider every ports are opened, no firewall (that's the specification that tells us that.)
		for (int i = 0; i < contents().count(); i++)
		{
			//We tell the content that it is able to send data.
			contents()[i]->setSending(true);
		}
	}*/
}

/*void JingleSession::addTransportInfo(const QDomElement& e)
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
}*/

JingleSession::State JingleSession::state() const
{
	return d->state;
}

void JingleSession::appendAction(JingleAction *action)
{
	// TODO:Implement me.
	switch (action->action())
	{
	case JingleAction::SessionTerminate :
		qDebug() << "SessionTerminate";
		qDebug() << "The other peer wants to terminate the session" << action->sid();
		
		emit terminated();
		
		break;
	case JingleAction::TransportInfo :
	{
		qDebug() << "TransportInfo";
		if (action->data().tagName() != "content")
		{
			qDebug() << "bad transport-info action";
			return;
		}
		
		qDebug() << "Transport info for content named" << action->data().attribute("name");
		JingleContent *c = contentWithName(action->data().attribute("name"));
		if (c)
			c->addTransportInfo(action->data());
		else
			qDebug() << "No such content :" << action->data().attribute("name");
		
		break;
	}
	case JingleAction::SessionAccept :
	{
		qDebug() << "SessionAccept";
		QDomElement content = action->data();

		while (!content.isNull())
		{
			JingleContent *c = contentWithName(content.attribute("name"));
			QList<QDomElement> payloads;
			QDomElement pType = content.firstChildElement().firstChildElement(); //FIXME:some checks would be nice...
			//		    content    description         payload-type
			
			while (!pType.isNull())
			{
				payloads << pType;
				pType = pType.nextSiblingElement();
			}
			
			c->setRemotePayloads(payloads);
			content = content.nextSiblingElement();
		}
		d->state = Active;

		qDebug() << "Ok, we switched to ACTIVE state, starting to stream.";

		emit stateChanged();

		break;
	}
	case JingleAction::ContentRemove :
	case JingleAction::SessionInfo :
	case JingleAction::ContentAdd :
	case JingleAction::ContentModify :
	case JingleAction::TransportReplace :
	case JingleAction::TransportAccept :
	default :
		qDebug() << "JingleSession::appendAction() : Not implemented yet !";
		//TODO:must send "not implemented" stanza.
	}
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
