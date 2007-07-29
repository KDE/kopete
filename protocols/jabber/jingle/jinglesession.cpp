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
}

JingleSession::~JingleSession()
{
	kDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << endl;
	delete d;
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
			

		}

	}else if(type == "set"){
		QDomElement jingleElement = root.firstChildElement();
		if( jingleElement.isNull() ){

		}else{
			QString action = jingleElement.attribute("action");
			if(action == "session-accept"){

				if(state != PENDING){
					sendStanza( Jingle::createOrderErrorMessage(root).toString() );
				}else{
					connection = transport()->createCandidateFromElement(
						jingleElement.firstChildElement().firstChildElement() );
					if( connection.isUseful() ){
						sendStanza(Jingle::createReceiptMessage(root).toString() );
						state = ACTIVE;
						emit accepted();
					}else{
						sendStanza(Jingle::createTransportNotAcceptableMessage(root).toString() );
						//the other client should now send a session-terminate
					}
				}

			}else if(action == "session-initiate"){

				if(state != PENDING){
					sendStanza( Jingle::createOrderErrorMessage(root).toString() );
				}else{
					initiator = root.attribute("from");
					responder = myself().full();
					sid = jingleElement.attribute("sid");
					sendStanza( checkPayload(root).toString() );
				}

			}else if(action == "session-terminate"){

				state = ENDED;
				sendStanza(Jingle::createReceiptMessage(root).toString() );
				emit terminated();

			}else if(action == "session-info"){

				sendStanza(Jingle::createReceiptMessage(root).toString() );

			}else if(action == "content-add"){

				if(state != ACTIVE){
					sendStanza(Jingle::createOrderErrorMessage(root).toString() );
				}else{
					sendStanza(Jingle::createReceiptMessage(root).toString() );
				}

			}else if(action == "content-remove"){

				removeContent(root);
				sendStanza(Jingle::createReceiptMessage(root).toString() );

			}else if(action == "content-modify"){

				sendStanza(Jingle::createReceiptMessage(root).toString() );

			}else if(action == "content-accept"){

				sendStanza(Jingle::createReceiptMessage(root).toString() );

			}else if(action == "transport-info"){

//				remoteCandidates.push_back(fooTransport.createCandidateFromElement(
//					jingleElement.firstChildElement().firstChildElement()));
				//this one should be fine for multiple content types, since only one candidate should
				//be sent at once
				if( addRemoteCandidate(jingleElement.elementsByTagName("transport").item(0).toElement() ) ){
					sendStanza(Jingle::createReceiptMessage(root).toString() );
				}else{
					//malformed candidate
				}
			}
		}
	}else if(type == "result"){

	}else if(type == "get"){

	}

}

#include "jinglesession.moc"
