/*
    jinglesessionmanager.cpp - Manage Jingle sessions.

    Copyright (c) 2006      by Michaël Larouche     <michael.larouche@kdemail.net>

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
// libjingle before everything else to not clash with Qt
#define POSIX
#include "talk/xmpp/constants.h"
#include "talk/base/sigslot.h"
#include "talk/xmpp/jid.h"
#include "talk/xmllite/xmlelement.h"
#include "talk/xmllite/xmlprinter.h"
#include "talk/base/network.h"
#include "talk/p2p/base/session.h"
#include "talk/p2p/base/sessionmanager.h"
#include "talk/p2p/base/helpers.h"
#include "talk/p2p/client/basicportallocator.h"
#include "talk/p2p/client/sessionclient.h"
#include "talk/base/physicalsocketserver.h"
#include "talk/base/thread.h"
#include "talk/base/socketaddress.h"
#include "talk/session/phone/call.h"
#include "talk/session/phone/phonesessionclient.h"
#include "talk/session/sessionsendtask.h"


#include "jinglesessionmanager.h"

//#include "jinglesession.h"
#include "jinglevoicesession.h"

#include "jinglewatchsessiontask.h"

#include "jabberaccount.h"
#include "jabberprotocol.h"

#include <kdebug.h>

#define JINGLE_NS "http://www.google.com/session"
#define JINGLE_VOICE_SESSION_NS "http://www.google.com/session/phone"

//BEGIN JingleSessionManager::SlotsProxy
class JingleSessionManager;
class JingleSessionManager::SlotsProxy : public sigslot::has_slots<>
{
public:
	SlotsProxy(JingleSessionManager *parent)
	 : sessionManager(parent)
	{}
	
	void OnSignalingRequest()
	{
		kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Requesting Jingle signaling." << endl;
		sessionManager->cricketSessionManager()->OnSignalingReady();
	}
	
	
private:
	JingleSessionManager *sessionManager;
};

//END JingleSessionManager::SlotsProxy

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
		delete portAllocator;
		delete sessionThread;
		delete cricketSessionManager;
	}
	
	JabberAccount *account;
	QValueList<JingleSession*> sessionList;
	JingleWatchSessionTask *watchSessionTask;

	cricket::NetworkManager *networkManager;
	cricket::BasicPortAllocator *portAllocator;
	cricket::Thread *sessionThread;
	cricket::SessionManager *cricketSessionManager;
};
//END JingleSessionManager::Private

JingleSessionManager::JingleSessionManager(JabberAccount *account)
 : QObject(account, 0), d(new Private(account))
{
	// Create slots proxy for libjingle
	slotsProxy = new SlotsProxy(this);

	// Create watch incoming session task.
	d->watchSessionTask = new JingleWatchSessionTask(account->client()->rootTask());
	connect(d->watchSessionTask, SIGNAL(watchSession(const QString &, const QString &)), this, SLOT(slotIncomingSession(const QString &, const QString &)));

	// Create global cricket variables common to all sessions.
	// Seed random generation with the JID of the account.
	QString accountJid = account->client()->jid().full();
	cricket::InitRandom( accountJid.ascii(), accountJid.length() );

	// Create the libjingle NetworkManager that manager local network connections
	d->networkManager = new cricket::NetworkManager();
	
	// Init the port allocator(select best ports) with the Google STUN server to help.
	cricket::SocketAddress *googleStunAddress = new cricket::SocketAddress("64.233.167.126", 19302);
	// TODO: Define a relay server.
  	d->portAllocator = new cricket::BasicPortAllocator(d->networkManager, googleStunAddress, 0L);

	// Create the Session manager that manager peer-to-peer sessions.
	d->sessionThread = new cricket::Thread();
	d->cricketSessionManager = new cricket::SessionManager(d->portAllocator, d->sessionThread);
	d->cricketSessionManager->SignalRequestSignaling.connect(slotsProxy, &JingleSessionManager::SlotsProxy::OnSignalingRequest);
	d->cricketSessionManager->OnSignalingReady();

	d->sessionThread->Start();
}

JingleSessionManager::~JingleSessionManager()
{
	kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << endl;

	kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Cleaning up Jingle sessions." << endl;
	QValueList<JingleSession*>::Iterator it, itEnd = d->sessionList.end();
	for(it = d->sessionList.begin(); it != itEnd; ++it)
	{
		JingleSession *deletedSession = *it;
		if( deletedSession )
		{
			kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "deleting a session." << endl;
			delete deletedSession;
		}
	}
	kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Done Cleaning up Jingle sessions." << endl;

	delete d;
}

cricket::SessionManager *JingleSessionManager::cricketSessionManager()
{
	return d->cricketSessionManager;
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
		kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Creating a voice session" << endl;
		newSession = new JingleVoiceSession(account(), peers);
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
	kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Removing a jingle session." << endl;

	d->sessionList.remove(session);
	delete session;	
}

void JingleSessionManager::slotIncomingSession(const QString &sessionType, const QString &initiator)
{
	kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Incoming session: " << sessionType << ". Initiator: " << initiator << endl;

	JingleSession *newSession = createSession(sessionType, XMPP::Jid(initiator));
	emit incomingSession(sessionType, newSession);
}

#include "jinglesessionmanager.moc"
