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
#include <KDebug>
//Kopete
#include "jabberjinglecontent.h"
#include "jabberjinglesession.h"
#include "jinglemediamanager.h"
#include "jinglertpsession.h"

//Iris
#include "jinglecontent.h"
#include "jinglesession.h"

JabberJingleContent::JabberJingleContent(JabberJingleSession* parent, XMPP::JingleContent* c)
 : m_content(c), m_jabberSession(parent)
{
	m_rtpInSession = 0;
	m_rtpOutSession = 0;
	m_mediaManager = m_jabberSession->mediaManager();
	if (!m_mediaManager)
		kDebug(KDE_DEFAULT_DEBUG_AREA) << "m_mediaManager is Null !";
	if (c == 0)
		return;
	
//	prepareRtpInSession();
//	prepareRtpOutSession();

	kDebug(KDE_DEFAULT_DEBUG_AREA) << "Created a new JabberJingleContent with" << c->name();
}

JabberJingleContent::~JabberJingleContent()
{
	delete m_content;
	delete m_rtpInSession;
	delete m_rtpOutSession;
}

void JabberJingleContent::setContent(XMPP::JingleContent* content)
{
	m_content = content;
//	connect(m_content, SIGNAL(socketReady()), this, SLOT(slotPrepareRtpSession()));
}

void JabberJingleContent::prepareRtpInSession()
{
	kDebug(KDE_DEFAULT_DEBUG_AREA) << "Prepare RTP IN session";
	if (m_rtpInSession == 0)
	{
		m_rtpInSession = new JingleRtpSession(JingleRtpSession::In);
		if (!m_content->inSocket())
		{
			kDebug() << "Fatal : Invalid Socket !";
			return;
		}
		m_rtpInSession->setPayload(m_content->bestPayload());
		m_rtpInSession->setRtpSocket(m_content->inSocket()); // This will set rtcp port = rtp port + 1. Maybe we don't want that for ice-udp.
		connect(m_rtpInSession, SIGNAL(readyRead(const QByteArray&)), this, SLOT(slotIncomingData(const QByteArray&)));
	}
}

void JabberJingleContent::prepareRtpOutSession()
{
	kDebug(KDE_DEFAULT_DEBUG_AREA) << "Prepare RTP OUT session";
	if (m_rtpOutSession == 0)
	{
		m_rtpOutSession = new JingleRtpSession(JingleRtpSession::Out);
		if (!m_content->outSocket())
		{
			kDebug() << "Fatal : Invalid Socket !";
			return;
		}
		m_rtpOutSession->setRtpSocket(m_content->outSocket()); // This will set rtcp port = rtp port + 1. Maybe we don't want that for ice-udp.
	}
}

void JabberJingleContent::slotIncomingData(const QByteArray& data)
{
	kDebug() << "Receiving ! (" << data.size() << "bytes)";
	m_mediaSession->playData(data);
}

void JabberJingleContent::startWritingRtpData()
{
	kDebug() << "Start Writing Rtp Data.";
	
	//slotPrepareRtpOutSession(); --> That should already be done...

	if (m_jabberSession->session()->state() == XMPP::JingleSession::Pending)
	{
		m_rtpOutSession->setPayload(m_content->bestPayload());
	}
	if (m_content->type() == XMPP::JingleContent::Audio)
	{
		m_mediaSession = m_mediaManager->createNewSession(m_content->bestPayload());
		if (m_mediaSession == 0)
		{
			kDebug() << "Media Session is NULL!";
			kDebug() << "Number of payloads :" << m_content->payloadTypes().count();
			kDebug() << "Number of responder payloads :" << m_content->responderPayloads().count();
			return;
		}
		connect(m_mediaSession, SIGNAL(readyRead(int)), this, SLOT(slotReadyRead(int)));
		m_mediaSession->start();
		/*connect(m_mediaManager, SIGNAL(audioReadyRead()), this, SLOT(slotSendRtpData()));
		m_mediaManager->startAudioStreaming();*/
	}
	//m_content->setSending(true);
}

void JabberJingleContent::slotReadyRead(int ts)
{
	m_rtpOutSession->send(m_mediaSession->data(), ts);
}

QString JabberJingleContent::elementToSdp(const QDomElement& elem)
{
	Q_UNUSED(elem)
	return QString();
}

/*DEPRECATED*/void JabberJingleContent::slotSendRtpData()
{
	//kDebug() << "Send RTP data.";
	m_rtpOutSession->send(m_mediaManager->data());
}

QString JabberJingleContent::contentName()
{
	if (!m_content)
		return "";
	return m_content->name();
}

