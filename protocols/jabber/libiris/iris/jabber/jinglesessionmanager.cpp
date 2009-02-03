/*
 * jinglesessionmanager.cpp - Manager for Jingle sessions
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

#include "jinglesessionmanager.h"
#include "jingletasks.h"
#include "jingleaction.h"

#include <QHttp>

using namespace XMPP;

class JingleSessionManager::Private
{
public:
	JT_PushJingleAction *pjs;
	Client *client;
	QList<JingleSession*> sessions;
	QStringList supportedTransports;
	QList<QDomElement> supportedAudioPayloads;
	QList<QDomElement> supportedVideoPayloads;
	QStringList supportedProfiles;
	QList<int> usedPorts;
	int firstPort;
	QString ip;
	QHttp *http;
};

JingleSessionManager::JingleSessionManager(Client* c)
: d(new Private)
{
	qDebug() << "JingleSessionManager::JingleSessionManager created.";
	d->client = c;
	d->pjs = new JT_PushJingleAction(d->client->rootTask());
	connect(d->pjs, SIGNAL(jingleActionReady()),
		this, SLOT(slotJingleActionReady()));
	
	Features f = d->client->features();
	
	f.addFeature(NS_JINGLE);
//	f.addFeature(NS_JINGLE_TRANSPORTS_ICE); //Not supported yet
	f.addFeature(NS_JINGLE_TRANSPORTS_RAW);
	f.addFeature(NS_JINGLE_APPS_RTP);
//	f.addFeature("urn:xmpp:tmp:jingle:apps:video-rtp"); //Not supported yet

	d->client->setFeatures(f);

	d->firstPort = 9000;
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

		//Prepare the JingleSession instance.
		JingleSession *incomingSession = new JingleSession(d->pjs->parent(), action->from());
		connect(incomingSession, SIGNAL(destroyed()), this, SLOT(slotRemoveSession()));

		incomingSession->setInitiator(action->initiator());
		incomingSession->setSid(action->sid());
		QDomElement content = action->data();
		while (!content.isNull())
		{
			if (content.tagName() == "content")
				incomingSession->addContent(content);
			content = content.nextSiblingElement();
		}

		d->sessions << incomingSession;

		connect(incomingSession, SIGNAL(terminated()), this, SLOT(slotSessionTerminated()));
		//QList<QString> incompatibleContents;
		
		QList<QString> unsupportedPayloads;
		// This is a list of the names of the contents which have no supported payloads.
		
		QList<QString> unsupportedTransports;
		// This is a list of the names of the contents which have no supported transports
		// We have to remove all contents present in those lists.
		//
		// If no content is supported, reject the session because it's not possible to establish a session.
	
		for (int i = 0; i < incomingSession->contents().count(); i++)
		{
			JingleContent *c = incomingSession->contents()[i];
			
			//Set supported payloads for this content.
			c->setPayloadTypes(c->type() == JingleContent::Audio ? d->supportedAudioPayloads : d->supportedVideoPayloads);
	
			// Check payloads for the content c
			if (!checkSupportedPayloads(c))
			{
				//incompatibleContents << c->name();
				unsupportedPayloads << c->name();
				continue;
			}
			
			if (!checkSupportedTransport(c))
			{
				//incompatibleContents << c->name();
				unsupportedTransports << c->name();
			}
		}
		
		if (unsupportedPayloads.count() + unsupportedTransports.count() == incomingSession->contents().count())
		{
			//Reject the session.
			JingleReason r(JingleReason::UnsupportedApplications);
			incomingSession->sessionTerminate(r);
			//TODO:What happens when we receive the ack of the session-terminate ?
			//     Session must be removed and deleted.
			return;
		}
		else if (unsupportedPayloads.count() + unsupportedTransports.count() > 0)
		{
			//remove this contents list
			incomingSession->removeContent(unsupportedPayloads + unsupportedTransports);
			return;
		}
		
		emit newJingleSession(incomingSession);
		
		d->sessions.last()->ring();
		
		d->sessions.last()->startNegotiation(); //FIXME:Why is that ? negotiation will automatically start when candidates are received.

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
		session(action->sid())->appendAction(action);
/*	
	case JingleAction::ContentRemove : 
		qDebug() << "Content remove for session " << sid;
		
		content = x.firstChildElement().firstChildElement();
		while (!content.isNull())
		{
			cName << content.attribute("name");
			qDebug() << " * Remove : " << cName;
			content = content.nextSiblingElement();
		}
		emit removeContent(sid, cName);
		break;
	case JingleAction::SessionInfo :
		qDebug() << "Session Info for session " << sid;
		
		emit sessionInfo(x.firstChildElement());
		break;
	case JingleAction::TransportInfo :
		qDebug() << "Transport Info for session " << sid;
		
		emit transportInfo(x.firstChildElement());

		break;
	case JingleAction::SessionTerminate :
		qDebug() << "Transport Info for session " << sid;
		
		reason = x.firstChildElement().firstChildElement();
		e = reason.firstChildElement();
		while(!e.isNull())
		{
			if (e.tagName() == "condition")
				condition = e.firstChildElement().tagName();
			else if (e.tagName() == "text")
				text = e.firstChildElement().toText().data();

			e = e.nextSiblingElement();
		}
		
		emit sessionTerminate(sid, JingleReason(stringToType(condition), text));

		break;
	case JingleAction::SessionAccept :
		qDebug() << "Transport Info for session " << sid;

		emit sessionAccepted(x.firstChildElement());
		break;*/
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
				d->sessions.removeAt(i);
		}
	}
}

