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
#include "rtpsession.h"

//Iris
#include "jinglecontent.h"
#include "jinglesession.h"

JabberJingleContent::JabberJingleContent(JabberJingleSession* parent, XMPP::JingleContent* c)
 : m_content(c), m_jabberSession(parent)
{
	m_rtpSession = 0;
	m_mediaManager = m_jabberSession->mediaManager();
	if (!m_mediaManager)
		kDebug(KDE_DEFAULT_DEBUG_AREA) << "m_mediaManager is Null !!!!!!!!!!!!!!!!!!!!!!!!!!";
	if (c == 0)
		return;
	if (m_content->socket() == 0 || !m_content->socket()->isValid())
		connect(c, SIGNAL(socketReady()), this, SLOT(slotPrepareRtpSession()));
	else
		slotPrepareRtpSession();
	kDebug(KDE_DEFAULT_DEBUG_AREA) << "Created a new JabberJingleContent with" << c->name();
}

JabberJingleContent::~JabberJingleContent()
{
	delete m_content;
	delete m_rtpSession;
}

void JabberJingleContent::setContent(XMPP::JingleContent* content)
{
	m_content = content;
	connect(m_content, SIGNAL(socketReady()), this, SLOT(slotPrepareRtpSession()));
}

void JabberJingleContent::slotPrepareRtpSession()
{
	kDebug(KDE_DEFAULT_DEBUG_AREA) << "Prepare RTP session";
	if (m_rtpSession == 0)
	{
		m_rtpSession = new JingleRtpSession();
		if (!m_content->socket())
		{
			kDebug() << "Fatal : Invalid Socket !";
			return;
		}
		m_rtpSession->setRtpSocket(m_content->socket()); // This will set rtcp port = rtp port + 1. Maybe we don't want that for ice-udp.
	}
}

void JabberJingleContent::startWritingRtpData()
{
	qDebug() << "Start Writing Rtp Data.";
	
	slotPrepareRtpSession();

	if (m_jabberSession->session()->state() == XMPP::JingleSession::Pending)
	{
		m_rtpSession->setPayload(elementToSdp(bestPayload(m_content->payloadTypes(), m_mediaManager->payloads())));
	}
	if (m_content->dataType() == XMPP::JingleContent::Audio)
	{
		connect(m_mediaManager, SIGNAL(audioReadyRead()), this, SLOT(slotSendRtpData()));
		m_mediaManager->startAudioStreaming();
	}
}

QString JabberJingleContent::elementToSdp(const QDomElement& elem)
{
	return QString();
}

void JabberJingleContent::slotSendRtpData()
{
	kDebug() << "Send RTP data.";
	m_rtpSession->send(m_mediaManager->data());
}

QString JabberJingleContent::contentName()
{
	if (!m_content)
		return "";
	return m_content->name();
}

QDomElement JabberJingleContent::bestPayload(const QList<QDomElement>& payload1, const QList<QDomElement>& payload2)
{
	for (int i = 0; i < payload1.count(); i++)
	{
		for (int j = 0; j < payload2.count(); j++)
		{
			if (samePayload(payload1[i], payload2[j]))
				return payload1[i];
		}
	}
	return QDomElement();
}

bool JabberJingleContent::samePayload(const QDomElement& p1, const QDomElement& p2)
{
	// Checking payload-type attributes
	if (!p1.hasAttribute("id") || !p2.hasAttribute("id"))
		return false;
	if (p1.attribute("id") != p2.attribute("id"))
		return false;
	int id = p1.attribute("id").toInt();
	if ((id >= 96) && (id <= 127)) //dynamic payloads, "name" attribute must be there
	{
		if (!p1.hasAttribute("name") || !p2.hasAttribute("name"))
			return false;
		if (p1.attribute("name") != p2.attribute("name"))
			return false;
	}
	if (p1.hasAttribute("channels") && p2.hasAttribute("channels"))
		if (p1.attribute("channels") != p2.attribute("channels"))
			return false;
	if (p1.hasAttribute("clockrate") && p2.hasAttribute("clockrate"))
		if (p1.attribute("clockrate") != p2.attribute("clockrate"))
			return false;
	// Parameters are informative, even if they differ, the payload is stil the same.
	kDebug() << "Payloads are the same.";
	return true;
}
