/*
    jinglevoicesession.cpp - Define a Jingle voice session.

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

#define JINGLE_NS "http://www.google.com/session"
#define JINGLE_VOICE_SESSION_NS "http://www.google.com/session/phone"

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
//BEGIN SlotsProxy
/**
 * This class is used to receive signals from libjingle, 
 * which is are not compatible with Qt signals.
 * So it's a proxy between JingeVoiceSession(qt)<->linjingle class.
 */
class JingleVoiceSession::SlotsProxy : public sigslot::has_slots<> 
{
public:
	SlotsProxy(JingleVoiceSession *parent)
	 : voiceSession(parent)
	{}
	
	void OnCallCreated(cricket::Call* call)
	{
		kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "SlotsProxy: CallCreated." << endl;

		call->SignalSessionState.connect(this, &JingleVoiceSession::SlotsProxy::PhoneSessionStateChanged);
		voiceSession->setCall(call);
	}
		
	void PhoneSessionStateChanged(cricket::Call *call, cricket::Session *session, cricket::Session::State state)
	{
		kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "State changed: " << state << endl;

		XMPP::Jid jid(session->remote_address().c_str());
		
		// Do nothing if the session do not contain a peers.
		//if( !voiceSession->peers().contains(jid) )
		if( !hasPeer(voiceSession->peers(), jid) )
			return;

		if (state == cricket::Session::STATE_INIT) 
		{}
		else if (state == cricket::Session::STATE_SENTINITIATE) 
		{}
		else if (state == cricket::Session::STATE_RECEIVEDINITIATE) 
		{
			voiceSession->setCall(call);
		}
		else if (state == cricket::Session::STATE_SENTACCEPT) 
		{}
		else if (state == cricket::Session::STATE_RECEIVEDACCEPT) 
		{
			emit voiceSession->accepted();
		}
		else if (state == cricket::Session::STATE_SENTMODIFY) 
		{}
		else if (state == cricket::Session::STATE_RECEIVEDMODIFY) 
		{
			//qWarning(QString("jinglevoicecaller.cpp: RECEIVEDMODIFY not implemented yet (was from %1)").arg(jid.full()));
		}
		else if (state == cricket::Session::STATE_SENTREJECT) 
		{}
		else if (state == cricket::Session::STATE_RECEIVEDREJECT) 
		{
			emit voiceSession->declined();
		}
		else if (state == cricket::Session::STATE_SENTREDIRECT) 
		{}
		else if (state == cricket::Session::STATE_SENTTERMINATE) 
		{
			emit voiceSession->terminated();
		}
		else if (state == cricket::Session::STATE_RECEIVEDTERMINATE) 
		{
			emit voiceSession->terminated();
		}
		else if (state == cricket::Session::STATE_INPROGRESS) 
		{
			emit voiceSession->sessionStarted();
		}
	}

	void OnSendingStanza(cricket::SessionClient*, const buzz::XmlElement *buzzStanza)
	{
		QString irisStanza(buzzStanza->Str().c_str());
		irisStanza.replace("cli:iq","iq");
		irisStanza.replace(":cli=","=");
	
		voiceSession->sendStanza(irisStanza);
	}
private:
	JingleVoiceSession *voiceSession;
};
//END SlotsProxy

//BEGIN JingleIQResponder
class JingleVoiceSession::JingleIQResponder : public XMPP::Task
{
public:
	JingleIQResponder(XMPP::Task *);
	~JingleIQResponder();

	bool take(const QDomElement &);
};

/**
 * \class JingleIQResponder
 * \brief A task that responds to jingle candidate queries with an empty reply.
 */
 
JingleVoiceSession::JingleIQResponder::JingleIQResponder(Task *parent) :Task(parent)
{
}

JingleVoiceSession::JingleIQResponder::~JingleIQResponder()
{
}

bool JingleVoiceSession::JingleIQResponder::take(const QDomElement &e)
{
	if(e.tagName() != "iq")
		return false;
	
	QDomElement first = e.firstChild().toElement();
	if (!first.isNull() && first.attribute("xmlns") == JINGLE_NS) {
		QDomElement iq = createIQ(doc(), "result", e.attribute("from"), e.attribute("id"));
		send(iq);
		return true;
	}
	
	return false;
}
//END JingleIQResponder

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

	d->phoneSessionClient->SignalSendStanza.connect(slotsProxy, &JingleVoiceSession::SlotsProxy::OnSendingStanza);
	d->phoneSessionClient->SignalCallCreate.connect(slotsProxy, &JingleVoiceSession::SlotsProxy::OnCallCreated);

	// Listen to incoming packets
	connect(account->client()->client(), SIGNAL(xmlIncoming(const QString&)), this, SLOT(receiveStanza(const QString&)));

	new JingleIQResponder(account->client()->rootTask());
}

JingleVoiceSession::~JingleVoiceSession()
{
	kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << endl;
	delete slotsProxy;
	delete d;
}

QString JingleVoiceSession::sessionType()
{
	return QString(JINGLE_VOICE_SESSION_NS);
}

void JingleVoiceSession::start()
{
	kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Starting a voice session..." << endl;
	d->currentCall = d->phoneSessionClient->CreateCall();

	QString firstPeerJid = ((XMPP::Jid)peers().first()).full();
	kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "With peer: " << firstPeerJid << endl;
	d->currentCall->InitiateSession( buzz::Jid(firstPeerJid.ascii()) );

	d->phoneSessionClient->SetFocus(d->currentCall);
}

void JingleVoiceSession::accept()
{	
	if(d->currentCall)
	{
		kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Accepting a voice session..." << endl;

		d->currentCall->AcceptSession(d->currentCall->sessions()[0]);
		d->phoneSessionClient->SetFocus(d->currentCall);
	}
}

void JingleVoiceSession::decline()
{
	if(d->currentCall)
	{
		d->currentCall->RejectSession(d->currentCall->sessions()[0]);
	}
}

void JingleVoiceSession::terminate()
{
	if(d->currentCall)
	{
		d->currentCall->Terminate();
	}
}

void JingleVoiceSession::setCall(cricket::Call *call)
{
	kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Updating cricket::call object." << endl;
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
			kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "User went offline without closing a call." << endl;
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
		if( !element.isNull() && element.attribute("xmlns") == JINGLE_NS) 
		{
			ok = true;
		}
		node = node.nextSibling();
	}
	
	// Spread the word
	if( ok )
	{
		kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Handing down buzz::stanza" << endl;
		buzz::XmlElement *e = buzz::XmlElement::ForStr(stanza.ascii());
		d->phoneSessionClient->OnIncomingStanza(e);
	}
}

#include "jinglevoicesession.moc"
