/*
 * jinglesession.cpp - Jingle session
 * Copyright (C) 2009 - Detlev Casanova <detlev.casanova@gmail.com>
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
#include "jinglecontent.h"

using namespace XMPP;

static QString genSid()
{
	QString s;
	//int id_seed = rand() % 0xffff;
	s.sprintf("a%x%x", rand() % 0xffff, rand() % 0xffff);
	return s;
}

class JingleSession::Private
{
public:
	Private() : isInitiator(false),
		    state(Pending)
	{}

	Jid to;
	QList<JingleContent*> contents;
	QList<JingleContent*> contentsToRemove;
	QList<JingleContent*> pendingContentsToRemove;
	Task *rootTask;
	QString sid;
	QString initiator;
	QString responder;
	
	bool isInitiator;
	State state;
};

JingleSession::JingleSession(Task *t, const Jid &j, JingleSessionManager *parent)
: d(new Private)
{
	d->to = j;
	d->rootTask = t;
}

JingleSession::~JingleSession()
{
	foreach (JingleContent *content, d->contents)
	{
		delete content;
	}

	delete d;
}

Task *JingleSession::rootTask() const
{
	return d->rootTask;
}

void JingleSession::addContent(JingleContent *c)
{
	c->setCreator(isInitiator() ? "initiator" : "responder");
	c->setParent(this);
	d->contents << c;
	//connect(c, SIGNAL(established()), this, SLOT(slotContentConnected()));
}

void JingleSession::addContents(const QList<JingleContent*>& l)
{
	foreach (JingleContent *content, l)
	{
		content->setCreator(isInitiator() ? "initiator" : "responder");
		content->setParent(this);
		d->contents << content;
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

bool JingleSession::isStarted() const
{
	return !d->sid.isNull();
}

void JingleSession::start()
{
	// Generate session ID
	d->sid = genSid();
	
	foreach(JingleContent *c, d->contents)
	{
		c->start();
	}

	JT_JingleAction *iAction = new JT_JingleAction(d->rootTask);
	iAction->setSession(this);
	connect(iAction, SIGNAL(finished()), this, SLOT(slotInitiateAcked()));
	iAction->initiate();
	iAction->go(true);
}

void JingleSession::slotInitiateAcked()
{
	JT_JingleAction *action = dynamic_cast<JT_JingleAction*>(sender());
	if (!action)
		return;

	if (!action->success())
	{
		emit error(action->statusCode(), action->statusString());
	}

}

void JingleSession::slotContentConnected()
{
	//FIXME:Actually, when it is connected, the application can start streaming.
	//	Also, this can happen on each content individually.
	//	The signal is to be catched by the application and not here.
	
	//No need for information from this content anymore.
/*	disconnect(sender(), 0, this, 0);
	bool allOk = true;
	
	// Checking if all contents are connected.
	foreach (JingleContent *content, d->contents)
	{
		if (!content->isReady())
		{
			allOk = false;
			break;
		}
	}
	
	if (allOk)
		d->allContentsConnected = true;
	else
		return;
*/
}

bool JingleSession::isInitiator() const
{
	return d->isInitiator;
}

void JingleSession::slotSessionAcceptAcked()
{
	d->state = Active;
	emit stateChanged();
}

void JingleSession::acceptSession()
{
	//Check if contents are being removed (don't leave them in session-accept)
	QList<JingleContent*> acceptContents;
	foreach (JingleContent *c, contents())
	{
		bool remove = false;
		
		foreach (JingleContent *r, d->pendingContentsToRemove)
		{
			if (c == r)
			{
				remove = true;
				break;
			}
		}

		if (!remove)
			acceptContents << c;
	}
	//Accept session.
	JT_JingleAction *sAction = new JT_JingleAction(d->rootTask);
	sAction->setSession(this);
	connect(sAction, SIGNAL(finished()), this, SLOT(slotSessionAcceptAcked()));
	sAction->sessionAccept(acceptContents);
	sAction->go(true);
}

void JingleSession::acceptContent()
{
	//TODO:Implement me !
}

void JingleSession::removeContent(QList<JingleContent*>& c)
{	
	if (d->contentsToRemove.count() != 0)
	{
		d->pendingContentsToRemove << c;
		return;
	}

	// Removing only existing contents.
	foreach (JingleContent *content, c)
	{
		for (int i = 0; i < d->contents.count(); i++)
		{
			if (content == d->contents[i])
			{
				content->stopNegotiation();
				d->contentsToRemove << d->contents.takeAt(i--);
			}
		}
	}

	if (d->contentsToRemove.count() == 0)
		return;

	//TODO:We don't manage that here, the responder will have to terminate the session itself.
	/*if (state() == Active && d->contents.count() == 0)
	{
		qDebug() << "Session terminate because there is no more contents here.";
		//Session terminate instead.
		//Reason will be set when konqueror can show me the jingle spec. BLOCKER
		sessionTerminate(QDomElement());
		return;
	}*/
	
	JT_JingleAction *rAction = new JT_JingleAction(d->rootTask);
	rAction->setSession(this);
	connect(rAction, SIGNAL(finished()), this, SLOT(slotRemoveAcked()));
	rAction->removeContents(d->contentsToRemove);
	rAction->go(true);
}

