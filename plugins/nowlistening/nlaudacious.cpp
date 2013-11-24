/*
	nlaudacious.cpp

    Kopete Now Listening To plugin

    Copyright (c) 2002,2003,2004 by Will Stephenson <will@stevello.free-online.co.uk>
    Copyright (c) 2009 by Attila Herman <attila589/at/gmail.com>

    Kopete (c) 2002,2003 by the Kopete developers  <kopete-devel@kde.org>

	Purpose:
	This class abstracts the interface to audacious by
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

#include "nlaudacious.h"

#include <kdebug.h>


#include <QtDBus/QtDBus>

struct audaciousPlayerStatus
{
    int state;  // 0 = Playing, 1 = Paused, 2 = Stopped.
    int random; // 0 = Playing linearly, 1 = Playing randomly.
    int repeat; // 0 = Go to the next element once the current has finished playing, 1 = Repeat the current element
    int repeatPlayList; // 0 = Stop playing once the last element has been played, 1 = Never give up playing
};

Q_DECLARE_METATYPE( audaciousPlayerStatus )

QDBusArgument &operator << ( QDBusArgument &arg, const audaciousPlayerStatus &status )
{
    arg.beginStructure();
    arg << status.state;
    arg << status.random;
    arg << status.repeat;
    arg << status.repeatPlayList;
    arg.endStructure();
    return arg;
}

const QDBusArgument &operator >> ( const QDBusArgument &arg, audaciousPlayerStatus &status )
{
    arg.beginStructure();
    arg >> status.state;
    arg >> status.random;
    arg >> status.repeat;
    arg >> status.repeatPlayList;
    arg.endStructure();
    return arg;
}

NLaudacious::NLaudacious() : NLMediaPlayer()
{
	m_type = Audio;
	m_name = "audacious";
	m_client = new QDBusInterface ( "org.mpris.audacious", "/Player" );
	qDBusRegisterMetaType<audaciousPlayerStatus>();
}

NLaudacious::~NLaudacious()
{
	delete m_client;
}

void NLaudacious::update()
{
	m_playing = false;

	if ( !m_client->isValid() )
	{
		delete m_client;
		m_client = new QDBusInterface ( "org.mpris.audacious", "/Player", "org.freedesktop.MediaPlayer" );
	}

	// see if audacious is registered with DBUS
	if ( m_client->isValid() )
	{
		// see if it's playing
		QDBusReply <audaciousPlayerStatus> audaciousStatus = m_client->call ( "GetStatus" );
		if ( audaciousStatus.value().state == 0 )
		{
			m_playing = true;
		}

		QDBusReply<QVariantMap> metaDataReply = m_client->call ( "GetMetadata" );
		if ( !metaDataReply.isValid() )
		{
			return;
		}

		const QVariantMap &metaData = metaDataReply.value();

		// Fetch title
		const QString newTrack = metaData["title"].toString();

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

}
