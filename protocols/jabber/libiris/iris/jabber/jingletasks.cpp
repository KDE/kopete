
#include <QtDebug>
#include <stdio.h>

#include "jingletasks.h"
#include "protocol.h"
#include "xmpp_xmlcommon.h"

using namespace XMPP;

QString genSid()
{
	QString s;
	int id_seed = rand() % 0xffff;
	s.sprintf("a%x", id_seed);
	return s;
}

bool iqJingleVerify(const QDomElement *x, const Jid& from, const QString& id, const QString& sid)
{
	if (x->attribute("from") != from.full())
		return false;
	if (x->attribute("id") != id)
	{
		// If the id has changed, we must check if the sid is the same.
		QDomElement jingle = x->firstChildElement();
		if (jingle.tagName() != "jingle" || jingle.attribute("sid") != sid)
			return false;
	}
	return true;
}

JingleSession::JingleAction jingleAction(const QDomElement& x)
{
	QString action = x.firstChildElement().attribute("action");
	qDebug() << x.tagName();
	qDebug() << x.firstChildElement().attribute("action");
	qDebug() << "There are " << x.firstChildElement().attributes().count() << " attributes.";
	qDebug() << x.firstChildElement().firstChildElement().tagName();
	qDebug() << action;
	if (action == "session-initiate")
		return JingleSession::SessionInitiate;
	else if (action == "session-terminate")
		return JingleSession::SessionTerminate;
	else if (action == "session-accept")
		return JingleSession::SessionAccept;
	else if (action == "session-info")
		return JingleSession::SessionInfo;
	else if (action == "content-add")
		return JingleSession::ContentAdd;
	else if (action == "content-remove")
		return JingleSession::ContentRemove;
	else if (action == "content-modify")
		return JingleSession::ContentModify;
	else if (action == "content-replace")
		return JingleSession::ContentReplace;
	else if (action == "content-accept")
		return JingleSession::ContentAccept;
	else if (action == "transport-info")
		return JingleSession::TransportInfo;
	else
		return JingleSession::NoAction;
}


//----------------------
// JingleContent
//----------------------

class JingleContent::Private
{
public:
	QList<QDomElement> payloads;
	//FIXME:Only 1 transport per content.
	QList<QDomElement> transports;
	QList<QDomElement> candidates;
	QString creator;
	QString name;
	QString profile;
	QString descriptionNS;
};

JingleContent::JingleContent()
: d(new Private())
{

}

JingleContent::~JingleContent()
{

}

void JingleContent::addPayloadType(const QDomElement& pl)
{
	d->payloads << pl;
}

void JingleContent::addTransportNS(const QDomElement& t)
{
	d->transports << t;
}

QList<QDomElement> JingleContent::payloadTypes() const
{
	return d->payloads;
}

QList<QDomElement> JingleContent::transports() const
{
	return d->transports;
}

void JingleContent::setCreator(const QString& c)
{
	d->creator = c;
}

void JingleContent::setName(const QString& n)
{
	d->name = n;
}

void JingleContent::setDesriptionNS(const QString& desc)
{
	d->descriptionNS = desc;
}

void JingleContent::setProfile(const QString& p)
{
	d->profile = p;
}

void JingleContent::fromElement(const QDomElement& e)
{
	if (e.tagName() != "content")
		return;
	d->creator = e.attribute("creator");
	d->name = e.attribute("name");
	//d->sender = e.attribute("sender");
	QDomElement desc = e.firstChildElement();
	d->descriptionNS = desc.attribute("xmlns");
	d->profile = desc.attribute("profile");
	QDomElement payload = desc.firstChildElement();
	while (!payload.isNull())
	{
		d->payloads << payload;
		payload = payload.nextSiblingElement();
	}
	QDomElement transport = desc.nextSiblingElement();

	while (!transport.isNull())
	{
		d->transports << transport;
		transport = transport.nextSiblingElement();
	}
}

