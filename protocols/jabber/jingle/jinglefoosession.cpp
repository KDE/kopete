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




JingleFooSession::JingleFooSession(JabberAccount *account, const JidList &peers)
 : JingleSession(account, peers)
{

	XMPP::Jid jid( account->client()->jid());
	state = JingleStateEnum::PENDING;

	//create connection candidates:  need them, no matter what
	types.candidates = JingleFooTransport.getLocalCandidates();
	types.push_back(JingleContentType("Foo","http://www.example.com/jingle/foo.html","http://www.example.com/jingle/foo-transport.html");

	// Listen to incoming packets
	connect(account->client()->client(), SIGNAL(xmlIncoming(const QString&)), this, SLOT(receiveStanza(const QString&)));

	new JingleIQResponder(account->client()->rootTask());
}

JingleFooSession::~JingleFooSession()
{
	kDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << endl;
	
} 

void JingeFooSession::start()
{

	sid = account->client()->genUniqueId();
	initiator = jid->full();
	responder = peers->first()->full();

	QDomElement message = createInitializationMessage(jid->full(), peers()->first()->full() account->client()->genUniqueId(), &sid, &initiator, types);	

	send(message);

	emit sessionStarted();

}

QString JingleFooSession::sessionType()
{
	return QString(JINGLE_FOO_SESSION_NS);
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
			state = JingleStateEnum::ENDED;
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
		//NOTE doesn't catch <error> messages
		if( !element.isNull() && element.attribute("xmlns") == JINGLE_NS && element.attribute("sid") == sid )
		{
			ok = true;
		}
		node = node.nextSibling();
	}

	// It's for me
	if( ok )
	{
		processStanza(doc);
	}
}


void JingleFooSession::accept()
{
	if(state == JingleStateEnum::PENDING){
		QDomElement accept = Jingle::createAcceptMessage(jid->full(), peers()->first()->full(), &initiator, &responder,  account->client()->genUniqueId(),&sid,types);
		send(accept);
		state = JingleStateEnum::ACTIVE;
		emit accepted();
	}
}

void JingleFooSession::decline()
{
	//It SHOULD be "PENDING", but decline and terminate do the same thing. so it doesn't matter.
	if(state != JingleStateEnum::ENDED)
	{
		QDomElement decline = Jingle::createTerminateMessage(jid->full(), peers()->first()->full(), &initiator, &responder, account->client()->genUniqueId(),&sid,"Declined");
		send(decline);
		state = JingleStateEnum::ENDED;
		emit declined();
	}
}

void JingleFooSession::terminate()
{
	if(state =! JingleStateEnum::ENDED)
	{
		QDomElement terminate = Jingle::createTerminateMessage(jid->full(), peers()->first()->full(), &initiator, &responder, account->client()->genUniqueId(),&sid,"Bye");
		send(terminate);
		state = JingleStateEnum::ENDED;
		emit terminated();
	}
}

QDomDocument JingleFooSession::checkPayload(QDomElement stanza)
{
	QDomElement content = stanza.elementsByTagName("content").item(0); //NOTE need to check for nulls
	//NOTE assumes only one type of content.
	QString name = content.attribute("name");
	bool rightContent = false;
	for( int i =0; i< types.length(); i++){
		if(types[i].name == name) rightContent = true;
	}
	if(rightContent){
		QString transportNS = content.elementsByTagName("transport").item(0).attribute("xmlns");
		if(transportNS == JINGLEFOOTRANSPORT){
			return Jingle::createReceiptMessage(stanza);
		}else{
			return Jingle::createTransportErrorMessage(stanza);
		}
	}else{
		return Jingle::createContentErrorMessage(stanza);
	}
}


void JingleFooSession::removeContent(QDomElement stanza)
{
	QDomElement content = stanza.firstChildElement().firstChildELement(); //NOTE need to check for nulls
	//NOTE assumes only one type of content.
	QString name = content.attribute("name");
	bool rightContent = false;
	int i = -1;

	do{
		i++;
		if(types[i].name == name) rightContent = true;
	}while( i < types.length() && !rightContent);

	types.remove(i);
	if(types.empty()){
		terminate();
	}
}

#include "jinglefoosession.moc"
