/*
 * jinglesessionmanager.cpp - Manager for Jingle sessions
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

#include "jinglesessionmanager.h"
#include "jingletasks.h"

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
};

JingleSessionManager::JingleSessionManager(Client* c)
: d(new Private)
{
	qDebug() << "JingleSessionManager::JingleSessionManager created.";
	d->client = c;
	d->pjs = new JT_PushJingleAction(d->client->rootTask());
	connect(d->pjs, SIGNAL(newSessionIncoming()),
		this, SLOT(slotSessionIncoming()));
	connect(d->pjs, SIGNAL(removeContent(const QString&, const QStringList&)),
		this, SLOT(slotRemoveContent(const QString&, const QStringList&)));
	connect(d->pjs, SIGNAL(sessionInfo(const QDomElement&)),
		this, SLOT(slotSessionInfo(const QDomElement&)));
	connect(d->pjs, SIGNAL(transportInfo(const QDomElement&)),
		this, SLOT(slotTransportInfo(const QDomElement&)));
	connect(d->pjs, SIGNAL(sessionTerminate(const QString&, const JingleReason&)),
		this, SLOT(slotSessionTerminate(const QString&, const JingleReason&)));

	Features f = d->client->features();
	
	f.addFeature("urn:xmpp:tmp:jingle");
//	f.addFeature("urn:xmpp:tmp:jingle:transports:ice-udp");
	f.addFeature("urn:xmpp:tmp:jingle:transports:raw-udp");
	f.addFeature("urn:xmpp:tmp:jingle:apps:audio-rtp");
	f.addFeature("urn:xmpp:tmp:jingle:apps:video-rtp");

	d->client->setFeatures(f);

	d->firstPort = 9000;
}

JingleSessionManager::~JingleSessionManager()
{

}

void JingleSessionManager::setSupportedTransports(const QStringList& transports)
{
	d->supportedTransports = transports;
}

void JingleSessionManager::setSupportedAudioPayloads(const QList<QDomElement>& payloads)
{
	d->supportedAudioPayloads = payloads;
}

void JingleSessionManager::setSupportedVideoPayloads(const QList<QDomElement>& payloads)
{
	d->supportedVideoPayloads = payloads;
}

void JingleSessionManager::setSupportedProfiles(const QStringList& profiles)
{
	d->supportedProfiles = profiles;
}

JingleSession *JingleSessionManager::startNewSession(const Jid& toJid, const QList<JingleContent*>& contents)
{
	XMPP::JingleSession *session = new XMPP::JingleSession(d->client->rootTask(), toJid.full());
	session->addContents(contents);
	d->sessions << session;
	connect(session, SIGNAL(terminated()), this, SLOT(slotSessionTerminated()));
	//connect(others);
	session->start();
	return session;
}

void JingleSessionManager::slotSessionIncoming()
{
	qDebug() << "JingleSessionManager::slotSessionIncoming() called.";
	d->sessions << d->pjs->takeNextIncomingSession();

	// TODO:Check if at least one payload is supported.
	// 	Check if the Transport method is supported.

	// FIXME:
	// 	QList<T>.last() should be called only if the list is not empty.
	// 	Could it happen here as we just append an element to the list ?
	emit newJingleSession(d->sessions.last());
	qDebug() << "SEND RINGING.";
	d->sessions.last()->ring();
	qDebug() << "START NEGOTIATION";
	d->sessions.last()->startNegotiation();
}

//void JingleSessionManager::removeContent(const QString& sid, const QString& cName)
//{
//	for (int i = 0; i < )
//}

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

/*void JingleSessionManager::slotSessionTerminated()
{
	JingleSession* sess = (JingleSession*) sender();
	//TODO: Before we delete the session, we must stop sending data and close connection.
	//	I dont know if we stop sending data before or after sending the stanza (I don't think it matters as it a UDP socket).
	//	A signal should be sent anyway.
	if (sess != 0)
		delete sess;
}*/

void JingleSessionManager::slotSessionTerminate(const QString& sid, const JingleReason& reason)
{
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

