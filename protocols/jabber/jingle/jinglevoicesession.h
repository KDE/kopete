/*
    jinglevoicesession.h - Define a Jingle voice session.

    Copyright (c) 2006      by Michaël Larouche     <michael.larouche@kdemail.net>

    Kopete    (c) 2001-2006 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/
#ifndef JINGLEVOICESESSION_H
#define JINGLEVOICESESSION_H

#include <jinglesession.h>

#include <xmpp.h> // XMPP::Jid
#include <qvaluelist.h>

namespace cricket
{
	class Call;
}

class JabberAccount;
class JingleSession;

/**
 * Implement a Jingle voice peer-to-peer session that is compatible with Google Talk voice offering.
 *
 * @author Michaël Larouche
*/
class JingleVoiceSession : public JingleSession
{
	Q_OBJECT
public:
	typedef QValueList<XMPP::Jid> JidList;

	JingleVoiceSession(JabberAccount *account, const JidList &peers);
	virtual ~JingleVoiceSession();

	virtual QString sessionType();

public slots:
	virtual void accept();
	virtual void decline();
	virtual void start();
	virtual void terminate();

protected slots:
	void receiveStanza(const QString &stanza);

private:
	void setCall(cricket::Call *call);

	class Private;
	Private *d;

	class SlotsProxy;
	SlotsProxy *slotsProxy;
	
	class JingleIQResponder;
};

#endif
