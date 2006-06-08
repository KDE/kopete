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

#include <kdebug.h>
#include <QString>

#include <dbus/qdbus.h>

#include "nlmediaplayer.h"
#include "nlamarok.h"

NLamaroK::NLamaroK() : NLMediaPlayer()
{
	m_type = Audio;
	m_name = "amaroK";
	m_client = new QDBusInterfacePtr("org.kde.amaroK", "/player");
}

QDBusInterface *NLamaroK::client()
{
	return m_client->interface();
}

void NLamaroK::update()
{
	m_playing = false;
	m_newTrack = false;
	QString newTrack;
	QString result;

	// TODO: Port to amarok 2.0 D-BUS interface
	if( !client()->isValid() )
		return;

	// See if amaroK is currently playing.
	QDBusReply<int> statusReply = client()->call("status");
	if( statusReply.isSuccess() )
	{
		if( statusReply.value() )
		{
			m_playing = true;
		}
	}

	// Fetch title
	QDBusReply<QString> newTrackReply = client()->call("title");
	if( newTrackReply.isSuccess() )
	{
		newTrack = newTrackReply.value();
	}

	if ( newTrack != m_track )
	{
		m_newTrack = true;
		m_track = newTrack;
	}

	// Fetch album
	QDBusReply<QString> albumReply = client()->call("album");
	if( albumReply.isSuccess() )
	{
		m_album = albumReply.value();
	}

	// Fetch artist
	QDBusReply<QString> artistReply = client()->call("artist");
	if( artistReply.isSuccess() )
	{
		m_artist = artistReply.value();
	}
}

