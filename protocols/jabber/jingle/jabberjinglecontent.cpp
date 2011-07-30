 /*
  * jabberjinglecontent.cpp - A Jingle content.
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

//Kopete
#include "jabberjinglecontent.h"
#include "jabberjinglesession.h"
#include "mediamanager.h"
#include "mediasession.h"
#include "jinglertpsession.h"

//Iris
#include "jinglecontent.h"
#include "jinglesession.h"

#include <KDebug>
#include <QMessageBox>

JabberJingleContent::JabberJingleContent(JabberJingleSession* parent, XMPP::JingleContent* c)
 : m_content(c), m_jabberSession(parent)
{
	m_rtpInSession = 0;
	m_rtpOutSession = 0;
	m_mediaSession = 0;
	m_mediaManager = m_jabberSession->mediaManager();
	if (!m_mediaManager)
		kDebug(KDE_DEFAULT_DEBUG_AREA) << "m_mediaManager is Null !";
	if (c == 0)
		return;
	
	kDebug(KDE_DEFAULT_DEBUG_AREA) << "Created a new JabberJingleContent with" << c->name();
}

JabberJingleContent::~JabberJingleContent()
{
	kDebug() << "destroyed";
	delete m_content;
	delete m_rtpInSession;
	delete m_rtpOutSession;
	delete m_mediaSession;
}

void JabberJingleContent::setContent(XMPP::JingleContent* content)
{
	m_content = content;
}

void JabberJingleContent::prepareRtpInSession()
{
	kDebug(KDE_DEFAULT_DEBUG_AREA) << "Prepare RTP IN session";
	if (m_rtpInSession == 0)
	{
		if (!m_content->inSocket())
		{
			kDebug() << "Fatal : Invalid Socket !";
			return;
		}
		m_rtpInSession = new JingleRtpSession(JingleRtpSession::In);
		m_rtpInSession->setMediaSession(m_mediaSession);
		m_rtpInSession->setPayload(m_content->bestPayload());
		m_rtpInSession->setRtpSocket(m_content->inSocket()); // This will set rtcp port = rtp port + 1. Maybe we don't want that for ice-udp.
		kDebug() << "Connecting m_rtpInSession readyRead signal.";
		connect(m_rtpInSession, SIGNAL(readyRead(QByteArray)), this, SLOT(slotIncomingData(QByteArray)));
	}
	else
		kDebug() << "RTP IN session already set !";
}

void JabberJingleContent::prepareRtpOutSession()
{
	kDebug(KDE_DEFAULT_DEBUG_AREA) << "Prepare RTP OUT session";
	if (m_rtpOutSession == 0)
	{
		if (!m_content->outSocket())
		{
			kDebug() << "Fatal : Invalid Socket !";
			return;
		}
		m_rtpOutSession = new JingleRtpSession(JingleRtpSession::Out);
		m_rtpOutSession->setMediaSession(m_mediaSession);
		m_rtpOutSession->setPayload(m_content->bestPayload());
		m_rtpOutSession->setRtpSocket(m_content->outSocket()); // This will set rtcp port = rtp port + 1. Maybe we don't want that for ice-udp.
		if (m_jabberSession->session()->state() == XMPP::JingleSession::Pending)
		{
			m_rtpOutSession->setPayload(m_content->bestPayload());
		}
		
	}
	else
		kDebug() << "RTP OUT session already set !";
}

void JabberJingleContent::slotIncomingData(const QByteArray& data)
{
	//kDebug() << "Receiving ! (" << data.size() << "bytes)";
	m_mediaSession->write(data);
}

void JabberJingleContent::startStreaming()
{
	kDebug() << "Start Streaming";

	if (m_content->type() == XMPP::JingleContent::Audio)
	{
		m_mediaSession = new MediaSession(m_mediaManager, "speex"/*FIXME:use m_content->bestPayload()*/);
		if (m_mediaSession == 0)
		{
			kDebug() << "Media Session is NULL!";
			return;
		}
		connect(m_mediaSession, SIGNAL(readyRead()), this, SLOT(slotReadyRead()));
		m_mediaSession->setSamplingRate(8000 /*FIXME:use m_content->bestPayload()*/);

		prepareRtpOutSession();
		prepareRtpInSession();

		if (!m_mediaSession->start())
			QMessageBox::warning(0, tr("Jingle audio"), tr("Unable to start you audio device, the session will start anyway."));
	}
}

void JabberJingleContent::slotReadyRead()
{
	m_rtpOutSession->send(m_mediaSession->read());
}

QString JabberJingleContent::elementToSdp(const QDomElement& elem)
{
	Q_UNUSED(elem)
	return QString();
}

/*DEPRECATED*/void JabberJingleContent::slotSendRtpData()
{
	//kDebug() << "Send RTP data.";
	m_rtpOutSession->send(m_mediaSession->read());
}

QString JabberJingleContent::contentName()
{
	if (!m_content)
		return "";
	return m_content->name();
}

