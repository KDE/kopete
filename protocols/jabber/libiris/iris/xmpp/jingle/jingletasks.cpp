/*
 * jingletasks.cpp - Tasks for the Jingle specification.
 * Copyright (C) 2009 - Detlev Casanova <detlev.casanova@gmail.com>
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
#include "jingleapplication.h"
#include "jingletransport.h"

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
	//QString id;
	//Jid from;
	QList<JingleAction*> actions;
};

JT_PushJingleAction::JT_PushJingleAction(Task *parent)
: Task(parent), d(new Private)
{
}

JT_PushJingleAction::~JT_PushJingleAction()
{
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
	
	d->actions << new JingleAction(x);

	emit jingleActionReady();

	return true;
}

void JT_PushJingleAction::ack(const QString& id, const Jid& to)
{
	d->iq = createIQ(doc(), "result", to.full(), id);
	send(d->iq);
}

void JT_PushJingleAction::unknownSession(const QString& id, const Jid& to)
{
	d->iq = createIQ(doc(), "error", to.full(), id);
	
	QDomElement error = doc()->createElement("error");
	error.setAttribute("type", "cancel");
	QDomElement inf = doc()->createElement("item-not-found");
	inf.setAttribute("xmlns", "urn:ietf:params:xml:ns:xmpp-stanzas");
	QDomElement us = doc()->createElement("unknown-session");
	us.setAttribute("xmlns", NS_JINGLE_ERROR);

	error.appendChild(inf);
	error.appendChild(us);

	d->iq.appendChild(error);

	send(d->iq);
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
	delete d;
}

void JT_JingleAction::setSession(JingleSession *sess)
{
	d->session = sess;
}

void JT_JingleAction::initiate()
{
	createJingleIq("session-initiate");
	
	QDomElement jingle = d->iq.firstChildElement();
	jingle.setAttribute("initiator", d->session->initiator());

	foreach (JingleContent *content, d->session->contents())
	{
		jingle.appendChild(content->contentElement(JingleTransport::LocalCandidates, JingleApplication::LocalApplication));
	}
}

void JT_JingleAction::contentAccept()
{
	//createJingleIq("content-accept");
}

void JT_JingleAction::sessionInfo(const QDomElement& e)
{
	createJingleIq("session-info");

	// Adding the argument to the Jingle element.
	d->iq.firstChildElement().appendChild(e);
}

void JT_JingleAction::terminate(const QDomElement& r)
{
	createJingleIq("session-terminate");

	QDomElement jingle = d->iq.firstChildElement();

	if (!r.isNull())
		jingle.appendChild(r);
	/*else
	 * 	add an empty reason.
	 */
}

void JT_JingleAction::removeContents(const QList<JingleContent*>& c)
{
	createJingleIq("content-remove");

	QDomElement jingle = d->iq.firstChildElement();
	
	foreach (JingleContent *content, c)
	{
		QDomElement contentElem = doc()->createElement("content");
		contentElem.setAttribute("name", content->name());
		jingle.appendChild(contentElem);
	}
}

void JT_JingleAction::transportInfo(const QString& contentName, const QDomElement& transport)
{
	createJingleIq("transport-info");
	
	QDomElement jingle = d->iq.firstChildElement();

	QDomElement content = doc()->createElement("content");
	content.setAttribute("name", contentName);
	content.setAttribute("creator", d->session->contentWithName(contentName)->creator());

	content.appendChild(transport);
	jingle.appendChild(content);
}

void JT_JingleAction::sessionAccept(const QList<JingleContent*>& contents)
{
	createJingleIq("session-accept");
	
	QDomElement jingle = d->iq.firstChildElement();
	jingle.setAttribute("responder", d->session->responder());

	foreach (JingleContent *content, contents)
	{
		jingle.appendChild(content->contentElement(JingleTransport::NoCandidate, JingleApplication::Application));
	}
}

bool JT_JingleAction::take(const QDomElement &x)
{
	if (!iqVerify(x, d->session->to().full(), id()))
		return false;
	
	if (x.attribute("type") == "result")
		setSuccess();
	else
		setError(x);

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

	d->iq = createIQ(doc(), "set", d->session->to().full(), id());
	d->iq.setAttribute("from", client()->jid().full());

	QDomElement jingle = doc()->createElement("jingle");
	jingle.setAttribute("xmlns", NS_JINGLE);
	jingle.setAttribute("action", action);
	//jingle.setAttribute("initiator", d->session->initiator());
	jingle.setAttribute("sid", d->session->sid());

	d->iq.appendChild(jingle);
}

