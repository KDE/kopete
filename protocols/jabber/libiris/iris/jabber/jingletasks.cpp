/*
 * jingletasks.cpp - Tasks for the Jingle specification.
 * Copyright (C) 2008 - Detlev Casanova <detlev.casanova@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <QtDebug>
#include <QNetworkInterface>
#include <QUdpSocket>
#include <stdio.h>

#include "jinglesessionmanager.h"

#include "jingletasks.h"
#include "protocol.h"
#include "xmpp_xmlcommon.h"

using namespace XMPP;

JingleSession::JingleAction jingleAction(const QDomElement& x)
{
	QString action = x.firstChildElement().attribute("action");
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
	else if (action == "transport-replace")
		return JingleSession::TransportReplace;
	else if (action == "transport-accept")
		return JingleSession::TransportAccept;
	else if (action == "transport-info")
		return JingleSession::TransportInfo;
	else
		return JingleSession::NoAction;
}



//------------------------
// JT_PushJingleAction
//------------------------
//RECEIVES THE ACTIONS

static JingleReason::Type stringToType(const QString& str)
{
	if (str == "busy")
	{
		return JingleReason::Busy;
	}
	else if (str == "decline")
	{
		return JingleReason::Decline;
	}
	else
	{
		return JingleReason::NoReason;
	}

}

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
	delete d;
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
	{
		jingleError(x);
		return true;
	}

	QStringList cName;
	QString sid = x.firstChildElement().attribute("sid");
	d->from = Jid(x.attribute("from"));
	QDomElement jingle;
	QDomElement content;
	QDomElement reason, e;
	QString condition;
	QString text;
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
		  * 	Continue to negotiate the contents to use --> Done by the JingleSession.
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
		break;
	case JingleSession::SessionInfo :
		qDebug() << "Session Info for session " << sid;
		// Ack session-info
		d->id = x.attribute("id");
		ack();
		
		emit sessionInfo(x.firstChildElement());
		break;
	case JingleSession::TransportInfo :
		qDebug() << "Transport Info for session " << sid;
		d->id = x.attribute("id");
		ack();
		
		emit transportInfo(x.firstChildElement());

		break;
	case JingleSession::SessionTerminate :
		qDebug() << "Transport Info for session " << sid;
		d->id = x.attribute("id");
		ack();
		
		reason = x.firstChildElement().firstChildElement();
		e = reason.firstChildElement();
		while(!e.isNull())
		{
			if (e.tagName() == "condition")
				condition = e.firstChildElement().tagName();
			else if (e.tagName() == "text")
				text = e.firstChildElement().toText().data();

			e = e.nextSiblingElement();
		}
		
		emit sessionTerminate(sid, JingleReason(stringToType(condition), text));

		break;
	case JingleSession::SessionAccept :
		qDebug() << "Transport Info for session " << sid;
		d->id = x.attribute("id");
		ack();

		emit sessionAccepted(x.firstChildElement());
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
	Q_UNUSED(x)
	//emit error(???);
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
	qDebug() << "Creating JT_JingleAction";
	d->session = 0;
}

JT_JingleAction::~JT_JingleAction()
{
	delete d;
}

void JT_JingleAction::setSession(JingleSession *sess)
{
	d->session = sess;
}

bool interfaceOrder(const QHostAddress& a1, const QHostAddress& a2)
{
	Q_UNUSED(a2)
	if ((a1 != QHostAddress::LocalHost) && (a1 != QHostAddress::Null))
		return true;
	return false;
}

void JT_JingleAction::initiate()
{
	qDebug() << id();
	d->iq = createIQ(doc(), "set", d->session->to().full(), id());
	d->iq.setAttribute("from", client()->jid().full());
	QDomElement jingle = doc()->createElement("jingle");
	jingle.setAttribute("xmlns", NS_JINGLE);
	jingle.setAttribute("action", "session-initiate");
	jingle.setAttribute("initiator", client()->jid().full());
	jingle.setAttribute("sid", d->session->sid());

	QString eip = client()->jingleSessionManager()->externalIP();

	for (int i = 0; i < d->session->contents().count(); i++)
	{
		QDomElement transport = d->session->contents()[i]->transport();
		//qDebug() << "Transport from the JingleContent is : " << client()->stream().xmlToString(transport, false);
		if (transport.attribute("xmlns") == NS_JINGLE_TRANSPORTS_RAW)
		{
			qDebug() << "Set raw-udp candidate for content" << i;
			QDomElement candidate = doc()->createElement("candidate");
			QString ip;

			//Trying to get the address with the most chances to succeed.
			if (eip != "") //does not seem to work...
			{
				ip = eip;
			}
			else
			{
				QNetworkInterface *interface = new QNetworkInterface();
				QList<QHostAddress> ips = interface->allAddresses();
				qSort(ips.begin(), ips.end(), interfaceOrder);
	
				if (ips.count() == 0)
				{
					qDebug() << "No Internet address found. Are you connected ?";
					//emit error(NoNetwork);
					return;
				}
				ip = ips[0].toString();
			}
			candidate.setAttribute("ip", ip); // ips[0] is not 127.0.0.1 if there is other adresses.
			int port = client()->jingleSessionManager()->nextRawUdpPort();
			//qDebug() << "Port =" << port;
			//qDebug() << "Port =" << QString("%1").arg(port);
			candidate.setAttribute("port", QString("%1").arg(port));
			candidate.setAttribute("generation", "0"); // FIXME:I don't know yet what it is.
			transport.appendChild(candidate);
			d->session->contents()[i]->bind(QHostAddress(ip), port);
			//qDebug() << client()->stream().xmlToString(transport, false);
		}
		else if (transport.attribute("xmlns") == NS_JINGLE_TRANSPORTS_ICE)
		{
			//TODO:implement me.
		}
		//d->session->contents()[i]->setTransport(transport);
		jingle.appendChild(d->session->contents()[i]->contentElement());
	}

	d->iq.appendChild(jingle);
	//send(d->iq);
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
	jingle.setAttribute("xmlns", NS_JINGLE);
	jingle.setAttribute("action", "content-accept");
	jingle.setAttribute("initiator", d->session->initiator());
	jingle.setAttribute("sid", d->session->sid());

	d->iq.appendChild(jingle);
	//send(d->iq);
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
	jingle.setAttribute("xmlns", NS_JINGLE);
	jingle.setAttribute("action", "session-info");
	jingle.setAttribute("initiator", d->session->initiator());
	jingle.setAttribute("sid", d->session->sid());
	
	QDomElement ring = doc()->createElement("ringing");
	ring.setAttribute("xmlns", "urn:xmpp:tmp:jingle:apps:audio-rtp:info");

	jingle.appendChild(ring);
	d->iq.appendChild(jingle);

	//send(d->iq);
}

void JT_JingleAction::terminate(const JingleReason& r)
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
	jingle.setAttribute("xmlns", NS_JINGLE);
	jingle.setAttribute("action", "session-terminate");
	jingle.setAttribute("initiator", d->session->initiator());
	jingle.setAttribute("sid", d->session->sid());

	QDomElement reason = doc()->createElement("reason");
	QDomElement condition = doc()->createElement("condition");

	QDomElement rReason;
	switch(r.type())
	{
	case JingleReason::Decline :
		rReason = doc()->createElement("decline");
		break;
	case JingleReason::NoReason :
		rReason = doc()->createElement("no-error");
		break;
	default:
		rReason = doc()->createElement("unknown");
	}

	d->iq.appendChild(jingle);
	jingle.appendChild(reason);
	reason.appendChild(condition);
	condition.appendChild(rReason);
	//send(d->iq);
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
	jingle.setAttribute("xmlns", NS_JINGLE);
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

	//send(d->iq);
}

void JT_JingleAction::transportInfo(JingleContent *c)
{
	QDomElement e = c->transport();
	// ----------------------------
	if (d->session == 0)
	{
		qDebug() << "d->session is NULL, did you set it calling JT_JingleAction::setSession() ?";
		return;
	}
	qDebug() << "Sending the transport-info to : " << d->session->to().full();

	d->iq = createIQ(doc(), "set", d->session->to().full(), id());
	d->iq.setAttribute("from", client()->jid().full());

	QDomElement jingle = doc()->createElement("jingle");
	jingle.setAttribute("xmlns", NS_JINGLE);
	jingle.setAttribute("action", "transport-info");
	jingle.setAttribute("initiator", d->session->initiator());
	jingle.setAttribute("sid", d->session->sid());
	//---------This part should be in another method (createJingleIQ(...))
	QString eip = client()->jingleSessionManager()->externalIP();

	if (e.attribute("xmlns") == NS_JINGLE_TRANSPORTS_RAW)
	{
		QDomElement content = doc()->createElement("content");
		content.setAttribute("name", c->name());
		content.setAttribute("creator", d->session->initiator() == d->session->to().full() ? d->session->to().full() : "initiator");

		QDomElement transport = doc()->createElement("transport");
		transport.setAttribute("xmlns", NS_JINGLE_TRANSPORTS_RAW);
		
		QDomElement candidate = doc()->createElement("candidate");
		QString ip;

		//Trying to get the address with the most chances to succeed.
		if (eip != "") //does not seem to work.
		{
			ip = eip;
		}
		else
		{
			QNetworkInterface *interface = new QNetworkInterface();
			QList<QHostAddress> ips = interface->allAddresses();
			qSort(ips.begin(), ips.end(), interfaceOrder);

			if (ips.count() == 0)
			{
				qDebug() << "No Internet address found. Are you connected ?";
				//emit error(NoNetwork);
				return;
			}
			ip = ips[0].toString();
		}
		candidate.setAttribute("ip", ip); // ips[0] is not 127.0.0.1 if there is other adresses.
		int port = client()->jingleSessionManager()->nextRawUdpPort();
		//qDebug() << "Port =" << port;
		//qDebug() << "Port =" << QString("%1").arg(port);
		candidate.setAttribute("port", QString("%1").arg(port));
		candidate.setAttribute("generation", "0"); // FIXME:I don't know yet what it is.
		transport.appendChild(candidate);
		content.appendChild(transport);
		jingle.appendChild(content);
		d->iq.appendChild(jingle);
		
		c->bind(QHostAddress(ip), port);
	}
	else if (e.attribute("xmlns") == NS_JINGLE_TRANSPORTS_ICE)
	{
		qDebug() << "ICE-UDP is not implemented yet.";
	}
	else
	{
		qDebug() << "Unsupported protocol (" << e.attribute("xmlns") << ")";
		return;
	}

	//send(d->iq);
}

void JT_JingleAction::trying(const JingleContent& c)
{
	QDomElement e = c.transport();
	// ----------------------------
	if (d->session == 0)
	{
		qDebug() << "d->session is NULL, did you set it calling JT_JingleAction::setSession() ?";
		return;
	}
	qDebug() << "Sending the session-info to : " << d->session->to().full();

	d->iq = createIQ(doc(), "set", d->session->to().full(), id());
	d->iq.setAttribute("from", client()->jid().full());

	QDomElement jingle = doc()->createElement("jingle");
	jingle.setAttribute("xmlns", NS_JINGLE);
	jingle.setAttribute("action", "session-info");
	jingle.setAttribute("initiator", d->session->initiator());
	jingle.setAttribute("sid", d->session->sid());
	//---------This par should be in another method (createJingleIQ(...))
	if (e.attribute("xmlns") == NS_JINGLE_TRANSPORTS_RAW)
	{
		QDomElement trying = doc()->createElement("trying");
		trying.setAttribute("xmlns", "urn:xmpp:tmp:jingle:transports:raw-udp:info");
		jingle.appendChild(trying);
		d->iq.appendChild(jingle);
	}
	else if (e.attribute("xmlns") == NS_JINGLE_TRANSPORTS_ICE)
	{
		qDebug() << "ICE-UDP is not implemented yet. (is trying message used with ICE-UDP ??? )";
	}
	else
	{
		qDebug() << "Unsupported protocol (" << e.attribute("xmlns") << ")";
		return;
	}

	//send(d->iq);
	
}

void JT_JingleAction::received()
{
	// ----------------------------
	if (d->session == 0)
	{
		qDebug() << "d->session is NULL, did you set it calling JT_JingleAction::setSession() ?";
		return;
	}
	qDebug() << "Sending the session-info to : " << d->session->to().full();

	d->iq = createIQ(doc(), "set", d->session->to().full(), id());
	d->iq.setAttribute("from", client()->jid().full());

	QDomElement jingle = doc()->createElement("jingle");
	jingle.setAttribute("xmlns", NS_JINGLE);
	jingle.setAttribute("action", "session-info");
	jingle.setAttribute("initiator", d->session->initiator());
	jingle.setAttribute("sid", d->session->sid());
	//---------This par should be in another method (createJingleIQ(...))
	QDomElement received = doc()->createElement("received");
	
	//That depends of the session content's transport.
	//Ice-udp does not need the "receive" informationnal message.
	received.setAttribute("xmlns", "urn:xmpp:tmp:jingle:transports:raw-udp:info");
	
	jingle.appendChild(received);
	d->iq.appendChild(jingle);
}

void JT_JingleAction::sessionAccept(const QList<JingleContent*>& contents)
{
	// ----------------------------
	if (d->session == 0)
	{
		qDebug() << "d->session is NULL, did you set it calling JT_JingleAction::setSession() ?";
		return;
	}
	qDebug() << "Sending the session-accept to : " << d->session->to().full();

	d->iq = createIQ(doc(), "set", d->session->to().full(), id());
	d->iq.setAttribute("from", client()->jid().full());

	QDomElement jingle = doc()->createElement("jingle");
	jingle.setAttribute("xmlns", NS_JINGLE);
	jingle.setAttribute("action", "session-accept");
	jingle.setAttribute("initiator", d->session->initiator());
	jingle.setAttribute("sid", d->session->sid());
	//---------This par should be in another method (createJingleIQ(...))
	
	for (int i = 0; i < contents.count(); i++)
	{
		jingle.appendChild(contents[i]->contentElement());
	}

	d->iq.appendChild(jingle);
	qDebug() << "Prepare to send :";
	client()->stream().xmlToString(d->iq, false);

	//send(d->iq);
}

bool JT_JingleAction::take(const QDomElement &x)
{
	if (!iqVerify(x, d->session->to().full(), id()))
		return false;
	
	setSuccess();
	emit finished();

	return true;
}

void JT_JingleAction::onGo()
{
	send(d->iq);
}

