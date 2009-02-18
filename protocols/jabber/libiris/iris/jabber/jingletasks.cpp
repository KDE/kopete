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
#include "jingleaction.h" 
#include "jingletasks.h"

#include "protocol.h"
#include "xmpp_xmlcommon.h"

using namespace XMPP;

//------------------------
// JT_PushJingleAction
//------------------------

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
	QList<JingleAction*> actions;
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

bool JT_PushJingleAction::hasPendingAction()
{
	return !d->actions.isEmpty();
}

JingleAction* JT_PushJingleAction::takeNextPendingAction()
{
	if (hasPendingAction())
		return d->actions.takeFirst();
	
	return 0;
}

bool JT_PushJingleAction::take(const QDomElement &x)
{
	if (x.firstChildElement().tagName() != "jingle")
		return false;
	
	d->from = x.attribute("from");
	d->id = x.attribute("id");
	ack();

	if (x.attribute("type") == "error")
	{
		jingleError(x);
		return true;
	}

	d->actions << new JingleAction(x);

	emit jingleActionReady();

	return true;
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


void JT_JingleAction::initiate()
{
	qDebug() << id();

	createJingleIq("session-initiate");
	
	QDomElement jingle = d->iq.firstChildElement();

	for (int i = 0; i < d->session->contents().count(); i++)
	{
		jingle.appendChild(d->session->contents()[i]->contentElement(JingleContent::LocalCandidates, JingleContent::LocalPayloads));
	}
}


void JT_JingleAction::contentAccept()
{
	createJingleIq("session-accept");
}

//FIXME:should be in a sessionInfo()
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
	createJingleIq("session-terminate");

	QDomElement jingle = d->iq.firstChildElement();

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
	case JingleReason::UnsupportedApplications :
		rReason = doc()->createElement("unsupported-applications");
		break;
	default:
		rReason = doc()->createElement("unknown");
	}

	jingle.appendChild(reason);
	reason.appendChild(condition);
	condition.appendChild(rReason);
}

void JT_JingleAction::removeContents(const QStringList& c)
{
	createJingleIq("content-remove");

	QDomElement jingle = d->iq.firstChildElement();
	
	for (int i = 0; i < c.count(); i++)
	{
		QDomElement content = doc()->createElement("content");
		content.setAttribute("name", c[i]);
		jingle.appendChild(content);
	}
}

void JT_JingleAction::transportInfo(const QString& contentName, const QDomElement& transport)
{
	createJingleIq("transport-info");
	
	QDomElement jingle = d->iq.firstChildElement();

	QDomElement content = doc()->createElement("content");
	content.setAttribute("name", contentName);
	content.setAttribute("creator", d->session->initiator() == d->session->to().full() ? d->session->to().full() : "initiator");

	content.appendChild(transport);
	jingle.appendChild(content);
}

void JT_JingleAction::sessionAccept(const QList<JingleContent*>& contents)
{
	createJingleIq("session-accept");
	
	QDomElement jingle = d->iq.firstChildElement();
	
	for (int i = 0; i < contents.count(); i++)
	{
		jingle.appendChild(contents[i]->contentElement(JingleContent::UsedCandidate, JingleContent::AcceptablePayloads));
	}

	qDebug() << "Prepare to send :";
	qDebug() << client()->stream().xmlToString(d->iq, false);
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

void JT_JingleAction::createJingleIq(const QString& action)
{
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
	jingle.setAttribute("action", action);
	jingle.setAttribute("initiator", d->session->initiator());
	jingle.setAttribute("sid", d->session->sid());

	d->iq.appendChild(jingle);
}

