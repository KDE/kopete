/*
 * jinglesessionmanager.cpp - Manager for Jingle sessions
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

#include "jinglesessionmanager.h"
#include "jingletasks.h"
#include "jingleaction.h"
#include "jingleapplication.h"
#include "jingletransport.h"

#include <QHttp>

using namespace XMPP;

class JingleSessionManager::Private
{
public:
	Private() :
		    firstPort(9000),
		    lastUsed(-1),
		    stunPort(-1),
		    uniqueSession(false)
	{}

	JT_PushJingleAction *pjs;
	Client *client;
	QList<JingleSession*> sessions;
	QStringList supportedTransports;
	QList<JingleApplication*> supportedApplications;
	
	int firstPort;
	int lastUsed;
	
	QHostAddress stunAddr;
	int stunPort;
	
	QHostAddress localAddr;

	bool uniqueSession; //Should we use a unique session for each contact (send the alternate session error).
};

JingleSessionManager *JingleSessionManager::self = 0;

JingleSessionManager::JingleSessionManager(Client* c)
: d(new Private)
{
	qDebug() << "JingleSessionManager::JingleSessionManager created.";
	d->client = c;

	d->pjs = new JT_PushJingleAction(d->client->rootTask());

	d->supportedTransports << NS_JINGLE_TRANSPORTS_ICE;

	connect(d->pjs, SIGNAL(jingleActionReady()), this, SLOT(slotJingleActionReady()));

	connect(c, SIGNAL(disconnected()), SLOT(cleanup()));

	Features f = d->client->features();

	f.addFeature(NS_JINGLE);
	f.addFeature(NS_JINGLE_APPS_RTP);
	f.addFeature(NS_JINGLE_TRANSPORTS_ICE);

	d->client->setFeatures(f);

	d->firstPort = 9000;

	d->stunPort = -1;

	self = this;
}

JingleSessionManager::~JingleSessionManager()
{
	self = 0;

	delete d->pjs;
	
	foreach(JingleSession* sess, d->sessions)
	{
		delete sess;
	}
	
	d->sessions.clear();
	
	/*
	foreach(JingleApplication* app, d->supportedApplications)
	{
		delete app;
	}
	That's the application's job.
	*/
	
	delete d;
}

void JingleSessionManager::cleanup()
{
	foreach(JingleSession* sess, d->sessions)
	{
		delete sess;
	}
	
	d->sessions.clear();
}

/*void JingleSessionManager::setSupportedFeatures(Features features)
{
	d->features = features;
	
	Features f = d->client->features();
	
	if (features & RTP)
		f.addFeature(NS_JINGLE_APPS_RTP);
		
	if (features & FileTransfer)
		f.addFeature(NS_JINGLE_APPS_FT);
	
	d->client->setFeatures(f);
}*/

JingleSessionManager *JingleSessionManager::manager()
{
        return self;
}

void JingleSessionManager::setUniqueSession(bool u)
{
	d->uniqueSession = u;
}

