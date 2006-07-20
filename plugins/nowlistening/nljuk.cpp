/*
    nljuk.cpp

    Kopete Now Listening To plugin

    Copyright (c) 2002,2003,2004 by Will Stephenson <will@stevello.free-online.co.uk>
    Copyright (c) 2003 by Ismail Donmez <ismail.donmez@boun.edu.tr>
    Copyright (c) 2002,2003 by the Kopete developers  <kopete-devel@kde.org>
	
	Purpose: 
	This class abstracts the interface to JuK by
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
#include "nljuk.h"

NLJuk::NLJuk() : NLMediaPlayer()
{
	m_type = Audio;
	m_name = "JuK";
	m_client = new QDBusInterfacePtr("org.kde.JuK", "/Player");
}

QDBusInterface *NLJuk::client()
{
	return m_client->interface();
}

void NLJuk::update()
{
	m_playing = false;
	QString newTrack;

	// TODO: Port to JuK D-BUS interface

	// see if JuK is  registered with DCOP
	if( client()->isValid() )
	{
		// see if it's playing

		QDBusReply<bool> playingReply = client()->call("playing");
		if( playingReply.isSuccess() )
		{
			m_playing = playingReply.value();
		}

	
		QDBusReply<QString> albumReply = client()->call( "trackProperty", QString("Album") );
		if( albumReply.isSuccess() )
		{
			m_album = albumReply.value();
		}

		QDBusReply<QString> artistReply = client()->call( "trackProperty", QString("Artist") );
		if( artistReply.isSuccess() )
		{
			m_artist = artistReply.value();
		}

		QDBusReply<QString> titleReply = client()->call( "trackProperty", QString("Title") );
		if( titleReply.isSuccess() )
		{
			newTrack = titleReply.value();
		}

		if ( newTrack != m_track )
		{
			m_newTrack = true;
			m_track = newTrack;
		}
		else
			m_newTrack = false;
	}
	else
		kDebug( 14307 ) << "Juk is not running!\n" << endl;
}

