/*
    jinglevoicesession.cpp - Define a Jingle voice session.

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

#include "jinglevoicesession.h"
#include "jinglesessionmanager.h"

// Qt includes
#include <qdom.h>

// KDE includes
#include <kdebug.h>

// Kopete Jabber includes
#include "jabberaccount.h"
#include "jabberprotocol.h"

#include <xmpp.h>
#include <xmpp_xmlcommon.h>

#include "jinglejabberxml.h"


static bool hasPeer(const JingleVoiceSession::JidList &jidList, const XMPP::Jid &peer)
{
	JingleVoiceSession::JidList::ConstIterator it, itEnd = jidList.constEnd();
	for(it = jidList.constBegin(); it != itEnd; ++it)
	{
		if( (*it).compare(peer, true) )
			return true;
	}

	return false;
}

class JingleVoiceSession::Private
{
public:
	Private()
	 : phoneSessionClient(0L), currentCall(0L)
	{}

	~Private()
	{
		if(currentCall)
			currentCall->Terminate();

		delete currentCall;
	}

	cricket::PhoneSessionClient *phoneSessionClient;
	cricket::Call* currentCall;
};

JingleVoiceSession::JingleVoiceSession(JabberAccount *account, const JidList &peers)
 : JingleSession(account, peers), d(new Private)
{
	slotsProxy = new SlotsProxy(this);

	buzz::Jid buzzJid( account->client()->jid().full().ascii() );

	// Create the phone(voice) session.
	d->phoneSessionClient = new cricket::PhoneSessionClient( buzzJid, account->sessionManager()->cricketSessionManager() );

	//d->phoneSessionClient->SignalSendStanza.connect(slotsProxy, &JingleVoiceSession::SlotsProxy::OnSendingStanza);
	d->phoneSessionClient->SignalCallCreate.connect(slotsProxy, &JingleVoiceSession::SlotsProxy::OnCallCreated);

	// Listen to incoming packets
	connect(account->client()->client(), SIGNAL(xmlIncoming(const QString&)), this, SLOT(receiveStanza(const QString&)));

	new JingleIQResponder(account->client()->rootTask());
}

JingleVoiceSession::~JingleVoiceSession()
{
	kDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << endl;
	delete slotsProxy;
	delete d;
}

QString JingleVoiceSession::sessionType()
{
	return QString(JINGLE_VOICE_SESSION_NS);
}

void JingleVoiceSession::start()
{
	kDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Starting a voice session..." << endl;
	d->currentCall = d->phoneSessionClient->CreateCall();

	QString firstPeerJid = ((XMPP::Jid)peers().first()).full();
	kDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "With peer: " << firstPeerJid << endl;
	d->currentCall->InitiateSession( buzz::Jid(firstPeerJid.ascii()), NULL );

	d->phoneSessionClient->SetFocus(d->currentCall);
	emit sessionStarted();
}

void JingleVoiceSession::accept()
{	
	if(d->currentCall)
	{
		kDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Accepting a voice session..." << endl;

		d->currentCall->AcceptSession(d->currentCall->sessions()[0]);
		d->phoneSessionClient->SetFocus(d->currentCall);
		emit accepted();
	}
}

void JingleVoiceSession::decline()
{
	if(d->currentCall)
	{
		d->currentCall->RejectSession(d->currentCall->sessions()[0]);
		emit declined();
	}
}

void JingleVoiceSession::terminate()
{
	if(d->currentCall)
	{
		d->currentCall->Terminate();
		emit terminated();
	}
}

void JingleVoiceSession::setCall(cricket::Call *call)
{
	kDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Updating cricket::call object." << endl;
	d->currentCall = call;
	d->phoneSessionClient->SetFocus(d->currentCall);
}

void JingleVoiceSession::receiveStanza(const QString &stanza)
{
	QDomDocument doc;
	doc.setContent(stanza);

	// Check if it is offline presence from an open chat
	if( doc.documentElement().tagName() == "presence" ) 
	{
		XMPP::Jid from = XMPP::Jid(doc.documentElement().attribute("from"));
		QString type = doc.documentElement().attribute("type");
		if( type == "unavailable" && hasPeer(peers(), from) ) 
		{
			//qDebug("JingleVoiceCaller: User went offline without closing a call.");
			kDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "User went offline without closing a call." << endl;
			emit terminated();
		}
		return;
	}
	
	// Check if the packet is destined for libjingle.
	// We could use Session::IsClientStanza to check this, but this one crashes
	// for some reason.
	QDomNode node = doc.documentElement().firstChild();
	bool ok = false;
	while( !node.isNull() && !ok ) 
	{
		QDomElement element = node.toElement();
		//NOTE doesn't catch <error> messages
		if( !element.isNull() && element.attribute("xmlns") == JINGLE_NS) 
		{
			ok = true;
		}
		node = node.nextSibling();
	}
	
	// Spread the word
	if( ok )
	{
		kDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Processing stanza" << endl;
		processStanza(doc);
		//d->phoneSessionClient->OnIncomingStanza(e);
	}
}


#include "jinglevoicesession.moc"
