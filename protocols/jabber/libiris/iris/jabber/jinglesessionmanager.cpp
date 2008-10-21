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
	connect(d->pjs, SIGNAL(sessionAccepted(const QDomElement&)),
		this, SLOT(slotSessionAccepted(const QDomElement&)));

	Features f = d->client->features();
	
	f.addFeature(NS_JINGLE);
//	f.addFeature(NS_JINGLE_TRANSPORTS_ICE);
	f.addFeature(NS_JINGLE_TRANSPORTS_RAW);
	f.addFeature(NS_JINGLE_APPS_RTP);
//	f.addFeature("urn:xmpp:tmp:jingle:apps:video-rtp");

	d->client->setFeatures(f);

	d->firstPort = 9000;
	
	//Get External IP address, This is not Standard and might not work but let's try it before we have ICE support
	//d->http = new QHttp(this);
	//d->http->setHost("www.swlink.net");
	//connect(d->http, SIGNAL(done(bool)), this, SLOT(slotExternalIPDone(bool)));
	//d->http->get("/~styma/REMOTE_ADDR.shtml");
	//FIXME:Crashes when www.swlink.net is not accessible. (down)
}

void JingleSessionManager::slotExternalIPDone(bool err)
{
	d->ip = "";
	if (err)
	{
		qDebug() << "err =" << err;
		d->http->deleteLater();
		return;
	}

	QByteArray data = d->http->readAll();
	// Parse XML here...
	// We know that the ip is on the 5th line.
	d->ip = data.split('\n').at(4);
	qDebug() << "Received External IP :" << d->ip;
	d->ip= "";
	
	delete d->http;
}

QString JingleSessionManager::externalIP() const
{
	return d->ip;
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

void JingleSessionManager::slotSessionTerminated()
{
	JingleSession* sess = static_cast<JingleSession*>(sender());

	for (int i = 0; i < d->sessions.count(); i++)
	{
		if (d->sessions[i] == sess)
			d->sessions.removeAt(i);
	}
}

void JingleSessionManager::slotSessionIncoming()
{
	qDebug() << "JingleSessionManager::slotSessionIncoming() called.";
	
	JingleSession *sess = d->pjs->takeNextIncomingSession();
	d->sessions << sess;
	connect(sess, SIGNAL(terminated()), this, SLOT(slotSessionTerminated()));

	// TODO:Check if the Transport method is supported.
	for (int i = 0; i < sess->contents().count(); i++)
	{
		JingleContent *c = sess->contents()[i];

		// Check payloads for the content c
		if (!checkSupportedPayloads(c))
		{
			//I think we have to send a no payload supported stanza.
			//sess->terminate(UnsupportedApplications);
			//--> no, only if all transports are unsupported
			//in this case, a content-remove MUST be sent.
			return;
		}
		
		if (!checkSupportedTransport(c))
		{
			//I think we have to send a no transport supported stanza.
			//sess->terminate(UnsupportedTransports);
			//--> no, only if all applications are unsupported
			//in this case, a content-remove MUST be sent.
			return;
		}

		//Set supported payloads for this content.
		c->setPayloadTypes(c->type() == JingleContent::Audio ? d->supportedAudioPayloads : d->supportedVideoPayloads);
	}
	
	emit newJingleSession(sess);
	
	d->sessions.last()->ring();
	
	d->sessions.last()->startNegotiation();
}

bool JingleSessionManager::checkSupportedPayloads(JingleContent *c)
{
	/*for (int i = 0; i < c->payloadTypes().count(); i++)
	{
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
	qDebug() << "return false";*/

	return true;
}

bool JingleSessionManager::checkSupportedTransport(JingleContent *c)
{
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
