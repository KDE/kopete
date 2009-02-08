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
#include <QMessageBox>

//Kopete
#include "jabberjinglecontent.h"
#include "jabberjinglesession.h"
#include "mediamanager.h"
#include "mediasession.h"
#include "jinglertpsession.h"

//Iris
#include "jinglecontent.h"
#include "jinglesession.h"

JabberJingleContent::JabberJingleContent(JabberJingleSession* parent, XMPP::JingleContent* c)
 : m_content(c), m_jabberSession(parent)
{
	m_rtpSession = 0;
	m_mediaSession = 0;
	m_mediaManager = m_jabberSession->mediaManager();
	contentConnected = false;
	if (!m_mediaManager)
		kDebug(KDE_DEFAULT_DEBUG_AREA) << "m_mediaManager is Null !";
	
	if (m_content)
	{
	//	connect(m_content, SIGNAL(established()), this, SLOT(slotConnectionEstablished()));
		kDebug(KDE_DEFAULT_DEBUG_AREA) << "Created a new JabberJingleContent with" << m_content->name();
	}
}

JabberJingleContent::~JabberJingleContent()
{
	kDebug() << "destroyed";
	//delete m_content; --> Destroyed by the XMPP::JingleSession
	delete m_rtpSession;
	delete m_mediaSession;
}

void JabberJingleContent::setContent(XMPP::JingleContent* content)
{
	m_content = content;

	//if (m_content)
	//	connect(m_content, SIGNAL(established()), this, SLOT(slotConnectionEstablished()));
}

void JabberJingleContent::prepareRtpSession()
{
	kDebug() << "Prepare RTP IN session";
	if (m_rtpSession == 0)
	{
		if (!m_content->isReady())
		{
			kDebug() << "Fatal : Invalid Socket !";
			return;
		}
		m_rtpSession = new JingleRtpSession();
		m_rtpSession->setMediaSession(m_mediaSession);
		m_rtpSession->setPayload(m_content->bestPayload());
		kDebug() << "Connecting m_rtpInSession readyRead signal.";
		connect(m_rtpSession, SIGNAL(mediaDataReady()), this, SLOT(slotMediaDataReady()));
		connect(m_rtpSession, SIGNAL(rtpDataReady()), this, SLOT(slotRtpDataReady()));
	}
	else
		kDebug() << "RTP session already set !";
}

void JabberJingleContent::slotMediaDataReady()
{
	m_mediaSession->write(m_rtpSession->takeMediaData());
}

void JabberJingleContent::slotRtpDataReady()
{
	if (!m_content)
	{
		kDebug() << "Content not set, unable to send RTP data to the remote peer.";
		return;
	}

	m_content->writeDatagram(m_rtpSession->takeRtpData());
}

void JabberJingleContent::slotRtpReadyRead()
{
	m_rtpSession->unpackRtpData(m_content->readAll());
}

void JabberJingleContent::slotIncomingData(const QByteArray& data)
{
	//kDebug() << "Receiving ! (" << data.size() << "bytes)";
	m_mediaSession->write(data);
}

void JabberJingleContent::slotConnectionEstablished()
{
//	kDebug() << "called";
//	contentConnected = true;

//	startStreaming();
}

void JabberJingleContent::startStreaming()
{
	kDebug() << (m_jabberSession->state() == JabberJingleSession::Active)/* << "&&" << contentConnected*/;

	if (m_jabberSession->state() != JabberJingleSession::Active/* && contentConnected*/)
	{
		kDebug() << "Not Ready yet.";
		return;
	}
	kDebug() << "Start Streaming";
	
	connect(m_content, SIGNAL(readyRead()), this, SLOT(slotRtpReadyRead()));
	
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

		prepareRtpSession();

		if (!m_mediaSession->start())
			QMessageBox::warning(0, tr("Jingle audio"), tr("Unable to start you audio device, the session will start anyway."));
	}
}

void JabberJingleContent::slotReadyRead()
{
	m_rtpSession->packMediaData(m_mediaSession->read());
}

QString JabberJingleContent::elementToSdp(const QDomElement& elem)
{
	Q_UNUSED(elem)
	return QString();
}

QString JabberJingleContent::contentName()
{
	if (!m_content)
		return "";
	return m_content->name();
}

