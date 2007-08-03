/*
    jinglevoicesessiondialog.cpp - GUI for a voice session.

    Copyright (c) 2007      by Joshua Hodosh     <josh.hodosh@gmail.com>

    Kopete    (c) 2001-2007 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/
#include "jinglefoosession.h"

#define JINGLE_NS "http://www.xmpp.org/extensions/xep-0166.html#ns"
#define JINGLE_FOO_NS "http://kopete.kde.com/jingle/foo.html"
#define JINGLEFOOTRANSPORT "http://kopete.kde.com/jingle/foo-transport.html"

#include <QtXml>
#include "jinglejabberxml.h"
#include "jabberaccount.h"
#include "jinglesession.h"
#include "jabberprotocol.h"
#include "jinglefooconnectioncandidate.h"

#include <kdebug.h>

#define GEN_ID (account()->client()->client()->genUniqueId())


JingleFooSession::JingleFooSession(JabberAccount *account, const JidList &peers)
 : JingleSession(account, peers)
{

	XMPP::Jid jid( account->client()->jid());
	state = PENDING;
	JingleContentType fooType;
	fooType.name = "Foo";
	fooType.xmlns = "http://www.example.com/jingle/foo.html";
	fooType.transportNS = "http://www.example.com/jingle/foo-transport.html";

	//create connection candidates:  need them, no matter what
	fooType.candidates = fooTransport.getLocalCandidates();
	types.push_back(fooType);

	// Listen to incoming packets
	connect(account->client()->client(), SIGNAL(xmlIncoming(const QString&)), this, SLOT(receiveStanza(const QString&)));

}

JingleFooSession::~JingleFooSession()
{
	kDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << endl;
	
} 

void JingleFooSession::start()
{

	sid = account()->client()->client()->genUniqueId();
	initiator = myself().full();
	amIInitiator = true;
	responder = peers().first().full();

	transactionID = GEN_ID;

	QDomDocument message = Jingle::createInitializationMessage(initiator, responder, transactionID, sid, initiator, types);	

	lastMessage = sessionInitiate;
	sendStanza( message.toString() );
	amIInitiator = true;

	emit sessionStarted();

}

QString JingleFooSession::sessionType()
{
	return QString(JINGLE_FOO_NS);
}

//NOTE this function needs a better place, to to be replaced.
static bool hasPeer(const JingleFooSession::JidList &jidList, const XMPP::Jid &peer)
{
	JingleFooSession::JidList::ConstIterator it, itEnd = jidList.constEnd();
	for(it = jidList.constBegin(); it != itEnd; ++it)
	{
		if( (*it).compare(peer, true) )
			return true;
	}

	return false;
}
void JingleFooSession::receiveStanza(const QString &stanza)
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
			kDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "User went offline without closing a call." << endl;
			state = ENDED;
			emit terminated();
		}
		return;
	}

	// Check if the packet is a jingle message.
	QDomNode node = doc.documentElement().firstChild();
	bool ok = false;
	while( !node.isNull() && !ok ) 
	{
		QDomElement element = node.toElement();

		//NOTE big assumption: we're getting the session-initiate message
		if( sid.isEmpty() ) sid = element.attribute("sid");
		// Catch messages that are a) jingle messages and b) for this session
		if( !element.isNull() && ((element.attribute("xmlns") == JINGLE_NS && element.attribute("sid") == sid ) 
			|| (doc.documentElement().attribute("id") == transactionID )) )
		{
			ok = true;
		}
		node = node.nextSibling();
	}

	// they're responding to something: might be an error or response
	if(doc.documentElement().attribute("id") == transactionID ){
		ok=true;
	}

	// It's for me
	if( ok )
	{
		processStanza(doc);
	}
}


void JingleFooSession::accept()
{
	if(state == PENDING){
		QString me = myself().full();
		QString them = peers().first().full();
		transactionID = GEN_ID;
		//QDomDocument accept = Jingle::createAcceptMessage(myself().full(), peers().first().full(), initiator, responder,  transactionID,&sid,types);
		QDomDocument accept = Jingle::createAcceptMessage(me, them, initiator, responder,  transactionID, sid,types, connection);
		sendStanza(accept.toString());
		state = ACTIVE;
		lastMessage = sessionAccept;
		emit accepted();
	}
}

void JingleFooSession::decline()
{
	//It SHOULD be "PENDING", but decline and terminate do the same thing. so it doesn't matter.
	if(state != ENDED)
	{
		QString me = myself().full();
		QString them = peers().first().full();
		//QString declined(declined);
		transactionID = GEN_ID;
		//QDomDocument decline = Jingle::createTerminateMessage(myself().full(), peers().first().full(), initiator, responder, transactionID,sid,"Declined");
		QDomDocument decline = Jingle::createTerminateMessage(me, them, initiator, responder, transactionID,sid, "Declined");
		sendStanza(decline.toString());
		state = ENDED;
		emit declined();
	}
}

void JingleFooSession::terminate()
{
	if(state != ENDED)
	{
		transactionID = GEN_ID;
		QDomDocument terminate = Jingle::createTerminateMessage(myself().full(), peers().first().full(), initiator, responder, transactionID,sid,"Bye");
		sendStanza(terminate.toString());
		state = ENDED;
		emit terminated();
	}
}

void JingleFooSession::checkNewContent(QDomElement stanza)
{
	QDomElement content = stanza.elementsByTagName("content").item(0).toElement(); //NOTE need to check for nulls
	//NOTE assumes only one type of content.  Since this handles session-initiate, there could be multiple
	QString name = content.attribute("name");
	bool rightContent = false;
	for( int i =0; i< types.size(); i++){
		if(types[i].name == name) rightContent = true;
	}
	if(rightContent){
		QString transportNS = content.elementsByTagName("transport").item(0).toElement().attribute("xmlns");
		if(transportNS == JINGLEFOOTRANSPORT){
			sendStanza( Jingle::createReceiptMessage(stanza).toString() );
			int i;//TODO add candidate, set i to its index
			sendTransportCandidates(i);
		}else{
			sendStanza( Jingle::createTransportErrorMessage(stanza).toString() );
		}
	}else{
		sendStanza( Jingle::createContentErrorMessage(stanza).toString() );
	}
}


void JingleFooSession::removeContent(QDomElement stanza)
{
	QDomElement content = stanza.firstChildElement().firstChildElement(); //NOTE need to check for nulls (bad requests)
	QString name = content.attribute("name");
	bool rightContent = false;
	int i = -1;

	do{
		i++;
		if(types[i].name == name) rightContent = true;
	}while( i < types.size() && !rightContent);

	types.removeAt(i);

	//send reciept here, in case of termination
	sendStanza(Jingle::createReceiptMessage(stanza).toString() );

	if(types.empty()){
		terminate();
	}else{
		//NOTE the standard is fuzzy; is this message supposed to be sent?
// 		transactionID = GEN_ID;
// 		lastMessage = contentAccept;
// 		sendStanza(Jingle::createContentAcceptMessage(myself().full(), peers().first().full(), initiator, responder, transactionID, sid, types).toString() );
	}
}

void JingleFooSession::updateContent(QDomElement stanza)
{
	QDomElement content = stanza.elementsByTagName("content").item(0).toElement();
	QString name = content.attribute("name");
	int i = 0;
	while( i < types.size() ){
		if (types[i].name == name) break; //breaks are bad.
	}
	//TODO replace stuff in types[i]

}

void JingleFooSession::checkContent(QDomElement stanza)
{
	QDomElement content = stanza.elementsByTagName("content").item(0).toElement();
	QString name = content.attribute("name");
	//if (magic FooContent checks)
		int i = 0;
		while( i < types.size() ){
			if (types[i].name == name) break; //breaks are bad.
		}
		if(i == types.size() ){
			//what kind of error does this send?
		}
		//TODO replace stuff in types[i]
		//QString transNS = content.elementsByTagName("transport").item(0).toElement().attribute("xmlns");
		//NOTE standard implies here we should again send transport candidates based on transNS
	//else (new FooContent isn't receiveable)
		//again, standard is fuzzy. used to be content-decline.
	//endif
}

bool JingleFooSession::addRemoteCandidate(QDomElement transportElement)
{
	JingleConnectionCandidate candidate = fooTransport.createCandidateFromElement(transportElement);

	return true;
}

void JingleFooSession::sendTransportCandidates(int contentIndex)
{
	//NOTE instead of a foreach, this could pass the ContentType and index of the ConnectionCandidate
	foreach (JingleConnectionCandidate candidate, types[contentIndex].candidates){
		lastMessage = transportInfo;
		transactionID = GEN_ID; //TODO make this a queue
		sendStanza( Jingle::createTransportCandidateMessage(
			myself().full(), peers().first().full(), initiator, responder, transactionID, sid,
			types[contentIndex].name, types[contentIndex].creator, types[contentIndex].transportNS,
			&candidate).toString() ) ;
	}
}
#include "jinglefoosession.moc"
