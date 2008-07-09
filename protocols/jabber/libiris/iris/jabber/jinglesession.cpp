#include <QString>

#include "jinglesession.h"

using namespace XMPP;

class JingleSession::Private
{
public:
	Jid to;
	QList<JingleContent> contents;
	Task *rootTask;
	QString sid;
	QString initiator;
	JT_JingleSession *jt;
	State state;
};

JingleSession::JingleSession(Task *t, const Jid &j)
: d(new Private)
{
	d->to = j;
	d->rootTask = t;
	d->state = Pending;
}

JingleSession::~JingleSession()
{
	delete d;
}

void JingleSession::addContent(const JingleContent& c)
{
	d->contents << c;
}

Jid JingleSession::to() const
{
	return d->to;
}

QList<JingleContent> JingleSession::contents() const
{
	return d->contents;
}

void JingleSession::start()
{
	/* FIXME:
	 * 	Use a JT_JingleAction instead.
	 */
	d->jt = new JT_JingleSession(d->rootTask);
	d->jt->start(this);
	d->jt->go();
	startNegotiation();
}

void JingleSession::acceptSession()
{
	qDebug() << "Starting payloads and transport negotiation.";

/*
 * That won't be done here : the negotiation must be done while ringing for
 * example and even as soon as the session-initiate is received.
 */
	

	/*
	 * Here, it becomes a bit complicated (well, not so much) :
	 * 	If we are in pending state, we must start negotiation but *NOT* send a session-accept action now !
	 * 	Also, at this point, JingleSession MUST know what payloads we wanna use for what content
	 * 	and if the transport is supported. 
	 * 	Also, This should be known from the very start of the application : what transport are supported and
	 * 	what payloads to use for a particular content (RTP/AVP,...).
	 * 	That is done in the JingleSessionManager which knows what transports and contents are supported.
	 * 	Access it by rootTask->client()->jingleSessionManager()
	 */
	// RAW UDP :
	//	Why should I start sending data as soon as I received the session-initiate action ?
	//		That does not make sense with Audio via RTP specification (I won't "immediately negotiate connectivity over the ICE transport by exchanging XML-formatted candidate transports for the channel" as I first negotiate contents...)
	//	Seeing the specification (XEP-0177), I can't tell the responder which contents I accept and which I don't.
	//
	
	// ICE-UDP :
	//	Why should I start sending data as soon as I received the session-initiate action ?
	//	Seeing the specification (XEP-0176), I can't tell the responder which contents I accept and which I don't.
	
	/*Maybe yes : do both things at the same time...*/

}

void JingleSession::acceptContent()
{
	/* FIXME:
	 * 	* The JT_JingleAction task should be in Private.
	 * 	* More than 1 JT_JingleAction could be present at the same time.
	 */
	JT_JingleAction *tAction = new JT_JingleAction(d->rootTask);
	tAction->setSession(this);
	tAction->contentAccept();
}

void JingleSession::removeContent(const QString& c) //FIXME:Is it possible to have more than 1 content in a remove-content action ?
{
/*
 * From Jingle Specification : 
 * A client MUST NOT return an error upon receipt of a 'content-remove' action for a content
 * type that is received after a 'content-remove' action has been sent but before the action
 * has been acknowledged by the peer. 
 * 
 * If the content-remove results in zero content types for the session, the entity that receives
 * the content-remove SHOULD send a session-terminate action to the other party (since a session
 * with no content types is void).
 */
	bool found = false;
	//if (d->state != Active) /*FIXME:Should be if (d->state == Pending)*/
	{
		// FIXME:whatever the state is, the same thing will be done here...
		// Checking if that content exists.
		int i = 0;
		for ( ; i < contents().count(); i++)
		{
			if (contents()[i].name() == c)
			{
				found = true;
				break;
			}
		}
		if (!found)
		{
			qDebug() << "This content does not exists for this session (" << c << ")";
			return;
		}
		d->contents.removeAt(i); //Or do it in the slotRemoveAcked() ??
		//FIXME:We must use a JT_JingleAction here, d->jt might be deleted.
		//	In the future, everything will be started with a JT_JingleAction
		//	as everything is an Action in Jingle.
		JT_JingleAction *rAction = new JT_JingleAction(d->rootTask);
		connect(rAction, SIGNAL(acked()), this, SLOT(slotRemoveAcked()));
		rAction->setSession(this);
		rAction->removeContent(c);
	}
}

void JingleSession::slotRemoveAcked()
{
	JT_JingleAction *rAction = (JT_JingleAction*) sender();
	if (rAction != 0)
		delete rAction;
	else
		return;
	if (d->state == Pending)
		acceptSession();
	//else if (d->state == Active)
	//	emit stopSending(d->contentToRemove);
}

void JingleSession::end(const QString& cond, const QString& text) //DEPRECATED, Use JingleSession::terminate(Reason)
{

}

void JingleSession::ring()
{
	JT_JingleAction *jtRing = new JT_JingleAction(d->rootTask);
	jtRing->setSession(this);
	jtRing->ringing();
}

void JingleSession::setSid(const QString& s)
{
	d->sid = s;
}

QString JingleSession::sid() const
{
	return d->sid;
}

void JingleSession::setInitiator(const QString& init)
{
	d->initiator = init;
}

void JingleSession::addContent(const QDomElement& content)
{
	JingleContent c;
	c.fromElement(content);
	d->contents << c;
}

void JingleSession::terminate(JingleSession::Reason r)
{
/*
 * FIXME: Reason should be a class so we can add informations like the new
 * 	  sid for an alternative-session condition.
 */
	JT_JingleAction *tAction = new JT_JingleAction(d->rootTask);
	tAction->setSession(this);
	tAction->terminate(r);

	connect(tAction, SIGNAL(finished()), this, SLOT(slotSessTerminated()));
	/* TODO:
	 * 	tAction will send a signal when aknowledged by the recipient.
	 * 	it MUST be deleted at that moment.
	 */
}

QString JingleSession::initiator() const
{
	return d->initiator;
}

void JingleSession::startNegotiation()
{
	/* TODO:
	 * 	For each transport in each contents, I must send all possible candidates.
	 * 	Those candidates can be found without the help of the application.
	 */
	for (int i = 0; i < d->contents.count(); i++)
	{
		for (int j = 0; j < d->contents[i].transports().count(); j++)
		{
			if (d->contents[i].transports()[j].attribute("name") == "ice-udp")
				sendIceUdpCandidates();
			else if (d->contents[i].transports()[j].attribute("name") == "raw-udp")
				sendRawUdpCandidates();
		}
	}
}

JingleContent *JingleSession::contentWithName(const QString& n)
{
	for (int i = 0; i < d->contents.count(); i++)
	{
		if (d->contents.at(i).name() == n)
			return &(d->contents[i]);
	}
	return 0;
}

void JingleSession::setTo(const Jid& to)
{
	d->to = to;
}

void JingleSession::sendIceUdpCandidates()
{
	qDebug() << "Sending ice-udp candidates";
	JT_JingleAction *cAction = new JT_JingleAction(d->rootTask)
	cAction->setSession(this);
	QDomDocument doc("");
	QDomElement candidate = doc.createElement();
	candidate.setAttribute("foo", "bar");
	cAction->sendCandidate(candidate);
	// --> Or better : sendTransportInfo(QDomElement transport);
}

void JingleSession::sendRawUdpCandidates()
{
	qDebug() << "Sending raw-udp candidates";
}

void JingleSession::slotSessTerminated()
{
	emit deleteMe();
}
