
#include <QtDebug>

#include "jingletasks.h"
#include "protocol.h"
#include "xmpp_xmlcommon.h"
#include "jinglesession.h"

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

//----------------------
// JingleContent
//----------------------

class JingleContent::Private
{
public:
	QList<QDomElement> payloads;
	QStringList transports;
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

void JingleContent::addTransportNS(const QString& t)
{
	d->transports << t;
}

QList<QDomElement> JingleContent::payloadTypes() const
{
	return d->payloads;
}

QStringList JingleContent::transportNS() const
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
		QDomElement transport = doc.createElement("transport");
		transport.setAttribute("xmlns", d->transports.at(i));
		content.appendChild(transport);
	}

	return content;
}


//------------------------
// JT_PushJingleSession
//------------------------
JT_PushJingleSession::JT_PushJingleSession(Task *parent)
: Task(parent)
{

}

JT_PushJingleSession::~JT_PushJingleSession()
{

}

void JT_PushJingleSession::onGo()
{

}

bool JT_PushJingleSession::take(const QDomElement &x)
{

}


//------------------------
// JT_JingleSession
//------------------------
class JT_JingleSession::Private
{
public:
	State state;
	JingleSession *session;
	QList<JingleContent> contents;
};

JT_JingleSession::JT_JingleSession(Task *parent)
: Task(parent), d(new Private())
{
	d->state = Initiation;
}

JT_JingleSession::~JT_JingleSession()
{

}

void JT_JingleSession::start(JingleSession *s)
{
	d->session = s;
	d->contents = s->contents();
	
	qDebug() << id();
	iq = createIQ(doc(), "set", d->session->to().full(), id());
	iq.setAttribute("from", client()->jid().full());
	QDomElement jingle = doc()->createElement("jingle");
	jingle.setAttribute("xmlns", "urn:xmpp:tmp:jingle");
	jingle.setAttribute("action", "session-initiate");
	jingle.setAttribute("initiator", client()->jid().full());
	sid = "a" + genSid();
	qDebug() << sid;
	jingle.setAttribute("sid", sid);

	for (int i = 0; i < d->contents.count(); i++)
	{
		jingle.appendChild(d->contents[i].contentElement());
	}

	/*QDomElement content = doc()->createElement("content");
	content.setAttribute("creator", "initiator");
	content.setAttribute("name", "audio-content");

	QDomElement description = doc()->createElement("description");
	description.setAttribute("xmlns", "urn:xmpp:tmp:jingle:apps:audio-rtp");
	
	for (int i = 0; i < d->session->payloadTypes().count(); i++)
	{
		description.appendChild(d->session->payloadTypes().at(i));
	}

	content.appendChild(description);
	
	for (int i = 0; i < d->session->transportNS().count(); i++)
	{
		QDomElement transport = doc()->createElement("transport");
		transport.setAttribute("xmlns", d->session->transportNS().at(i));
		content.appendChild(transport);
	}
	jingle.appendChild(content);*/

	iq.appendChild(jingle);
}

void JT_JingleSession::onGo()
{
	send(iq);
}

bool JT_JingleSession::take(const QDomElement &x)
{
	//if (!iqVerify(&x, d->session->to(), id()))
	//	return false;
	if (!iqJingleVerify(&x, d->session->to(), id(), sid))
		return false;

	switch (d->state)
	{
	case Initiation :
		if (x.attribute("type") == "result")
			d->state = WaitContentAccept;
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
		break;
	case WaitContentAccept :
		
		break;
	default :
		break;
	}

	return true;
}

