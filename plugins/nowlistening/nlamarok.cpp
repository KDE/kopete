/*
    nlamarok.cpp

    Kopete Now Listening To plugin

    Copyright (c) 2002,2003,2004 by Will Stephenson <will@stevello.free-online.co.uk>
    Kopete
	Copyright (c) 2002,2003,2004 by the Kopete developers  <kopete-devel@kde.org>

	Purpose:
	This class abstracts the interface to amaroK by
	implementing NLMediaPlayer

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "nlamarok.h"

#include <kdebug.h>


#include <QtDBus/QtDBus>

#include "nlmediaplayer.h"

// from kvirc mpris interface
struct MPRISPlayerStatus
{
          int Play;
          int Random;
          int RepeatCurrent;
          int RepeatPlaylist;
};
Q_DECLARE_METATYPE( MPRISPlayerStatus )

QDBusArgument &operator<<(QDBusArgument &argument, const MPRISPlayerStatus &status)
{
	argument.beginStructure();
	argument << status.Play << status.Random << status.RepeatCurrent << status.RepeatPlaylist;
	argument.endStructure();
	return argument;
}
  
const QDBusArgument &operator>>(const QDBusArgument &argument, MPRISPlayerStatus &status)
{
	argument.beginStructure();
	argument >> status.Play >> status.Random >> status.RepeatCurrent >> status.RepeatPlaylist;
	argument.endStructure();
	return argument;
}

NLamaroK::NLamaroK() : NLMediaPlayer()
{
	m_type = Audio;
	m_name = "amaroK";
	m_client = new QDBusInterface("org.mpris.amarok", "/Player", "org.freedesktop.MediaPlayer");
	qDBusRegisterMetaType<MPRISPlayerStatus>();
}

NLamaroK::~NLamaroK()
{
	delete m_client;
}

void NLamaroK::update()
{
	m_playing = false;
	m_newTrack = false;
	QString newTrack;
	QString result;

	if (!m_client->isValid())
	{
		delete m_client;
		m_client = new QDBusInterface("org.mpris.amarok", "/Player", "org.freedesktop.MediaPlayer");
	}
	if( !m_client->isValid() )
		return;

	// See if amarok is currently playing.
	QDBusReply<MPRISPlayerStatus> statusReply = m_client->call("GetStatus");
	if(statusReply.isValid())
	{
		// 0 = playing, 1 = paused, 2 = stopped
		if(statusReply.value().Play == 0)
			m_playing = true;
		else {
			m_playing = false;
			m_newTrack = true;
			m_track = "";
                        m_album = "";
			m_artist = "";
			return;
		}
	}
	else
		m_playing = false;

	QDBusReply<QVariantMap> metaDataReply = m_client->call("GetMetadata");
	if (!metaDataReply.isValid())
	{
		return;
	}

	const QVariantMap &metaData = metaDataReply.value();

	// Fetch title
	newTrack = metaData["title"].toString();

	if ( newTrack != m_track )
	{
		m_newTrack = true;
		m_track = newTrack;
	}

	// Fetch album
	m_album = metaData["album"].toString();

	// Fetch artist
	m_artist = metaData["artist"].toString();
}
