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
};

JingleSessionManager::JingleSessionManager(Client* c)
: d(new Private)
{
	qDebug() << "JingleSessionManager::JingleSessionManager created.";
	d->client = c;
	d->pjs = new JT_PushJingleAction(d->client->rootTask());
	connect(d->pjs, SIGNAL(newSessionIncoming()), this, SLOT(slotSessionIncoming()));
	connect(d->pjs, SIGNAL(removeContent(const QString&, const QStringList&)), this, SLOT(slotRemoveContent(const QString&, const QStringList&)));
	connect(d->pjs, SIGNAL(transportInfo(const QDomElement&)), this, SLOT(slotTransportInfo(const QDomElement&)));

	Features f = d->client->features();
	
	f.addFeature("urn:xmpp:tmp:jingle");
	f.addFeature("urn:xmpp:tmp:jingle:transports:ice-udp");
	f.addFeature("urn:xmpp:tmp:jingle:transports:raw-udp");
//	f.addFeature("urn:xmpp:tmp:jingle:apps:audio-rtp");
	f.addFeature("urn:xmpp:tmp:jingle:apps:video-rtp");

	d->client->setFeatures(f);
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

// const QStringList& transports are the transports supported by the responder.
void JingleSessionManager::startNewSession(const Jid& toJid, const QList<JingleContent*>& contents)
{
	XMPP::JingleSession *session = new XMPP::JingleSession(d->client->rootTask(), toJid.full());
	session->addContents(contents);
	d->sessions << session;
	connect(session, SIGNAL(terminated()), this, SLOT(slotSessionTerminated()));
	session->start();
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
	d->sessions.last()->ring();
	d->sessions.last()->startNegotiation();
	emit newJingleSession(d->sessions.last());
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

void JingleSessionManager::slotTransportInfo(const QDomElement& x)
{
	QString sid = x.attribute("sid");
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
	if (sess == 0)
	{
		//unknownSession();
		return;
	}
	//sess->contentWithName(x.firstChildElement().attribute("name"))->addTransportInfo(x.firstChildElement().firstChildElement());
	sess->addTransportInfo(x.firstChildElement().firstChildElement());
}

void JingleSessionManager::slotSessionTerminated()
{
	JingleSession* sess = (JingleSession*) sender();
	//TODO: Before we delete the session, we must stop sending data and close connection.
	//	I dont know if we stop sending data before or after sending the stanza (I don't think it matters as it a UDP socket).
	//	A signal should be sent anyway.
	if (sess != 0)
		delete sess;
}
