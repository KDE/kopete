/*
    jinglesession.h - Define a Jingle session.

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
#include "jinglesession.h"

#include "jinglejabberxml.h"
#include "jingletransport.h"

#include <kdebug.h>

#include "jabberaccount.h"
#include "jabberprotocol.h"

bool inline JingleSession::isModifying(){
	if(lastMessage == contentAdd || lastMessage == contentRemove || lastMessage == contentModify ) return true;
	return false;
}

class JingleSession::Private
{
public:
	Private(JabberAccount *t_account, const JidList &t_peers)
	 :  peers(t_peers), account(t_account)
	{}

	XMPP::Jid myself;
	JidList peers;
	JabberAccount *account;
};

JingleSession::JingleSession(JabberAccount *account, const JidList &peers)
 : QObject(account, 0), d(new Private(account, peers))
{
	d->myself = account->client()->jid();
	state = PENDING;
}

JingleSession::~JingleSession()
{
	kDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << endl;
	delete d;
	foreach (JingleContentType content, types){
		if(content.writeConnection != NULL) delete content.writeConnection;
		if(content.connection != NULL) delete content.connection;
	}
}

const XMPP::Jid &JingleSession::myself() const
{
	return d->myself;
}

const JingleSession::JidList &JingleSession::peers() const
{
	return d->peers;
}

JingleSession::JidList &JingleSession::peers()
{
	return d->peers;
}
JabberAccount *JingleSession::account()
{
	return d->account;
}

void JingleSession::sendStanza(const QString &stanza)
{
	account()->client()->send( stanza );
}

void JingleSession::processStanza(QDomDocument doc)
{
	QDomElement root = doc.documentElement();
	QString type = root.attribute("type");
	
	//error
	if( type == "error"){
		QDomElement errorElement = root.firstChildElement();
		if( !errorElement.isNull() ){
			handleError(errorElement);
		}

	}else if(type == "set"){
		QDomElement jingleElement = root.firstChildElement();
		if( jingleElement.isNull() ){

		}else if( state == ENDED){ //session is already over
			sendStanza( Jingle::createUnknownSessionError(root).toString() );
		}else{
			QString action = jingleElement.attribute("action");
			if(action == "session-accept"){

				if(state != PENDING){
					sendStanza( Jingle::createOrderErrorMessage(root).toString() );
				}else{
					if(handleSessionAccept(root)){
						sendStanza(Jingle::createReceiptMessage(root).toString() );
						state = ACTIVE;
						lastMessage = sessionAccept;
						emit accepted();
					}else{
						/*one or more transports is unwriteable*/
						sendStanza(Jingle::createTransportNotAcceptableMessage(root).toString() );
						//the other client should now send a session-terminate
					}
				}

			}else if(action == "session-initiate"){

				if(state != PENDING){
					sendStanza( Jingle::createOrderErrorMessage(root).toString() );
				}else{
					initiator = root.attribute("from");
					amIInitiator = false;
					responder = myself().full();
					sid = jingleElement.attribute("sid");
					checkNewContent(root);
				}

			}else if(action == "session-terminate"){

				state = ENDED;
				sendStanza(Jingle::createReceiptMessage(root).toString() );
				emit terminated();

			}else if(action == "session-info"){

				if(jingleElement.hasChildNodes()){
					recieveSessionInfo(root);
				}else{
					
				}
				sendStanza(Jingle::createReceiptMessage(root).toString() );

			}else if(action == "content-add"){

				//out of order if still pending or if session being modified
				if(state != ACTIVE  || ( isModifying() && amIInitiator ) ){
					sendStanza(Jingle::createOrderErrorMessage(root).toString() );
				}else{
					//sendStanza(Jingle::createReceiptMessage(root).toString() );
					checkNewContent(root);
				}

			}else if(action == "content-remove"){
				if ( isModifying() && amIInitiator ){
					sendStanza( Jingle::createOrderErrorMessage(root).toString() );
				}else{
					removeContent(root); //remove content and send accept or terminate
				}

			}else if(action == "content-modify"){

				if ( isModifying() && amIInitiator ){
					sendStanza( Jingle::createOrderErrorMessage(root).toString() );
				}else{
					sendStanza( Jingle::createReceiptMessage(root).toString() );
					checkContent(root);
				}

			}else if(action == "content-accept"){

				sendStanza(Jingle::createReceiptMessage(root).toString() );
				updateContent(root);

			}else if(action == "transport-info"){
				//this one should be fine for multiple content types, since only one candidate should
				//be sent at once
				if( addRemoteCandidate(jingleElement.elementsByTagName("content").item(0).toElement() ) ){
					sendStanza(Jingle::createReceiptMessage(root).toString() );
				}else{
					//malformed candidate
				}
			}
		}
	}else if(type == "result"){
		if(root.attribute("id")==transactionID) acknowledged = true;
	}else if(type == "get"){

	}

}

