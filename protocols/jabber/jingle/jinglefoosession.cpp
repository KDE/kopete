/*
    jinglefoosession.cpp - Definition of foo session

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

	JingleContentType fooType;
	fooType.name = "Foo";
	fooType.xmlns = "http://www.example.com/jingle/foo.html";
	fooType.transportNS = "http://www.example.com/jingle/foo-transport.html";

	//create connection candidates:  need them, no matter what
	fooType.candidates = fooTransport.getLocalCandidates();
	types.push_back(fooType);

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
		if( !element.isNull() && element.attribute("xmlns") == JINGLE_NS && element.attribute("sid") == sid  )
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
		QDomDocument accept = Jingle::createAcceptMessage(me, them, initiator, responder,  transactionID, sid,types);
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
	bool alreadyUsed = false;
	for( int i =0; i< types.size(); i++){
		if(types[i].name == name) alreadyUsed = true;
	}

	bool rightContent;//TODO check if content NS is acceptable
	
	if(rightContent){
		QString transportNS = content.elementsByTagName("transport").item(0).toElement().attribute("xmlns");
		if(transportNS == JINGLE_FOO_TRANSPORT_NS){
			sendStanza( Jingle::createReceiptMessage(stanza).toString() );
			JingleContentType t;
			if(amIInitiator) t.creator="responder";
			else t.creator="initiator";
			t.name = name;
			t.xmlns = content.firstChild().toElement().attribute("xmlns");
			t.transportNS = transportNS;
			//TODO set the information about the content
			t.candidates = fooTransport.getLocalCandidates();
			types.append(t);
			lastMessage = contentAccept;
			transactionID = GEN_ID;
			sendStanza( Jingle::createContentAcceptMessage( myself().full(), peers().first().full(),
				initiator, responder, transactionID, sid, t).toString() );
			sendTransportCandidates(types.size() - 1);
		}else{
			sendStanza( Jingle::createTransportErrorMessage(stanza).toString() );
		}
	}else{
		sendStanza( Jingle::createContentErrorMessage(stanza).toString() );
	}
}




int JingleFooSession::updateContent(QDomElement stanza)
{
	QDomElement content = stanza.elementsByTagName("content").item(0).toElement();
	QString name = content.attribute("name");
	int i = 0;
	bool found = false;
	while( i < types.size() && !found){
		if (types[i].name == name)
			found = true;
		else
			i++;
	}
	//TODO replace stuff in types[i]
	return i;
}

void JingleFooSession::checkContent(QDomElement stanza)
{
	QDomElement content = stanza.elementsByTagName("content").item(0).toElement();
	QString name = content.attribute("name");
	//if (magic FooContent checks) //for a multi-content session, this could depend on the content NS
		int i = 0;
		bool found = false;
		while( i < types.size() && !found){
			if (types[i].name == name)
				found = true;
			else
				i++;
		}
		if(i == types.size() ){
			//what kind of error does this send?
		}
		//TODO replace stuff in types[i]
		lastMessage = contentAccept;
		transactionID = GEN_ID;
		sendStanza( Jingle::createContentAcceptMessage( myself().full(), peers().first().full(),
			initiator, responder, transactionID, sid, types[i]).toString() );
		//QString transNS = content.elementsByTagName("transport").item(0).toElement().attribute("xmlns");
	//else (new FooContent isn't receiveable)
		//give an error
	//endif
}

bool JingleFooSession::addRemoteCandidate(QDomElement contentElement)
{
	QString name = contentElement.attribute("name");
	int i = 0;
	bool found = false;
	while( i < types.size() && !found){
		if (types[i].name == name)
			found = true;
		else
			i++;
	}

	if(!found); //huh?
	
	if(types[i].connection != NULL){
		//already have a connection
	}else{
		QDomElement candidateElement = contentElement.elementsByTagName("candidate").item(0).toElement();
		JingleConnectionCandidate* candidate = fooTransport.createCandidateFromElement(candidateElement);
		if(candidate->isUseful()){
			types[i].connection = candidate;
			if((amIInitiator && types[i].creator == "responder") || (!amIInitiator && types[i].creator == "initiator")){
				transactionID = GEN_ID;
				acknowledged = false;
				if(state == PENDING){
					lastMessage = sessionAccept;
					sendStanza(Jingle::createAcceptMessage(myself().full(), peers().first().full(), initiator, responder, transactionID, sid, types).toString() );
				}else{
					lastMessage = contentAccept;
					sendStanza(Jingle::createContentAcceptMessage(myself().full(), peers().first().full(), initiator, responder, transactionID, sid, types[i]).toString() );
				}
			}
		}else delete candidate;
	}
	return true;
}

bool JingleFooSession::handleSessionAccept(QDomElement stanza)
{
	QDomNodeList contentElements = stanza.elementsByTagName("content");
	for(int j=0; j<contentElements.size(); j++){
		QDomElement contentElement = contentElements.at(j).toElement();
		QString name = contentElement.attribute("name");
		int i = updateContent(contentElement);

		QDomElement candidateElement = contentElement.elementsByTagName("candidate").item(0).toElement();
		//NOTE eventually, createCandidateFromElement should be somewhere in types[i]
		JingleConnectionCandidate* candidate = fooTransport.createCandidateFromElement(candidateElement);
		if( !( candidate->isUseful() ) ){ 
			delete candidate;
			return false;
		}	
		types[i].writeConnection = candidate;
		
	}

	return true;
}

#include "jinglefoosession.moc"
