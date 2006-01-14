/*
    jinglesession.h - Define a Jingle session.

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
#ifndef JINGLESESSION_H
#define JINGLESESSION_H

#include <qobject.h>
#include <qstring.h>

#include <xmpp.h> // XMPP::Jid
#include <qvaluelist.h>

class JabberAccount;
/**
 * @brief Base class for peer-to-peer session that use Jingle signaling
 *
 * @author Michaël Larouche <michael.larouche@kdemail.net>
 */
class JingleSession : public QObject
{
	Q_OBJECT
public:
	typedef QValueList<XMPP::Jid> JidList;

	JingleSession(JabberAccount *account, const JidList &peers);
	virtual ~JingleSession();

	/**
	 * Return the JabberAccount associated with this session.
	 */
	JabberAccount *account();

	const XMPP::Jid &myself() const;
	const JidList &peers() const;
	JidList &peers();
	
	/**
	 * Return the type of session(ex: voice, video, games)
	 * Note that you must return the XML namespace that define 
	 * the session: ex:(http://jabber.org/protocol/jingle/sessions/audio)
	 */
	virtual QString sessionType() = 0;

public slots:
	/**
	 * @brief Start a session with the give JID.
	 * You should begin the negociation here.
	 */
	virtual void start() = 0;
	/**
	 * @brief Acept a session request.
	 */
	virtual void accept() = 0;
	/**
	 * @brief Decline a session request.
	 */
	virtual void decline() = 0;
	/**
	 * @brief Terminate a Jingle session.
	 */
	virtual void terminate() = 0;

protected slots:
	void sendStanza(const QString &stanza);
	
signals:
	/**
	 * Session is started(negocation and connection test are done).
	 */
	void sessionStarted();

	void accepted();
	void declined();
	void terminated();

private:
	class Private;
	Private *d;
};

#endif
