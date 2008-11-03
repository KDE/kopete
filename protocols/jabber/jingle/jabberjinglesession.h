 /*
  * jabberjinglesession.h - A Jingle session.
  *
  * Copyright (c) 2008 by Detlev Casanova <detlev.casanova@gmail.com>
  *
  * Kopete    (c) by the Kopete developers  <kopete-devel@kde.org>
  *
  * *************************************************************************
  * *                                                                       *
  * * This program is free software; you can redistribute it and/or modify  *
  * * it under the terms of the GNU General Public License as published by  *
  * * the Free Software Foundation; either version 2 of the License, or     *
  * * (at your option) any later version.                                   *
  * *                                                                       *
  * *************************************************************************
  */
#ifndef JABBER_JINGLE_SESSION_H
#define JABBER_JINGLE_SESSION_H
#include <QObject>
#include <QDomElement>
#include <QTimer>
#include <QTime>

namespace XMPP
{
	class JingleSession;
	class JingleContent;
}
class JingleCallsManager;
class JabberJingleContent;
class MediaManager;
class MediaSession;
class JingleRtpSession;

class JabberJingleSession : public QObject
{
	Q_OBJECT
public:
	// Session states
	enum State {
		Pending = 0,
		Active,
		Ended
	};
	JabberJingleSession(JingleCallsManager*);
	~JabberJingleSession();
	
	void setJingleSession(XMPP::JingleSession*);
	XMPP::JingleSession *jingleSession() {return m_jingleSession;}
	
	JabberJingleContent *contentWithName(const QString&);
	XMPP::JingleSession *session() const {return m_jingleSession;} //FIXME:Use jingleSession()
	MediaManager *mediaManager() const;
	QList<JabberJingleContent*> contents() const {return jabberJingleContents;}

	State state();

	QTime upTime();

public slots:
	//void writeRtpData(XMPP::JingleContent*);
	void slotSessionTerminated();
	void slotStateChanged();

signals:
	void terminated();
	void stateChanged();

private:
	XMPP::JingleSession *m_jingleSession;
	JingleCallsManager *m_callsManager;
	MediaManager *m_mediaManager;
	QList<JabberJingleContent*> jabberJingleContents;
	QTime m_startTime;
};

#endif
