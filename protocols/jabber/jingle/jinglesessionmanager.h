/*
    jinglesessionmanager.h - Manage Jingle sessions.

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
#ifndef JINGLESESSIONMANAGER_H
#define JINGLESESSIONMANAGER_H

#include <xmpp.h>
#include <im.h>

#include <qobject.h>
#include <qvaluelist.h>

namespace cricket
{
	class SessionManager;
}

class JingleSession;
class JingleVoiceSession;
class JabberAccount;

/**
 * @brief Manage Jingle sessions. 
 * @author Michaël Larouche <michael.larouche@kdemail.net>
 */
class JingleSessionManager : public QObject
{
	Q_OBJECT
public:
	typedef QValueList<XMPP::Jid> JidList;

	JingleSessionManager(JabberAccount *account);
	~JingleSessionManager();

	/**
	 * Get the (single) instance of the cricket session manager.
	 */
	cricket::SessionManager *cricketSessionManager();

	/**
	 * Return the JabberAccount associated with this session manager.
	 */
	JabberAccount *account();

public slots:
	/**
	 * Create a new Jingle session. Returned pointer is managed by this class.
	 * @param sessionType the session you want to create. You must pass its XML namespace(ex: http://jabber.org/protocol/sessions/audio)
	 * @param peers Lists of participants of the session.
	 */
	JingleSession *createSession(const QString &sessionType, const JidList &peers);
	/**
	 * Override method that create a session for a one-to-one session.
	 * It behave like createSession method.
	 * @param sessionType the sesion you want to create. You must pass its XML namespace(ex: http://jabber.org/protocol/sessions/audio)
	 * @param user The JID of the user you want to begin a session with.
	 */
	JingleSession *createSession(const QString &sessionType, const XMPP::Jid &user);
	
	void removeSession(JingleSession *session);

signals:
	void incomingSession(const QString &sessionType, JingleSession *session);

private slots:
	void slotIncomingSession(const QString &sessionType, const QString &initiator);

private:
	class Private;
	Private *d;

	class SlotsProxy;
	SlotsProxy *slotsProxy;
};

#endif