QDomElement JingleContent::contentElement()
{
	// Create the QDomElement which has to be returned.
	QDomDocument doc("");
	
	QDomElement content = doc.createElement("content");
	content.setAttribute("creator", d->creator);
	content.setAttribute("name", d->name);
	content.setAttribute("sender", "both"); //Setting to default now, change it !
	
	QDomElement description = doc.createElement("description");
	description.setAttribute("xmlns", d->descriptionNS);
	description.setAttribute("profile", d->profile);

	for (int i = 0; i < d->payloads.count(); i++)
	{
		description.appendChild(d->payloads.at(i));
	}
	content.appendChild(description);
	for (int i = 0; i < d->transports.count(); i++)
	{
		content.appendChild(d->transports.at(i));
	}

	return content;
}

QString JingleContent::name() const
{
	return d->name;
}

QString JingleContent::descriptionNS() const
{
	return d->descriptionNS;
}

QString JingleContent::dataType()
{
	if (d->descriptionNS == "urn:xmpp:tmp:jingle:apps:audio-rtp")
		return "Audio";
	else if (d->descriptionNS == "urn:xmpp:tmp:jingle:apps:video-rtp")
		return "Video";
	else if (d->descriptionNS == "urn:xmpp:tmp:jingle:apps:file-transfer")
		return "File Transfer";
	else
		return "Unknown Namespace. Join us on #kopete on irc.freenode.org .";
}

void JingleContent::addTransportInfo(const QDomElement& e)
{
	// From Now on, I consider only 1 transport per content !.
	// TODO: ask.
	QDomElement transport = e.firstChildElement();
	if (transport.attribute("xmlns") == "urn:xmpp:tmp:jingle:transports:ice-udp")
	{
		if (d->transports[0].attribute("pwd") != transport.attribute("pwd"))
		{
			qDebug() << "Bad ICE Password !";
			return;
		}
		
		if (d->transports[0].attribute("ufrag") != transport.attribute("ufrag"))
		{
			qDebug() << "Bad ICE User Fragment !";
			return;
		}
		QDomElement child = transport.firstChildElement();
		//FIXME:Is it possible to have more than one candidate per transport-info ?
		//	See Thread "Jingle: multiple candidates per transport-info?" on xmpp-standards.
		if (child.tagName() == "candidate")
		{
			// Just adding the Xml Element.
			d->candidates << child;
		}
	}
}

QString JingleContent::iceUdpPassword()
{
	for (int i = 0; i < d->transports.count(); i++)
	{
		if (d->transports[i].attribute("xmlns") == "urn:xmpp:tmp:jingle:transports:ice-udp")
			return d->transports[i].attribute("pwd");
	}
	return "";
}

QString JingleContent::iceUdpUFrag()
{
	for (int i = 0; i < d->transports.count(); i++)
	{
		if (d->transports[i].attribute("xmlns") == "urn:xmpp:tmp:jingle:transports:ice-udp")
			return d->transports[i].attribute("ufrag");
	}
	return "";
}

//------------------------
// JT_PushJingleSession
//------------------------
//RECEIVES THE ACTIONS
class JT_PushJingleSession::Private
{
public:
	JingleSession *incomingSession;
	QList<JingleSession*> incomingSessions;
	QDomElement iq;
	QString id;
	Jid from;
};

JT_PushJingleSession::JT_PushJingleSession(Task *parent)
: Task(parent), d(new Private)
{
	printf("\n\nCreating the PushJingleSession task....\n\n");
}

JT_PushJingleSession::~JT_PushJingleSession()
{
	printf("Deleting the PushJingleSession task....\n");
}

void JT_PushJingleSession::onGo()
{
//	send(d->iq);
}

