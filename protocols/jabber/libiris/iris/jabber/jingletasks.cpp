
#include <QtDebug>
#include <stdio.h>

#include "jingletasks.h"
#include "protocol.h"
#include "xmpp_xmlcommon.h"

using namespace XMPP;


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
	QDomElement transport;
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

void JingleContent::setTransport(const QDomElement& t)
{
	d->transport = t;
}

QList<QDomElement> JingleContent::payloadTypes() const
{
	return d->payloads;
}

QDomElement JingleContent::transport() const
{
	return d->transport;
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
	// FIXME:tag order may not always be the same !!!
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
	d->transport = transport;
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
	content.appendChild(d->transport);

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
	QDomElement transport = e.firstChildElement();
	if (transport.attribute("xmlns") == "urn:xmpp:tmp:jingle:transports:ice-udp")
	{
		if (d->transport.attribute("pwd") != transport.attribute("pwd"))
		{
			qDebug() << "Bad ICE Password !";
			return;
		}
		
		if (d->transport.attribute("ufrag") != transport.attribute("ufrag"))
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
	if (d->transport.attribute("xmlns") == "urn:xmpp:tmp:jingle:transports:ice-udp")
		return d->transport.attribute("pwd");
	return "";
}

QString JingleContent::iceUdpUFrag()
{
	if (d->transport.attribute("xmlns") == "urn:xmpp:tmp:jingle:transports:ice-udp")
		return d->transport.attribute("ufrag");
	return "";
}

//------------------------
// JT_PushJingleAction
//------------------------
//RECEIVES THE ACTIONS
class JT_PushJingleAction::Private
{
public:
	JingleSession *incomingSession;
	QList<JingleSession*> incomingSessions;
	QDomElement iq;
	QString id;
	Jid from;
};

JT_PushJingleAction::JT_PushJingleAction(Task *parent)
: Task(parent), d(new Private)
{
	qDebug() << "Creating the PushJingleSession task....";
}

JT_PushJingleAction::~JT_PushJingleAction()
{
	qDebug() << "Deleting the PushJingleSession task....";
}

void JT_PushJingleAction::onGo()
{
//	send(d->iq);
}

bool JT_PushJingleAction::take(const QDomElement &x)
{
	/*
	 * We take this stanza when it is a session-initiate stanza for sure.
	 * Now, 2 possibilities :
	 * 	* This task is used by the JingleSession to established the connection
	 * 	* A new JT_JingleSession is used by the JingleSession to established the connection
	 * I'd rather use the second one, see later...
	 */
	qDebug() << "JT_PushJingleAction::take\n";
	// qDebug() << "tagName : %s\n", x.firstChildElement().tagName().toLatin1().constData();
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
		qDebug() << "There are some troubles with the Jingle Implementation. Be carefull that this is still low performances software.";
	}
	return true;
}

JingleSession *JT_PushJingleAction::takeNextIncomingSession()
{
	return d->incomingSessions.takeLast();
}

void JT_PushJingleAction::ack()
{
	d->iq = createIQ(doc(), "result", d->from.full(), d->id);
	send(d->iq);
}

void JT_PushJingleAction::jingleError(const QDomElement& x)
{
	qDebug() << "There was an error from the responder. Not supported yet.";
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

void JT_JingleAction::initiate()
{
	qDebug() << id();
	d->iq = createIQ(doc(), "set", d->session->to().full(), id());
	d->iq.setAttribute("from", client()->jid().full());
	QDomElement jingle = doc()->createElement("jingle");
	jingle.setAttribute("xmlns", "urn:xmpp:tmp:jingle");
	jingle.setAttribute("action", "session-initiate");
	jingle.setAttribute("initiator", client()->jid().full());
	qDebug() << d->session->sid();
	jingle.setAttribute("sid", d->session->sid());

	for (int i = 0; i < d->session->contents().count(); i++)
	{
		jingle.appendChild(d->session->contents()[i].contentElement());
	}

	d->iq.appendChild(jingle);
	send(d->iq);
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

void JT_JingleAction::removeContents(const QStringList& c)
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
	
	for (int i = 0; i < c.count(); i++)
	{
		QDomElement content = doc()->createElement("content");
		content.setAttribute("name", c[i]);
		jingle.appendChild(content);
	}
	//FIXME:MUST the 'creator' tag be there ?
	
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