void JingleSession::removeContent(JingleContent* c) // Provided for convenience.
{
	removeContent(QList<JingleContent*>() << c);
}

void JingleSession::slotRemoveAcked()
{
	// Remove contents from the d->contents list.
	foreach (JingleContent *content, d->contentsToRemove)
	{
		delete content;
	}

	d->contentsToRemove.clear();
	
	if (d->pendingContentsToRemove.count() != 0)
		removeContent(d->pendingContentsToRemove);
}

void JingleSession::setSid(const QString& s)
{
	d->sid = s;
}

QString JingleSession::sid() const
{
	return d->sid;
}

void JingleSession::setInitiator(const QString& init, bool isInit)
{
	d->initiator = init;
	d->isInitiator = isInit;
}

void JingleSession::setResponder(const QString& resp, bool isResp)
{
	d->responder = resp;
	d->isInitiator = !isResp;
}

void JingleSession::addContent(const QDomElement& content)
{
	JingleContent *c = new JingleContent(this);

	c->setSupportedApplications(JingleSessionManager::manager()->supportedApplications());

	//connect(c, SIGNAL(started()), this, SLOT(slotContentReady()));
	//connect(c, SIGNAL(established()), this, SLOT(slotContentConnected()));
	
	d->contents << c;
	
	c->fromElement(content);
}

void JingleSession::sessionTerminate(const QDomElement& r)
{
	JT_JingleAction *tAction = new JT_JingleAction(d->rootTask);
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
	foreach (JingleContent *content, d->contents)
	{
		if (content->name() == n)
			return content;
	}

	return 0;
}

void JingleSession::setTo(const Jid& to)
{
	d->to = to;
}

void JingleSession::slotSessTerminated()
{
	emit terminated();
}

JingleSession::State JingleSession::state() const
{
	return d->state;
}

void JingleSession::appendAction(JingleAction *action)
{
	switch (action->action())
	{
	case JingleAction::SessionTerminate :
		qDebug() << "SessionTerminate.\nThe other peer wants to terminate the session" << action->sid();
		
		emit terminated();
		break;
	case JingleAction::TransportInfo :
	{
		qDebug() << "TransportInfo";
		if (action->data().firstChildElement().tagName() != "content")
		{
			qDebug() << "bad transport-info action";
			return;
		}
		
		JingleContent *c = contentWithName(action->data().firstChildElement().attribute("name"));
		if (c)
			c->addTransportInfo(action->data().firstChildElement());
		else
		{
			qDebug() << "No such content :" << action->data().firstChildElement().attribute("name");
		}
		
		break;
	}
	case JingleAction::SessionAccept :
	{
		qDebug() << "SessionAccept";
		QDomElement content = action->data().firstChildElement();

		while (!content.isNull())
		{
			//Adding application.
			JingleContent *c = contentWithName(content.attribute("name"));
			
			QDomElement desc = content.firstChildElement();
			QDomElement pApp;
			while (!desc.isNull())
			{
				if (desc.tagName() == "description")
				{
					pApp = desc;
					break;
				}

				desc = desc.nextSiblingElement();
			}
			
			if (pApp.isNull())
			{
				sessionTerminate();//FIXME:Precise the reason (e.g. no application)
				return;
			}

			JingleApplication *app = JingleApplication::createFromXml(pApp, c);
			if (app != 0)
			{
				c->setApplication(app);
			}
			else
			{
				// Coming here would be weird though, Just ignoring for now.
				qDebug() << "Application not supported";
			}
			
			content = content.nextSiblingElement();
		}

		d->state = Active;

		emit stateChanged();
		break;
	}
	case JingleAction::SessionInfo :
	{
		QDomElement info = action->data().firstChildElement();
		/*
		 * Currently, no session-info action is to be managed by the
		 * session itself. Passing it to contents.
		 */

		foreach (JingleContent *c, d->contents)
		{
			c->sessionInfo(info);
		}
		break;
	}
	case JingleAction::ContentRemove :
	{
		QDomElement content = action->data().firstChildElement();
		while (!content.isNull())
		{
			QString name = content.attribute("name");

			for (int j = 0; j < contents().count(); j++)
			{
				if (name == contents()[j]->name())
				{
					//FIXME:should it be the role of the application to delete it ?
					//emit contentRemoved(d->contents.takeAt(j--));
					delete d->contents.takeAt(j--);
					break;
				}
			}

			content = content.nextSiblingElement();
		}
		break;
	}
	case JingleAction::ContentModify :
	{
		if (action->data().firstChildElement().tagName() != "content")
			break;

		QDomElement content = action->data().firstChildElement();

		JingleContent *c = contentWithName(content.attribute("name"));
		
		if (!c)
			break;

		c->setSenders(JingleContent::strToSenders(content.attribute("senders")));
	}
	case JingleAction::ContentAdd :
	case JingleAction::TransportReplace :
	case JingleAction::TransportAccept :
	default :
		qDebug() << "JingleSession::appendAction() : Not implemented yet !";
		//TODO:must send "not implemented" stanza.
		//TODO:should actually be implemented...
	}
}

QString JingleSession::responder() const
{
	return d->responder;
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