bool JT_PushJingleSession::take(const QDomElement &x)
{
	/*
	 * We take this stanza when it is a session-initiate stanza for sure.
	 * Now, 2 possibilities :
	 * 	* This task is used by the JingleSession to established the connection
	 * 	* A new JT_JingleSession is used by the JingleSession to established the connection
	 * I'd rather use the second one, see later...
	 */
	printf("JT_PushJingleSession::take\n");
	//printf("tagName : %s\n", x.firstChildElement().tagName().toLatin1().constData());
	if (x.firstChildElement().tagName() != "jingle")
		return false;
	
	if (x.attribute("type") == "error")
		jingleError(x);

	QStringList cName;
	QString sid = x.firstChildElement().attribute("sid");
	d->from = Jid(x.attribute("from"));
	QDomElement jingle;
	QDomElement content;
	switch(jingleAction(x))
	{
	case JingleSession::SessionInitiate :
		qDebug() << "New Incoming session : " << sid;
		d->id = x.attribute("id");
		ack();
		
		//Prepare the JingleSession instance.
		d->incomingSession = new JingleSession(parent(), Jid());
		d->incomingSession->setTo(x.attribute("from"));
		jingle = x.firstChildElement();
		d->incomingSession->setInitiator(jingle.attribute("initiator"));
		d->incomingSession->setSid(jingle.attribute("sid"));
		content = jingle.firstChildElement();
		while (!content.isNull())
		{
			if (content.tagName() == "content");
				d->incomingSession->addContent(content);

			content = content.nextSiblingElement();
		}
		
		d->incomingSessions << d->incomingSession;

		emit newSessionIncoming();
		 /* TODO : 
		  * 	Continue to negotiate the contents to use --> Done by the JT_JingleSession.
		  */
		break;
	case JingleSession::ContentRemove : 
		qDebug() << "Content remove for session " << sid;
		// Ack content-remove
		d->id = x.attribute("id");
		ack();
		
		content = x.firstChildElement().firstChildElement();
		while (!content.isNull())
		{
			cName << content.attribute("name");
			qDebug() << " * Remove : " << cName;
			content = content.nextSiblingElement();
		}
		emit removeContent(sid, cName);
		/*
		 * JingleSessionManager will receive this signal and then tell the right session
		 * (given the session ID) to remove the correct content
		 */
		
		/*if (d->state == WaitContentAccept)
		{
			d->state = StartNegotiation;
			 *
			 * Content has been removed, we can take it as a content-accept.
			 * Now, we stop ringing but the session should change it by itself depending
			 * on the state when receiving a content-remove
			 * After we acknowledge the responder that the content has been removed,
			 * we must start negotiate a candidate with him (depending if we use ICE-UDP or RAW-UDP)
			 * ADVICE: Begin with RAW-UDP, it is simpler.
			 *
		}*/
	case JingleSession::SessionInfo :
		qDebug() << "Session Info for session " << sid;
		// Ack session-info
		d->id = x.attribute("id");
		ack();
	case JingleSession::TransportInfo :
		qDebug() << "Transport Info for session " << sid;
		d->id = x.attribute("id");
		ack();
		
		emit transportInfo(x.firstChildElement());

		break;
	default:
		printf("There are some troubles with the Jingle Implementation. Be carefull that this is still low performances software.\n");
	}
	return true;
}

JingleSession *JT_PushJingleSession::takeNextIncomingSession()
{
	return d->incomingSessions.takeLast();
}

void JT_PushJingleSession::ack()
{
	d->iq = createIQ(doc(), "result", d->from.full(), d->id);
	send(d->iq);
}

void JT_PushJingleSession::jingleError(const QDomElement& x)
{
	qDebug() << "There was an error from the responder. Not supported yet.";
}

//------------------------
// JT_JingleSession
//------------------------
//Used until the session is in ACTIVE state.
class JT_JingleSession::Private
{
public:
	State state;
	JingleSession *session;
	QList<JingleContent> contents;
	QDomElement iq;
	QString id;
};

JT_JingleSession::JT_JingleSession(Task *parent)
: Task(parent), d(new Private())
{
	d->state = Initiation;
	d->session->setSid("a" + genSid());
}

JT_JingleSession::~JT_JingleSession()
{

}

void JT_JingleSession::start(JingleSession *s)
{
	d->session = s;
	d->contents = s->contents();
	
	qDebug() << id();
	d->iq = createIQ(doc(), "set", d->session->to().full(), id());
	d->iq.setAttribute("from", client()->jid().full());
	QDomElement jingle = doc()->createElement("jingle");
	jingle.setAttribute("xmlns", "urn:xmpp:tmp:jingle");
	jingle.setAttribute("action", "session-initiate");
	jingle.setAttribute("initiator", client()->jid().full());
	qDebug() << d->session->sid();
	jingle.setAttribute("sid", d->session->sid());

	for (int i = 0; i < d->contents.count(); i++)
	{
		jingle.appendChild(d->contents[i].contentElement());
	}

	d->iq.appendChild(jingle);
}

