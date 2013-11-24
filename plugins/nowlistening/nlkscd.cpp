/*
    nlkscd.cpp

    Kopete Now Listening To plugin

    Copyright (c) 2002,2003,2004 by Will Stephenson <will@stevello.free-online.co.uk>

    Kopete    (c) 2002,2003,2004 by the Kopete developers  <kopete-devel@kde.org>
	
	Purpose: 
	This class abstracts the interface to KsCD by
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

#include "nlkscd.h"

#include <kdebug.h>
#include <QStringList>

#include <QtDBus/QtDBus>

#include "nlmediaplayer.h"

NLKscd::NLKscd() : NLMediaPlayer()
{
	m_client = new QDBusInterface("org.kde.kscd", "/CDPlayer");
	m_type = Audio;
	m_name = "KsCD";
}

NLKscd::~NLKscd()
{
	delete m_client;
}

void NLKscd::update()
{
	m_playing = false;
	QString newTrack;

	if (!m_client->isValid())
	{
		delete m_client;
		m_client = new QDBusInterface("org.kde.kscd", "/CDPlayer");
	}
	// see if it's registered with D-BUS
	if ( m_client->isValid() )
	{
		// see if it's playing
		QDBusReply<bool> playingReply = m_client->call("playing");
		if( playingReply.isValid() )
		{
			m_playing = playingReply.value();
		}

		// poll it for its current artist 
		QDBusReply<QString> artistReply = m_client->call("currentArtist");
		if( artistReply.isValid() )
		{
			m_artist = artistReply.value();
		}

		//album
		QDBusReply<QString> albumReply = m_client->call("currentAlbum");
		if( albumReply.isValid() )
		{
			m_album = albumReply.value();
		}

		// Get the current track title
		QDBusReply<QString> trackReply = m_client->call("currentTrackTitle");
		if( trackReply.isValid() )
		{
			newTrack = trackReply.value();
		}

		// if the current track title has changed
		if ( newTrack != m_track )
		{
			m_newTrack = true;
			m_track = newTrack;
		}
		else
			m_newTrack = false;
//		kDebug( 14307 ) << "NLKscd::update() - found kscd - "
//			<< m_track << endl;

	}
//	else
//		kDebug( 14307 ) << "NLKscd::update() - kscd not found";
}

// vim: set noet ts=4 sts=4 sw=4:
