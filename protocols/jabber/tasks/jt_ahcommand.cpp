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
	mStatus = string2status(e.attribute(QStringLiteral("status")));
	mNode = e.attribute(QStringLiteral("node"));
	mAction = string2action(e.attribute(QStringLiteral("action")));
	mSessionId = e.attribute(QStringLiteral("sessionid"));

	for(QDomNode n = e.firstChild(); !n.isNull(); n = n.nextSibling())
	{
		QDomElement ee = n.toElement();
		if(ee.isNull())
			continue;
		if(ee.tagName() == QLatin1String("x") && ee.attribute(QStringLiteral("xmlns")) == QLatin1String("jabber:x:data"))
		{
			// Data form
			mData.fromXml(ee);
			mHasData = true;
		}
		else if(ee.tagName() == QLatin1String("actions"))
		{
			// Actions
			QString execute = ee.attribute(QStringLiteral("execute"));
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
	QDomElement command = doc->createElement(QStringLiteral("command"));
	command.setAttribute(QStringLiteral("xmlns"), AHC_NS);
	if(mStatus != NoStatus)
		command.setAttribute(QStringLiteral("status"), status2string(mStatus));
	if(mHasData)
		command.appendChild(mData.toXml(doc, submit));
	if(mAction != Execute)
		command.setAttribute(QStringLiteral("action"), action2string(mAction));
	command.setAttribute(QStringLiteral("node"), mNode);
	if(!mSessionId.isEmpty())
		command.setAttribute(QStringLiteral("sessionid"), mSessionId);
	return command;
}

QString AHCommand::status2string(Status status)
{
	switch(status)
	{
	case Executing:
		return QStringLiteral("executing");
	case Completed:
		return QStringLiteral("completed");
	case Canceled:
		return QStringLiteral("canceled");
	default:
		return QLatin1String("");
	}
}

QString AHCommand::action2string(Action action)
{
	switch(action)
	{
	case Prev:
		return QStringLiteral("prev");
	case Next:
		return QStringLiteral("next");
	case Cancel:
		return QStringLiteral("cancel");
	case Complete:
		return QStringLiteral("complete");
	default:
		return QLatin1String("");
	}
}

AHCommand::Action AHCommand::string2action(const QString &s)
{
	if(s == QLatin1String("prev"))
		return Prev;
	else if(s == QLatin1String("next"))
		return Next;
	else if(s == QLatin1String("complete"))
		return Complete;
	else if(s == QLatin1String("cancel"))
		return Cancel;
	else
		return Execute;
}

AHCommand::Status AHCommand::string2status(const QString &s)
{
	if(s == QLatin1String("canceled"))
		return Canceled;
	else if(s == QLatin1String("completed"))
		return Completed;
	else if(s == QLatin1String("executing"))
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
	QDomElement e = createIQ(doc(), QStringLiteral("set"), mJid.full(), id());
	e.appendChild(mCommand.toXml(doc(), true));
	send(e);
}

bool JT_AHCommand::take(const QDomElement &e)
{
	if(!iqVerify(e, mJid, id()))
		return false;
	
	// Result of a command
	if(e.attribute(QStringLiteral("type")) == QLatin1String("result"))
	{
		QDomElement i = e.firstChildElement(QStringLiteral("command"));
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
	QDomElement e = createIQ(doc(), QStringLiteral("get"), mJid.full(), id());
	QDomElement q = doc()->createElement(QStringLiteral("query"));
	q.setAttribute(QStringLiteral("xmlns"), QStringLiteral("http://jabber.org/protocol/disco#items"));
	q.setAttribute(QStringLiteral("node"), AHC_NS);
	e.appendChild(q);
	send(e);
}

bool JT_AHCGetList::take(const QDomElement &e)
{
	if(!iqVerify(e, mJid, id()))
		return false;

	if(e.attribute(QStringLiteral("type")) == QLatin1String("result"))
	{
		mCommands.clear();
		QDomElement commands = e.firstChildElement(QStringLiteral("query"));
		if(!commands.isNull())
		{
			for(QDomNode n = commands.firstChild(); !n.isNull(); n = n.nextSibling())
			{
				QDomElement i = n.toElement();
				if(i.isNull())
					continue;
				if(i.tagName() == QLatin1String("item"))
				{
					JT_AHCGetList::Item ci;
					ci.jid = i.attribute(QStringLiteral("jid"));
					ci.node = i.attribute(QStringLiteral("node"));
					ci.name = i.attribute(QStringLiteral("name"));
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