void JT_JingleSession::onGo()
{
	//send(d->iq);
	//if (d->state == StartNegotiation)
	//{
		// Now we start negotiating a candidate, trying to connect, etc... using RAW-UDP or ICE-UDP.
		// WARNING : We should already have agreed on a transport method.
		// If the responder does not support the proposed transport, what should he answer ?
		//sendCandidate();
	//}
}

bool JT_JingleSession::take(const QDomElement &x)
{
	//return false; //Now, JT_PushJingleTask takes all incoming stanzas
	/* FIXME:
	 * JingleSession's methods used to end the session, remove content,...
	 * should have other names (wantToEnd(), wantToRemoveContent(), ...)
	 * because end(), removeContent(),.. are used by the application when
	 * the user wants to execute the action.
	 */
	/* FIXME:
	 * Another problem is the one of the Tasks : to stick to the Iris policy,
	 * a different instance should be used for each possible different id.
	 * As a consequence, JT_PushJingleTask takes all incoming stanzas and emits
	 * signals with the sid so the application know what session it is for.
	 * Also, a task which sends an action is destroyed as soon
	 * as the action is terminated (either success or failure) :
	 * 	* JT_JingleAction (sends and manages 1 action then is destroyed);
	 * 	* JT_JingleSession (start the session : negotiation,  all incoming
	 * 	  stanzas will be taken by the JT_PushJingleSession and then emitted
	 * 	  to the application.)
	 */
	if (!iqVerify(x, d->session->to(), id()))
		return false;

	QString condition;
	QString text;
/*	switch (jingleAction(x))
	{
	case JingleSession::ContentAccept :
		* FIXME:
		 * 	After Example 39 of XEP-0167, shoudn't the responder send a content-accept ?
		 * 	Maybe not, as soon as the responder asked to remove something, he accepts the proposed content without the removed element(s(?))
		 *
		//Ack content-accept
		d->id = x.attribute("id");
		//ack();
		
		// That's accepted, we should now start the stream... see later.
		// content-accept is used if no modifications are done to the stream.
		* QDomElement info = x.firstChildElement().firstChildElement();
		if ()*
		// State changes in function of the transport.
		//d->state = StartNegotiation;
		break;
	case JingleSession::ContentRemove : 
	case JingleSession::SessionTerminate :
		// Ack end of session
		d->id = x.attribute("id");
		//ack();
		
		d->session->end(condition, text);
		break;
	case JingleSession::SessionInfo :
		// Ack ringing
		d->id = x.attribute("id");
		//ack();
		
		if (x.firstChild().firstChildElement().tagName() == "ringing")
		{
			d->session->ringing();
		}
		break;
	case JingleSession::NoAction :
		if (d->state == Initiation)
		{
			if (x.attribute("type") == "result")
			{
				qDebug() << "JT_JingleSession::take : switching to WaitContentAccept state.";
				d->state = WaitContentAccept;
			}
			else if (x.attribute("type") == "error")
			{
				QDomElement errorElem = x.firstChildElement();
				if (errorElem.attribute("type") == "cancel")
				{
					if (errorElem.firstChildElement().tagName() == "service-unavailable")
						emit error(ServiceUnavailable);
					if (errorElem.firstChildElement().tagName() == "redirect")
					{
						redirectTo = errorElem.firstChild().firstChild().toText().data();
						emit error(Redirect);
					}
					if (errorElem.firstChildElement().tagName() == "bad-request")
						emit error(BadRequest);
				}
				else if (errorElem.attribute("type") == "wait")
				{
					if (errorElem.firstChildElement().tagName() == "resource-constraint")
						emit error(ResourceConstraint);
				}
			}
		
		}
		break;
	default :
		break;
	}*/

	return true;
}

//-----------------------
// JT_JingleAction
//-----------------------

class JT_JingleAction::Private
{
public :
	JingleSession *session;
	QDomElement iq;
	QString sid;
	Jid to;
};

JT_JingleAction::JT_JingleAction(Task *parent)
: Task(parent), d(new Private())
{
	d->session = 0;
}

JT_JingleAction::~JT_JingleAction()
{

}

void JT_JingleAction::setSession(JingleSession *sess)
{
	d->session = sess;
}

