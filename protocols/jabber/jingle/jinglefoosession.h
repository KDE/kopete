/*
    jinglevoicesessiondialog.cpp - GUI for a voice session.

    Copyright (c) 2007      by Joshua Hodosh     <josh.hodosh@gmail.com>

    Kopete    (c) 2001-2007 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/
#ifndef JINGLEFOOSESSION_H_
#define JINGLEFOOSESSION_H_

#include "jinglefootransport.h"
#include "jinglesession.h"

class JabberAccount;
class QDomDocument;
class JingleFooConnectionCandidate;


/**
 * Session which sends ASCII text "foo"
 *
*/
class JingleFooSession : public JingleSession
{
	Q_OBJECT
public:
	typedef Q3ValueList<XMPP::Jid> JidList;

	JingleFooSession(JabberAccount *account, const JidList &peers);
	virtual ~JingleFooSession();

	virtual QString sessionType();

public slots:
	virtual void accept();
	virtual void decline();
	virtual void start();
	virtual void terminate();

protected slots:
	void receiveStanza(const QString &stanza);

protected:

	virtual JingleTransport* transport()  { return & fooTransport; }

	//virtual JingleConnectionCandidate* connection() { return connection; }

	virtual bool addRemoteCandidate(QDomElement transportElement);

private:
	virtual void updateContent(QDomElement stanza);
	virtual void sendTransportCandidates(int contentIndex);

	virtual void checkContent(QDomElement stanza);

	QList<JingleFooConnectionCandidate> remoteCandidates;

	virtual void checkNewContent(QDomElement stanza);

	virtual void removeContent(QDomElement stanza);

	JingleFooTransport fooTransport;
	//JingleFooConnectionCandidate connection;
	
};
 
#endif
