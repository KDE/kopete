 /*
  * jabberjinglesession.cpp - A Jingle session.
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
#include "jabberjinglesession.h"
#include "jabberjinglecontent.h"
#include "mediamanager.h"
#include "mediasession.h"
#include "jinglecallsmanager.h"

#include "jinglesession.h"
#include "jinglecontent.h"

#include <KDebug>

//using namespace XMPP;

JabberJingleSession::JabberJingleSession(JingleCallsManager* parent)
: m_callsManager(parent)
{
	m_mediaManager = m_callsManager->mediaManager();
	qDebug() << "Created a new JabberJingleSession";
	m_timeUp = QTime(0, 0);
	QTimer *uptime = new QTimer(this);
	connect(uptime, SIGNAL(timeout()), this, SLOT(slotUptimeOut()));
	uptime->start(1000);
}

JabberJingleSession::~JabberJingleSession()
{
	kDebug() << "destroyed";
	for (int i = 0; i < jabberJingleContents.count(); i++)
		delete jabberJingleContents[i];
	delete m_jingleSession;
}

void JabberJingleSession::slotUptimeOut()
{
	m_timeUp.addSecs(1);
}

void JabberJingleSession::setJingleSession(XMPP::JingleSession* sess)
{
	qDebug() << "Setting JingleSession in the JabberJingleSession :" << sess;
	m_jingleSession = sess;
	connect(sess, SIGNAL(stateChanged()), this, SLOT(slotStateChanged()));
	connect(sess, SIGNAL(terminated()), this, SLOT(slotSessionTerminated()));
	// Create Contents :
	for (int i = 0; i < sess->contents().count(); i++)
	{
		JabberJingleContent *jContent = new JabberJingleContent(this, sess->contents()[i]);
		jabberJingleContents << jContent;
	}
}

void JabberJingleSession::slotStateChanged()
{
	if (m_jingleSession->state() != XMPP::JingleSession::Active)
		return;

	for (int i = 0; i < m_jingleSession->contents().count(); i++)
	{
		JabberJingleContent *jContent = contentWithName(m_jingleSession->contents()[i]->name());
		if (jContent == 0)
		{
			qDebug() << "Create JabberJingleContent for content" << m_jingleSession->contents()[i]->name();
			jContent = new JabberJingleContent(this, m_jingleSession->contents()[i]);
			jabberJingleContents << jContent;
		}
		jContent->prepareRtpInSession();
		jContent->prepareRtpOutSession();
		jContent->startWritingRtpData();
		//FIXME:Those 3 methods should be set in a method like startStreaming()
	}
}

void JabberJingleSession::slotSessionTerminated()
{
	for (int i = 0; i < jabberJingleContents.count(); i++)
		delete jabberJingleContents.takeAt(i);
	
	emit terminated();
}

void JabberJingleSession::writeRtpData(XMPP::JingleContent* content)
{
	qDebug() << "Called void JabberJingleSession::writeRtpData(XMPP::JingleContent* content)";
	JabberJingleContent *jContent = contentWithName(content->name());
	if (jContent == 0)
	{
		jContent = new JabberJingleContent(this, content);
		jabberJingleContents << jContent;
	}
	jContent->startWritingRtpData();
	//FIXME:need different m_rtpSession for each content.
}

JabberJingleContent *JabberJingleSession::contentWithName(const QString& name)
{
	for (int i = 0; i < jabberJingleContents.count(); i++)
	{
		if (jabberJingleContents[i]->contentName() == name)
			return jabberJingleContents[i];
	}
	return 0;
}

MediaManager *JabberJingleSession::mediaManager() const
{
	kDebug(KDE_DEFAULT_DEBUG_AREA) << "m_mediaManager is" << (m_mediaManager == 0 ? "Null" : "Valid");
	return m_mediaManager;
}

QTime JabberJingleSession::upTime()
{
	//TODO:Implement me !
	return m_timeUp;
}