void JT_JingleAction::contentAccept()
{
	if (d->session == 0)
	{
		qDebug() << "d->session is NULL, did you set it calling JT_JingleAction::setSession() ?";
		return;
	}

	qDebug() << "Sending the content-accept to : " << d->session->to().full();
	
	d->iq = createIQ(doc(), "set", d->session->to().full(), id());
	d->iq.setAttribute("from", client()->jid().full());
	
	QDomElement jingle = doc()->createElement("jingle");
	jingle.setAttribute("xmlns", "urn:xmpp:tmp:jingle");
	jingle.setAttribute("action", "content-accept");
	jingle.setAttribute("initiator", d->session->initiator());
	jingle.setAttribute("sid", d->session->sid());

	d->iq.appendChild(jingle);
	send(d->iq);
}

void JT_JingleAction::ringing()
{
	if (d->session == 0)
	{
		qDebug() << "d->session is NULL, did you set it calling JT_JingleAction::setSession() ?";
		return;
	}

	qDebug() << "Sending the session-info (ringing) to : " << d->session->to().full();
	
	d->iq = createIQ(doc(), "set", d->session->to().full(), id());
	d->iq.setAttribute("from", client()->jid().full());
	
	QDomElement jingle = doc()->createElement("jingle");
	jingle.setAttribute("xmlns", "urn:xmpp:tmp:jingle");
	jingle.setAttribute("action", "session-info");
	jingle.setAttribute("initiator", d->session->initiator());
	jingle.setAttribute("sid", d->session->sid());
	
	QDomElement ring = doc()->createElement("ringing");
	ring.setAttribute("xmlns", "urn:xmpp:tmp:jingle:apps:audio-rtp:info");

	jingle.appendChild(ring);
	d->iq.appendChild(jingle);

	send(d->iq);
}

void JT_JingleAction::terminate(int r)
{
	if (d->session == 0)
	{
		qDebug() << "d->session is NULL, did you set it calling JT_JingleAction::setSession() ?";
		return;
	}
	qDebug() << "Sending the session-terminate to : " << d->session->to().full();
	
	d->iq = createIQ(doc(), "set", d->session->to().full(), id());
	d->iq.setAttribute("from", client()->jid().full());
	
	QDomElement jingle = doc()->createElement("jingle");
	jingle.setAttribute("xmlns", "urn:xmpp:tmp:jingle");
	jingle.setAttribute("action", "session-terminate");
	jingle.setAttribute("initiator", d->session->initiator());
	jingle.setAttribute("sid", d->session->sid());

	QDomElement reason = doc()->createElement("reason");
	QDomElement condition = doc()->createElement("condition");

	QDomElement rReason;
	switch(r)
	{
	case JingleSession::Decline :
		rReason = doc()->createElement("decline");
		break;
	default:
		rReason = doc()->createElement("unknown");
	}

	d->iq.appendChild(jingle);
	jingle.appendChild(reason);
	reason.appendChild(condition);
	condition.appendChild(rReason);
	send(d->iq);
}

void JT_JingleAction::removeContent(const QString& c)
{
	// ----------------------------
	if (d->session == 0)
	{
		qDebug() << "d->session is NULL, did you set it calling JT_JingleAction::setSession() ?";
		return;
	}
	qDebug() << "Sending the session-terminate to : " << d->session->to().full();
	
	d->iq = createIQ(doc(), "set", d->session->to().full(), id());
	d->iq.setAttribute("from", client()->jid().full());
	
	QDomElement jingle = doc()->createElement("jingle");
	jingle.setAttribute("xmlns", "urn:xmpp:tmp:jingle");
	jingle.setAttribute("action", "content-remove");
	jingle.setAttribute("initiator", d->session->initiator());
	jingle.setAttribute("sid", d->session->sid());
	//---------This par should be in another method (createJingleIQ(...))
	
	QDomElement content = doc()->createElement("content");
	content.setAttribute("name", c);
	//FIXME:MUST the 'creator' tag be there ?
	
	jingle.appendChild(content);
	d->iq.appendChild(jingle);

	send(d->iq);
}

bool JT_JingleAction::take(const QDomElement &x)
{
	if (!iqVerify(x, d->session->to().full(), id()))
		return false;
	
	emit finished();
	return true;
}

void JT_JingleAction::onGo()
{
//	send(d->iq);
}

