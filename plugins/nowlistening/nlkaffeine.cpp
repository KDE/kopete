/*
    nlkaffeine.cpp

    Kopete Now Listening To plugin

    Copyright (c) 2004 by Will Stephenson <will@stevello.free-online.co.uk>
    Kopete
	Copyright (c) 2002,2003,2004 by the Kopete developers  <kopete-devel@kde.org>
	
	Purpose: 
	This class abstracts the interface to Kaffeine by
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
#include "nlkaffeine.h"

NLKaffeine::NLKaffeine() : NLMediaPlayer()
{
	m_client = new QDBusInterfacePtr("org.kde.Kaffeine", "/KaffeineIface");
	m_type = Video;
	m_name = "Kaffeine";
}

QDBusInterface *NLKaffeine::client()
{
	return m_client->interface();
}

void NLKaffeine::update()
{
	m_playing = false;
	m_newTrack = false;
	QString newTrack;

	// TODO: Port to Kaffeine D-BUS Interface
	// see if kaffeine is  registered with D-BUS
	if ( client()->isValid() )
	{

		QDBusReply<bool> isPlayingReply = client()->call("isPlaying");
		if( isPlayingReply.isSuccess() )
		{
			m_playing = isPlayingReply.value();
		}

		QDBusReply<QString> getTrackReply = client()->call("getTrack");
		if( getTrackReply.isSuccess() )
		{
			newTrack = getTrackReply.value();
		}

		if( newTrack != m_track )
		{
			m_newTrack = true;
			m_track = newTrack;
		}
	}
	else
		kDebug ( 14307 ) << "Kaffeine is not running!\n" << endl;
}

