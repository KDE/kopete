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

#ifndef JT_AHCOMMAND_H
#define JT_AHCOMMAND_H

#include <QSet>

#include "xmpp_task.h"	
#include "xmpp_jid.h"
#include "xmpp_xdata.h"

class QDomElement;

class AHCommand
{
public:
	// Types
	enum Action { NoAction, Execute, Prev, Next, Complete, Cancel };
	enum Status { NoStatus, Completed, Executing, Canceled };

	AHCommand(const QString &node, const QString &sessionId = "", Action action = Execute);
	AHCommand(const QString &node, XMPP::XData data, const QString &sessionId = "", Action action = Execute);
	AHCommand(const QDomElement &e);

	QString node() const {return mNode; }
	QString sessionId() const {return mSessionId; }
	Status status() const {return mStatus; }
	Action defaultAction() const {return mDefaultAction; }
	const QSet<Action> &actions() const {return mActions; }
	const XMPP::XData &data() const {return mData; }

	QDomElement toXml(QDomDocument* doc, bool submit);

protected:
	QString status2string(Status status);
	QString action2string(Action action);
	Action string2action(const QString &s);
	Status string2status(const QString &s);

private:
	QString      mNode;
	bool         mHasData;
	Status       mStatus;
	Action       mDefaultAction;
	Action       mAction;
	QString      mSessionId;
	XMPP::XData  mData;
	QSet<Action> mActions;
};

class JT_AHCommand : public XMPP::Task
{
	Q_OBJECT
public:
	JT_AHCommand(const XMPP::Jid &jid, const AHCommand &command, XMPP::Task *parent);
	~JT_AHCommand();

	void onGo();
	bool take(const QDomElement &e);
private:
	XMPP::Jid mJid;
	AHCommand mCommand;
};

class JT_AHCGetList : public XMPP::Task
{
public:
	struct Item
	{
		QString jid;
		QString node;
		QString name;
	};

	JT_AHCGetList(XMPP::Task *t, const XMPP::Jid &j);

	void onGo();
	bool take(const QDomElement &x);

	const QList<Item> &commands() const {return mCommands; }

private:
	XMPP::Jid mJid;
	QList<Item> mCommands;
};

#endif
