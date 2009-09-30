/*
 * jingleaction.cpp - Represent a Jingle action
 *
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
#include "jingleaction.h"

using namespace XMPP;

class JingleAction::Private {
public:
	Action action;
	Jid from;
	Jid to;
	QString id; // may not be needed;
	QString sid;
	QString initiator;
	QString type;
	QDomElement data;
	bool valid; // True if the parsed stanza is a valid Jingle stanza, false if not.
};

JingleAction::JingleAction()
 : d (new Private())
{
	d->valid = false;
}

JingleAction::JingleAction(const QDomElement& stanza)
 : d (new Private())
{
	d->valid = setStanza(stanza) == EXIT_SUCCESS ? true : false;
}

JingleAction::~JingleAction()
{
	delete d;
}

bool JingleAction::isValid() const
{
	return d->valid;
}

void JingleAction::setData(const QDomElement& data)
{
	d->data = data;
}

int JingleAction::setStanza(const QDomElement& stanza)
{
	if (stanza.tagName() != "iq" || stanza.firstChildElement().tagName() != "jingle")
		return EXIT_FAILURE;
	
	d->type = stanza.attribute("type");
	d->action = jingleAction(stanza.firstChildElement().attribute("action"));
	d->sid = stanza.firstChildElement().attribute("sid");
	d->initiator = stanza.firstChildElement().attribute("initiator");
	d->id = stanza.attribute("id");
	d->from = Jid(stanza.attribute("from"));
	d->to = Jid(stanza.attribute("to"));
	d->data = stanza.firstChildElement();

	return EXIT_SUCCESS;
}

QString JingleAction::sid() const
{
	return d->sid;
}

JingleAction::Action JingleAction::action() const
{
	return d->action;
}

QString JingleAction::id() const
{
	return d->id;
}

Jid JingleAction::from() const
{
	return d->from;
}

Jid JingleAction::to() const
{
	return d->to;
}

QString JingleAction::initiator() const
{
	return d->initiator;
}

QDomElement JingleAction::data() const
{
	return d->data;
}

JingleAction::Action JingleAction::jingleAction(const QString& action)
{
	if (action == "session-initiate")
		return SessionInitiate;
	else if (action == "session-terminate")
		return SessionTerminate;
	else if (action == "session-accept")
		return SessionAccept;
	else if (action == "session-info")
		return SessionInfo;
	else if (action == "content-add")
		return ContentAdd;
	else if (action == "content-remove")
		return ContentRemove;
	else if (action == "content-modify")
		return ContentModify;
	else if (action == "transport-replace")
		return TransportReplace;
	else if (action == "transport-accept")
		return TransportAccept;
	else if (action == "transport-info")
		return TransportInfo;
	else
		return NoAction;
}

