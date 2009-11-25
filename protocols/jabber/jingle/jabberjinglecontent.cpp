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
#include "jinglertpmanager.h"

//Iris
#include "jinglecontent.h"
#include "jinglesession.h"
#include "jingletransport.h"
#include "jingleapplication.h"

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
	if (m_content)
		delete m_content;
	if (m_rtpInSession)
		delete m_rtpInSession;
	if (m_rtpOutSession)
		delete m_rtpOutSession;
	if (m_mediaSession)
		delete m_mediaSession;
}

void JabberJingleContent::setContent(XMPP::JingleContent* content)
{
	m_content = content;

	if (!m_content)
		return;

	m_transport = content->transport();
	m_application = content->application();

	connect(m_content, SIGNAL(established()), SLOT(startStreaming()));
}

void JabberJingleContent::slotReadyRead(int c)
{
	//kDebug() << "Receiving ! (" << data.size() << "bytes)";
	
	m_mediaSession->write(m_transport->readAll((XMPP::JingleTransport::Channel) c), c);
}

void JabberJingleContent::startStreaming()
{
	kDebug() << "Start Streaming";

	if (m_content->application()->mediaType() == XMPP::JingleApplication::Audio)
	{
		m_mediaSession = new /*or MediaRtpSession*/MediaSession(m_mediaManager, "speex"/*FIXME:use m_content->bestPayload()*/);
		if (m_mediaSession == 0)
		{
			kDebug() << "Media Session is NULL!";
			return;
		}
		
		connect(m_mediaSession, SIGNAL(readyRead(int)), SLOT(slotReadySend(int)));
		connect(m_transport, SIGNAL(readyRead(int)), SLOT(slotReadyRead(int)));
		
		m_mediaSession->setSamplingRate(8000 /*FIXME:use m_content->bestPayload()*/);

		if (!m_mediaSession->start())
		{
			QMessageBox::warning(0, tr("Jingle audio"), tr("Unable to start you audio device, terminating the session."));
			//m_content->parentSession()->terminate();
		}
	}
}

void JabberJingleContent::slotReadySend(int c)
{
	if (m_transport)
		m_transport->writeDatagram(m_mediaSession->read(c), (XMPP::JingleTransport::Channel) c);
	
	//m_rtpOutSession->send(m_mediaSession->read());
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

