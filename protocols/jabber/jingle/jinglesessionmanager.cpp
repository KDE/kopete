/*
    jinglesessionmanager.cpp - Manage Jingle sessions.

    Copyright (c) 2006      by Michaël Larouche     <larouche@kde.org>

    Kopete    (c) 2001-2006 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "jinglesessionmanager.h"

//#include "jinglesession.h" //forward declaration works
#include "jinglevoicesession.h"
#include "jinglefoosession.h"
#include "jinglenetwork.h"

#include "jinglewatchsessiontask.h"
#include "jingleinfotask.h" //for portaddress

#include "jabberaccount.h"
#include "jabberprotocol.h"

#include <kdebug.h>
//Added by qt3to4:
#include <Q3ValueList>

#include <QtNetwork>

/* FIXME:
 * Bad namespaces. Check XMPP Jingle specification
 */

#define JINGLE_NS "http://www.google.com/session"
#define JINGLE_VOICE_SESSION_NS "http://www.google.com/session/phone"
#define JINGLE_FOO_NS "http://kopete.kde.com/jingle/foo.html"

//BEGIN JingleSessionManager::SlotsProxy
/*class JingleSessionManager;
class JingleSessionManager::SlotsProxy : public QObject
{
	Q_OBJECT
public:
	SlotsProxy(JingleSessionManager *parent)
	 : sessionManager(parent)
	{}
	
	void OnSignalingRequest()
	{
		kDebug(JABBER_DEBUG_GLOBAL) << "Requesting Jingle signaling.";
		//sessionManager->cricketSessionManager()->OnSignalingReady();
	}
	
	
private:
	JingleSessionManager *sessionManager;
};*/


//BEGIN JingleSessionManager::Private
class JingeSession;
class JingleSessionManager::Private
{
public:
	Private(JabberAccount *t_account)
	 : account(t_account), watchSessionTask(0L)
	{}

	~Private()
	{
		delete networkManager;
		delete watchSessionTask;
	}
	
	JabberAccount *account;
	QList<JingleSession*> sessionList;
	JingleWatchSessionTask *watchSessionTask;

	JingleNetworkManager *networkManager;
};
//END JingleSessionManager::Private

JingleSessionManager::JingleSessionManager(JabberAccount *account)
/* : QObject(account), d(new Private(account))*/
{

	// Create watch incoming session task.
	d->watchSessionTask = new JingleWatchSessionTask(account->client()->rootTask());
	connect(d->watchSessionTask, SIGNAL(watchSession(const QString &, const QString &)), this, SLOT(slotIncomingSession(const QString &, const QString &)));

	// Create global cricket variables common to all sessions.
	// Seed random generation with the JID of the account.
	QString accountJid = account->client()->jid().full();
	//cricket::InitRandom( accountJid.toAscii(), accountJid.length() );

	
	
	// Init the port allocator(select best ports) with the Google STUN server to help.
	//talk_base::SocketAddress *googleStunAddress = new talk_base::SocketAddress("64.233.167.126", 19302);
	PortAddress *googleStunAddress=new PortAddress(QHostAddress(QString("64.233.167.126")), 19302);

	// Create the JingleNetworkManager that manager local network connections
	d->networkManager = new JingleNetworkManager(googleStunAddress);
}

JingleSessionManager::~JingleSessionManager()
{
	kDebug(JABBER_DEBUG_GLOBAL) ;

	kDebug(JABBER_DEBUG_GLOBAL) << "Cleaning up Jingle sessions.";
	QList<JingleSession*>::Iterator it, itEnd = d->sessionList.end();
	for(it = d->sessionList.begin(); it != itEnd; ++it)
	{
		JingleSession *deletedSession = *it;
		if( deletedSession )
		{
			kDebug(JABBER_DEBUG_GLOBAL) << "deleting a session.";
			delete deletedSession;
		}
	}
	kDebug(JABBER_DEBUG_GLOBAL) << "Done Cleaning up Jingle sessions.";

	delete d;
}

JabberAccount *JingleSessionManager::account()
{
	return d->account;
}

JingleSession *JingleSessionManager::createSession(const QString &sessionType, const JidList &peers)
{	
	JingleSession *newSession = 0L;

	if(sessionType == JINGLE_VOICE_SESSION_NS)
	{
		kDebug(JABBER_DEBUG_GLOBAL) << "Creating a voice session";
		//newSession = new JingleVoiceSession(account(), peers);
	}else if (sessionType == JINGLE_FOO_NS)
	{
		kDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Creating a foo session" <<endl;
		newSession = new JingleFooSession(account(), peers);
	}
	
	if(newSession)
		d->sessionList.append(newSession);

	return newSession;
}

JingleSession *JingleSessionManager::createSession(const QString &sessionType, const XMPP::Jid &user)
{
	JingleSessionManager::JidList jidList;
	jidList.append(user);
	
	return createSession(sessionType, jidList);
}

void JingleSessionManager::removeSession(JingleSession *session)
{
	kDebug(JABBER_DEBUG_GLOBAL) << "Removing a jingle session.";

	d->sessionList.removeAll(session);
	delete session;	
}

void JingleSessionManager::slotIncomingSession(const QString &sessionType, const QString &initiator)
{
	kDebug(JABBER_DEBUG_GLOBAL) << "Incoming session: " << sessionType << ". Initiator: " << initiator;

	JingleSession *newSession = createSession(sessionType, XMPP::Jid(initiator));
	//emit incomingSession(sessionType, newSession);
}

/*void OnSignalingReady()
{
	//for(int i = 0; i < d->sessionList.size(); i++);

}*/

#include "jinglesessionmanager.moc"