JingleSessionManager::~JingleSessionManager()
{
	delete d;
}

void JingleSessionManager::setSupportedTransports(const QStringList& transports)
{
	d->supportedTransports = transports;
}

void JingleSessionManager::setSupportedAudioPayloads(const QList<QDomElement>& payloads)
{
	d->supportedAudioPayloads = payloads;
}

QList<QDomElement> JingleSessionManager::supportedAudioPayloads() const
{
	return d->supportedAudioPayloads;
}

void JingleSessionManager::setSupportedVideoPayloads(const QList<QDomElement>& payloads)
{
	d->supportedVideoPayloads = payloads;
}

QList<QDomElement> JingleSessionManager::supportedVideoPayloads() const
{
	return d->supportedVideoPayloads;
}

void JingleSessionManager::setSupportedProfiles(const QStringList& profiles)
{
	d->supportedProfiles = profiles;
}

JingleSession *JingleSessionManager::startNewSession(const Jid& toJid, const QList<JingleContent*>& contents)
{
	XMPP::JingleSession *session = new XMPP::JingleSession(d->client->rootTask(), toJid.full());
	session->setInitiator(d->client->jid().full());
	session->addContents(contents);
	d->sessions << session;
	connect(session, SIGNAL(terminated()), this, SLOT(slotSessionTerminated()));
	//connect(others);
	session->start();
	return session;
}

bool JingleSessionManager::checkSupportedPayloads(JingleContent *c)
{
	qDebug() << "We have" << c->responderPayloads().count() << "responder payloads in this content.";
	for (int i = 0; i < c->payloadTypes().count(); i++)
	{
		qDebug() << "We have" << d->supportedAudioPayloads.count() << "supported payloads.";
		for (int j = 0; j < d->supportedAudioPayloads.count(); j++)
		{
			qDebug() << "compare" << c->payloadTypes().at(i).attribute("name") << "to" << d->supportedAudioPayloads.at(j).attribute("name");
			if (c->payloadTypes().at(i).attribute("name") == d->supportedAudioPayloads.at(j).attribute("name"))
			{
				//This payload name is supported.
				//A static method should be written to compare 2 payloads elements.
				qDebug() << "return true";
				return true;
			}
		}
	}

	qDebug() << "return false";
	return false;
}

bool JingleSessionManager::checkSupportedTransport(JingleContent *c)
{
	Q_UNUSED(c)
	/*for (int i = 0; i < d->supportedTransports.count(); i++)
	{
		qDebug() << "compare" << c->transport().attribute("xmlns") << "to" << d->supportedTransports.at(i);
		if (c->transport().attribute("xmlns") == d->supportedTransports.at(i))
		{
			qDebug() << "return true";
			return true;
		}
	}
	qDebug() << "return false";*/

	return true;
}

void JingleSessionManager::slotRemoveContent(const QString& sid, const QStringList& cNames)
{
	qDebug() << "JingleSessionManager::slotRemoveContent(" << sid << ", " << cNames << ") called.";
	//emit contentRemove(sid, cNames); //The slotRemoveContent slot should not exist so we can connect both signals directly.
	/*
	 * Whatever we have to do at this point will be done by the application on the JingleSession.
	 * That means that the application must keep a list of the JingleSession or this class should
	 * give access to the session list.
	 */
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

void JingleSessionManager::slotSessionInfo(const QDomElement& x)
{
	JingleSession *sess = session(x.attribute("sid"));
	if (sess == 0)
	{
		//unknownSession();
		return;
	}
	sess->addSessionInfo(x.firstChildElement());
}

void JingleSessionManager::slotTransportInfo(const QDomElement& x)
{
	JingleSession *sess = session(x.attribute("sid"));
	if (sess == 0)
	{
		qDebug() << "Session is null, We have a proble here...";
		//unknownSession();
		return;
	}
	//sess->contentWithName(x.firstChildElement().attribute("name"))->addTransportInfo(x.firstChildElement().firstChildElement());
	sess->addTransportInfo(x.firstChildElement());
}

void JingleSessionManager::slotSessionTerminate(const QString& sid, const JingleReason& reason)
{
	Q_UNUSED(reason)
	JingleSession *sess = session(sid);
	if (!sess)
	{
		//sendUnknownSession([need the stanza id]);
		return;
	}

	//Remove the session from the sessions list (the session is not destroyed, it has to be done by the application.)
	for (int i = 0; i < d->sessions.count(); i++)
	{
		if (sess == d->sessions[i])
		{
			d->sessions.removeAt(i);
			break;
		}
	}
	emit sessionTerminate(sess);
	
}

int JingleSessionManager::nextRawUdpPort()
{
	int lastUsed;
	if (d->usedPorts.count() == 0)
		lastUsed = d->firstPort - 1;
	else
		lastUsed = d->usedPorts.last();
	d->usedPorts << lastUsed + 1 << lastUsed + 2;
	qDebug() << "JingleSessionManager::nextRawUdpPort() returns" << (lastUsed + 1);
	return (lastUsed + 1);
}

void JingleSessionManager::setFirstPort(int f)
{
	d->firstPort = f;
}

void JingleSessionManager::slotSessionAccepted(const QDomElement& x)
{
	JingleSession *sess = session(x.attribute("sid"));
	if (sess == 0)
	{
		qDebug() << "Session is null, We have a proble here...";
		//unknownSession();
		return;
	}
	
	qDebug() << "JingleSessionManager::slotSessionAccept(const QDomElement& x) : call sess->sessionAccepted(x);";
	sess->sessionAccepted(x);
}
