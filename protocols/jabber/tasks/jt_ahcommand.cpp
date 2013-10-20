 /*
    Copyright (c) 2008 by Igor Janssen  <alaves17@gmail.com>

    Kopete    (c) 2008 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
 */

#include "jt_ahcommand.h"

#include "xmpp_xmlcommon.h"
#include "xmpp_client.h"

#include "dlgahcommand.h"

#define AHC_NS "http://jabber.org/protocol/commands"

using namespace XMPP;

AHCommand::AHCommand(const QString &node, const QString &sessionId, Action action)
{
	mNode = node;
	mHasData = false;
	mStatus = NoStatus;
	mDefaultAction = NoAction;
	mAction = action;
	mSessionId = sessionId;
}

AHCommand::AHCommand(const QString &node, XMPP::XData data, const QString &sessionId, Action action)
{
	mNode = node;
	mHasData = true;
	mData = data;
	mStatus = NoStatus;
	mDefaultAction = NoAction;
	mAction = action;
	mSessionId = sessionId;
}

AHCommand::AHCommand(const QDomElement &e)
{
	mHasData = false;
	mDefaultAction = NoAction;
	mStatus = string2status(e.attribute("status"));
	mNode = e.attribute("node");
	mAction = string2action(e.attribute("action"));
	mSessionId = e.attribute("sessionid");

	for(QDomNode n = e.firstChild(); !n.isNull(); n = n.nextSibling())
	{
		QDomElement ee = n.toElement();
		if(ee.isNull())
			continue;
		if(ee.tagName() == "x" && ee.attribute("xmlns") == "jabber:x:data")
		{
			// Data form
			mData.fromXml(ee);
			mHasData = true;
		}
		else if(ee.tagName() == "actions")
		{
			// Actions
			QString execute = ee.attribute("execute");
			if(!execute.isEmpty()) 
				mDefaultAction = string2action(execute);
			for(QDomNode m = ee.firstChild(); !m.isNull(); m = m.nextSibling())
			{
				Action a = string2action(m.toElement().tagName());
				if(a == Prev || a == Next || a == Complete)
					mActions += a;
			}
		}
	}
}

QDomElement AHCommand::toXml(QDomDocument *doc, bool submit)
{
	QDomElement command = doc->createElement("command");
	command.setAttribute("xmlns", AHC_NS);
	if(mStatus != NoStatus)
		command.setAttribute("status", status2string(mStatus));
	if(mHasData)
		command.appendChild(mData.toXml(doc, submit));
	if(mAction != Execute)
		command.setAttribute("action", action2string(mAction));
	command.setAttribute("node", mNode);
	if(!mSessionId.isEmpty())
		command.setAttribute("sessionid", mSessionId);
	return command;
}

QString AHCommand::status2string(Status status)
{
	switch(status)
	{
	case Executing:
		return "executing";
	case Completed:
		return "completed";
	case Canceled:
		return "canceled";
	default:
		return "";
	}
}

QString AHCommand::action2string(Action action)
{
	switch(action)
	{
	case Prev:
		return "prev";
	case Next:
		return "next";
	case Cancel:
		return "cancel";
	case Complete:
		return "complete";
	default:
		return "";
	}
}

AHCommand::Action AHCommand::string2action(const QString &s)
{
	if(s == "prev")
		return Prev;
	else if(s == "next")
		return Next;
	else if(s == "complete")
		return Complete;
	else if(s == "cancel")
		return Cancel;
	else
		return Execute;
}

AHCommand::Status AHCommand::string2status(const QString &s)
{
	if(s == "canceled")
		return Canceled;
	else if(s == "completed")
		return Completed;
	else if(s == "executing")
		return Executing;
	else 
		return NoStatus;
}

//----------------------------------------------------------------------------------------------

JT_AHCommand::JT_AHCommand(const XMPP::Jid &jid, const AHCommand &command, XMPP::Task *parent):
Task(parent), mCommand(command)
{
	mJid = jid;
}

JT_AHCommand::~JT_AHCommand()
{
}

void JT_AHCommand::onGo()
{
	QDomElement e = createIQ(doc(), "set", mJid.full(), id());
	e.appendChild(mCommand.toXml(doc(), true));
	send(e);
}

bool JT_AHCommand::take(const QDomElement &e)
{
	if(!iqVerify(e, mJid, id()))
		return false;
	
	// Result of a command
	if(e.attribute("type") == "result")
	{
		QDomElement i = e.firstChildElement("command");
		if(!i.isNull())
		{
			AHCommand c(i);
			if(c.status() == AHCommand::Executing)
			{
				dlgAHCommand *w = new dlgAHCommand(c, mJid, client());
				w->show();
			}
			else if(c.status() == AHCommand::Completed && i.childNodes().count() > 0)
			{
				dlgAHCommand *w = new dlgAHCommand(c, mJid, client(), true);
				w->show();
			}
			setSuccess();
			return true;
		}
	}
	setError(e);
	return false;
}

//----------------------------------------------------------------------------------------------



JT_AHCGetList::JT_AHCGetList(Task *t, const Jid &j):
Task(t)
{
	mJid = j;
}

void JT_AHCGetList::onGo()
{
	QDomElement e = createIQ(doc(), "get", mJid.full(), id());
	QDomElement q = doc()->createElement("query");
	q.setAttribute("xmlns", "http://jabber.org/protocol/disco#items");
	q.setAttribute("node", AHC_NS);
	e.appendChild(q);
	send(e);
}

bool JT_AHCGetList::take(const QDomElement &e)
{
	if(!iqVerify(e, mJid, id()))
		return false;

	if(e.attribute("type") == "result")
	{
		mCommands.clear();
		QDomElement commands = e.firstChildElement("query");
		if(!commands.isNull())
		{
			for(QDomNode n = commands.firstChild(); !n.isNull(); n = n.nextSibling())
			{
				QDomElement i = n.toElement();
				if(i.isNull())
					continue;
				if(i.tagName() == "item")
				{
					JT_AHCGetList::Item ci;
					ci.jid = i.attribute("jid");
					ci.node = i.attribute("node");
					ci.name = i.attribute("name");
					mCommands.append(ci);
				}
			}
		}
		setSuccess();
		return true;
	}
	else
	{
		setError(e);
		return false;
	}
}

#include "jt_ahcommand.moc"