void JingleSession::handleError(QDomElement errorElement)
{
	if ( lastMessage == sessionInitiate ){
		/* possible errors:
			service-unavailable
			redirect
			feature-not-implemented and unsupported-transport
			feature-not-implemented and unsupported-content
			bad-request (malformed)
		*/
		if ( !( errorElement.elementsByTagName("service-unavailable").isEmpty() ) ){
			//no jingle support, or jingle connection not permitted
			state = ENDED;
			emit terminated();
		}else if( !( errorElement.elementsByTagName("redirect").isEmpty() ) ){
			//change the TO
			peers()[0] = XMPP::Jid( errorElement.firstChildElement().text() );
			responder = errorElement.firstChildElement().text();
			start(); //NOTE this emits sessionStarted() again
		}else if( !( errorElement.elementsByTagName("bad-request").isEmpty() ) ){
			//syntax error.
			kDebug(JABBER_DEBUG_GLOBAL) << "our session-initiate syntax stanza was malformed" << endl;
		}else if( !(errorElement.elementsByTagName("unsupported-transport").isEmpty() ) ){
			state = ENDED;
			emit terminated();
		}else if( !(errorElement.elementsByTagName("unsupported-content").isEmpty() ) ){
			state = ENDED;
			emit terminated();
		}
	}else if( lastMessage == sessionAccept) {
		
	}else if( isModifying() && !amIInitiator ){
		if (!(errorElement.elementsByTagName("unexpected-request").isEmpty() ) ){
			//their modification takes precedence.
		}
	}else if( !(errorElement.elementsByTagName("unsupported-transport").isEmpty() ) ){
		//adding content failed
	}else if( !(errorElement.elementsByTagName("unsupported-content").isEmpty() ) ){
		//adding content failed
	}

}

void JingleSession::sendTransportCandidates(int contentIndex)
{
	//NOTE instead of a foreach, this could pass the ContentType and index of the ConnectionCandidate
	/*foreach (JingleConnectionCandidate candidate, types[contentIndex].candidates){
		lastMessage = transportInfo;
		transactionID = account()->client()->client()->genUniqueId(); //TODO make this a queue
		sendStanza( Jingle::createTransportCandidateMessage(
			myself().full(), peers().first().full(), initiator, responder, transactionID, sid,
			types[contentIndex].name, types[contentIndex].creator, types[contentIndex].transportNS,
			&candidate).toString() ) ;
	}*/

	for(int i = 0; i < types[contentIndex].candidates.size(); i++){
		lastMessage = transportInfo;
		transactionID = account()->client()->client()->genUniqueId(); //TODO make this a queue
		sendStanza( Jingle::createTransportCandidateMessage(
			myself().full(), peers().first().full(), initiator, responder, transactionID, sid,
			types[contentIndex].name, types[contentIndex].creator, types[contentIndex].transportNS,
			&(types[contentIndex].candidates[i])).toString() ) ;
	}
}

void JingleSession::removeContent(QDomElement stanza)
{
	QDomElement content = stanza.firstChildElement().firstChildElement(); //NOTE need to check for nulls (bad requests)
	QString name = content.attribute("name");
	bool rightContent = false;
	int i = -1;

	do{
		i++;
		if(types[i].name == name) rightContent = true;
	}while( i < types.size() && !rightContent);

	if( types[i].connection != NULL) delete types[i].connection;
	if( types[i].writeConnection != NULL) delete types[i].writeConnection;
	
	types.removeAt(i);

	//send reciept here, in case of termination
	sendStanza(Jingle::createReceiptMessage(stanza).toString() );

	if(types.empty()){
		terminate();
	}//else{
		
// 		transactionID = GEN_ID;
// 		lastMessage = contentAccept;
// 		sendStanza(Jingle::createContentAcceptMessage(myself().full(), peers().first().full(), initiator, responder, transactionID, sid, types).toString() );
	//}
}

#include "jinglesession.moc"
