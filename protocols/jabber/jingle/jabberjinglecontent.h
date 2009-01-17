/*
 * jabberjinglecontent.h - A Jingle content.
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
#ifndef JABBER_JINGLE_CONTENT
#define JABBER_JINGLE_CONTENT

#include <QObject>
#include <QString>
#include <QDomElement>

namespace XMPP
{
	class JingleContent;
	class JingleSession;
}
class JabberJingleSession;
class MediaManager;
class MediaSession;
class JingleRtpSession;

class JabberJingleContent : public QObject
{
	Q_OBJECT
public:
	JabberJingleContent(JabberJingleSession* parent = 0, XMPP::JingleContent* c = 0);
	~JabberJingleContent();

	void setContent(XMPP::JingleContent*);
	//void startWritingRtpData();
	void startStreaming();
	QString contentName();
	QString elementToSdp(const QDomElement&);

public slots:
	void slotSendRtpData();
	void slotIncomingData(const QByteArray&);
	void slotReadyRead();

private:
	XMPP::JingleContent *m_content;
	XMPP::JingleSession *m_jingleSession;
	MediaManager *m_mediaManager;
	MediaSession *m_mediaSession;
	JingleRtpSession *m_rtpInSession;
	JingleRtpSession *m_rtpOutSession;
	JabberJingleSession *m_jabberSession;
	
	void prepareRtpOutSession();
	void prepareRtpInSession();
};

#endif