void JingleSessionManager::slotJingleActionReady()
{
	JingleAction *action = 0;
	if (d->pjs->hasPendingAction())
		action = d->pjs->takeNextPendingAction();
	else
	{
		qDebug() << "JingleSessionManager::slotJingleActionReady() : There is no pending action, why am I called ?";
		return;
	}
	
	switch(action->action())
	{
	case JingleAction::SessionInitiate :
	{
		qDebug() << "New Incoming session : " << action->sid();

		// Acknowledges the jingle action stanza.
		d->pjs->ack(action->id(), action->from());

		// Prepare the new JingleSession instance.
		JingleSession *incomingSession = new JingleSession(d->pjs->parent(), action->from(), this);
		connect(incomingSession, SIGNAL(terminated()), this, SLOT(slotRemoveSession()));
		connect(incomingSession, SIGNAL(destroyed()), this, SLOT(slotRemoveSession()));

		incomingSession->setInitiator(action->initiator(), false);
		incomingSession->setResponder(action->to().full(), action->to().full() == d->client->jid().full());
		incomingSession->setSid(action->sid());

		// Check if we should send an alternate session error.
		// FIXME:Should be done only if contents are identical.
		// 	 For example, A file transfer should not be added to an RTP session.
		if (d->uniqueSession)
		{
			foreach (JingleSession *sess, d->sessions)
			{
				if (sess->to() == action->from())
				{
					QDomDocument doc("");
					QDomElement reason = doc.createElement("reason");
					QDomElement alt = doc.createElement("alternative-session");
					QDomElement sid = doc.createElement("sid");
					QDomText txt = doc.createTextNode(sess->sid());
					sid.appendChild(txt);
					alt.appendChild(sid);
					reason.appendChild(alt);

					incomingSession->sessionTerminate(reason);

					return;
				}
			}
		}

		QDomElement content = action->data().firstChildElement();
		while (!content.isNull())
		{
			if (content.tagName() == "content")
				incomingSession->addContent(content);

			content = content.nextSiblingElement();
		}

		d->sessions << incomingSession;

		emit newJingleSession(incomingSession);

		break;
	}
	case JingleAction::ContentRemove : 
	case JingleAction::SessionInfo :
	case JingleAction::TransportInfo :
	case JingleAction::SessionTerminate :
	case JingleAction::SessionAccept :
	case JingleAction::ContentAdd :
	case JingleAction::ContentModify :
	case JingleAction::TransportReplace :
	case JingleAction::TransportAccept :
	default :
	{
		JingleSession *sess = session(action->sid());
		if (sess)
		{
			// We can acknowledge the stanza now.
			d->pjs->ack(action->id(), action->from());
			sess->appendAction(action);
			
			//if (sess->appendAction(action))
			//	d->pjs->ack(action->id(), action->from());
			//else
			//	Unknown session...
		}
		else
		{
			// Unknown Session.
			qDebug() << "Session" << action->sid() << "not found.";
			d->pjs->unknownSession(action->id(), action->from());
		}
	}
	}
}

void JingleSessionManager::slotRemoveSession()
{
	qDebug() << "Removing session from session manager.";
	if (sender())
	{
		for (int i = 0; i < d->sessions.count(); ++i)
		{
			if (sender() == d->sessions[i])
			{
				d->sessions.removeAt(i);
				break;
			}
		}
	}
}

void JingleSessionManager::setSupportedApplications(QList<JingleApplication*>& sa)
{
	d->supportedApplications = sa;
}

QList<JingleApplication*> JingleSessionManager::supportedApplications() const
{
	return d->supportedApplications;
}

JingleSession *JingleSessionManager::createNewSession(const Jid& toJid)
{
	XMPP::JingleSession *session = new XMPP::JingleSession(d->client->rootTask(), toJid.full(), this);

	session->setInitiator(d->client->jid().full(), true);
	d->sessions << session;
	
	connect(session, SIGNAL(terminated()), this, SLOT(slotRemoveSession()));
	connect(session, SIGNAL(destroyed()), this, SLOT(slotRemoveSession()));
	
	return session;
}

JingleSession *JingleSessionManager::session(const QString& sid)
{
	JingleSession *sess;
	sess = 0;
	for (int i = 0; i < d->sessions.count(); i++)
	{
		if (d->sessions.at(i)->sid() == sid)
		{
			sess = d->sessions.at(i);
			break;
		}
	}
	return sess;
}


/**
 * Returns the next available UDP base port.
 * The argument n is the number of ports that will be used with the returned base port.
 */
int JingleSessionManager::nextUdpPort(int n)
{
	if (d->lastUsed == -1)
		d->lastUsed = d->firstPort - 1;

	int ret = d->lastUsed + 1;
	d->lastUsed += n;
	
	return ret;
}

void JingleSessionManager::setBasePort(int f)
{
	d->firstPort = f;
}

void JingleSessionManager::setStunServiceAddress(const QHostAddress& addr, int port)
{
	qDebug() << "Set Stun address and port";
	d->stunAddr = addr;
	d->stunPort = port;
}

int JingleSessionManager::stunPort() const
{
	return d->stunPort;
}

QHostAddress JingleSessionManager::stunAddress() const
{
	return d->stunAddr;
}

void JingleSessionManager::setSelfAddress(const QHostAddress& addr)
{
	d->localAddr = addr;
}

QHostAddress JingleSessionManager::selfAddr() const
{
	return d->localAddr;
}
