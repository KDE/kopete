#include jinglefoosession.h

#define JINGLE_NS "http://www.xmpp.org/extensions/xep-0166.html#ns"
#define JINGLE_FOO_NS "http://www.example.com/foo"
#define JINGLEFOOTRANSPORT "footransport"

#include <QtXml>
#include "jinglejabberxml.h"

//BEGIN JingleStateEnum
enum JingleStateEnum{
	PENDING,
	ACTIVE,
	ENDED
};
//END JingleStateEnum


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
	//responding on session-initiate means we accept the codecs, transport, etc.
	if (!first.isNull() && first.attribute("xmlns") == JINGLE_NS  && first.attribute("action") == "session-initiate") {
		QDomElement iq = createIQ(doc(), "result", e.attribute("from"), e.attribute("id"));
		send(iq);
		return true;
	}
	
	return false;
}
//END JingleIQResponder

JingleFooSession::JingleFooSession(JabberAccount *account, const JidList &peers)
 : JingleSession(account, peers)
{

	XMPP::Jid jid( account->client()->jid());
	state = JingleStateEnum::PENDING;
	types.push_back(JingleContentType("Foo","http://www.example.com/jingle/foo.html");

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
		//NOTE doesn't catch <error> messages
		if( !element.isNull() && element.attribute("xmlns") == JINGLE_NS) 
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

void JingleFooSession::processStanza(QDomDocument doc)
{
	QDomElement root = doc.documentElement();
	QString type = root.attribute("type");
	
	//error
	if( type == "error"){
		QDomElement errorElement = root.firstChildElement();
		if(errorElement != NULL){
			

		}

	}else if(type == "set"){
		QDomElement jingleElement = root.firstChildElement();
		if(jingleElement == NULL){

		}else{
			QString action = jingleElement.attribute("action");
			if(action == "session-accept"){

				if(state != JingleStateEnum::PENDING){
					send(createOrderErrorMessage(root));
				}else{state=JingleStateEnum::ACTIVE;
					emit accepted();
				}

			}else if(action == "session-initiate"){

				if(state != JingleStateEnum::PENDING){
					send(createOrderErrorMessage(root));
				}
				initiator = root.attribute("from");
				responder = jid->full();
				sid = jingleElement.attribute("sid");
				send(checkPayload(root));

			}else if(action == "session-terminate"){

				state = JinglsStateEnum::ENDED;
				emit terminated();

			}else if(action == "session-info"){

			}else if(action == "content-add"){

				if(state != JingeStateEnum::ACTIVE){
					send(createOrderErrorMessage(root));
				}

			}else if(action == "content-remove"){

				removeContent(root);

			}else if(action == "content-modify"){

			}else if(action == "content-accept"){

			}else if(action == "transport-info"){
				
			}
		}
	}else if(type == "result"){

	}else if(type == "get"){

	}

}

void JingleFooSession::accept()
{
	if(state == JingleStateEnum::PENDING){
		QDomElement accept = createAcceptMessage(jid->full(), peers()->first()->full(), &initiator, &responder,  account->client()->genUniqueId(),&sid,types);
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
		QDomElement decline = createTerminateMessage(jid->full(), peers()->first()->full(), &initiator, &responder, account->client()->genUniqueId(),&sid,"Declined");
		send(decline);
		state = JingleStateEnum::ENDED;
		emit declined();
	}
}

void JingleFooSession::terminate()
{
	if(state =! JingleStateEnum::ENDED)
	{
		QDomElement terminate = createTerminateMessage(jid->full(), peers()->first()->full(), &initiator, &responder, account->client()->genUniqueId(),&sid,"Bye");
		send(terminate);
		state = JingleStateEnum::ENDED;
		emit terminated();
	}
}

QDomElement checkPayload(QDomElement stanza)
{
	QDomElement content = stanza.firstChildElement().firstChildELement(); //NOTE need to check for nulls
	//NOTE assumes only one type of content.
	QString name = content.attribute("name");
	bool rightContent = false;
	for( int i =0; i< types.length(); i++){
		if(types[i].name == name) rightContent = true;
	}
	if(rightContent){
		QString transportNS = content.elementsByTagName("transport").item(0).attribute("xmlns");
		if(transportNS == JINGLEFOOTRANSPORT){
			return createReceiptMessage(stanza);
		}else{
			return createTransportErrorMessage(stanza);
		}
	}else{
		return createContentErrorMessage(stanza);
	}
}

/*
 * Removes designated content type.  If there are none left, closes the session.
 */
void removeContent(QDomElement stanza)
{
	QDomElement content = stanza.firstChildElement().firstChildELement(); //NOTE need to check for nulls
	//NOTE assumes only one type of content.
	QString name = content.attribute("name");
	bool rightContent = false;
	int i = -1;
	do{
		i++;
		if(types[i].name == name) rightContent = true;
	}while( i < types.length() && !rightContent
// 	for( int i =0; i< types.length(); i++){
// 		if(types[i].name == name) rightContent = true;
// 	}

	types.remove(i);
	if(types.empty()){
		terminate();
	}
}

#include "jinglefoosession.moc"
