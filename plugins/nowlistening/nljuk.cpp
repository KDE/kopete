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

#include "nljuk.h"

#include <kdebug.h>


#include <QtDBus/QtDBus>

#include "nlmediaplayer.h"

NLJuk::NLJuk() : NLMediaPlayer()
{
	m_type = Audio;
	m_name = "JuK";
	m_client = new QDBusInterface("org.kde.juk", "/Player");
}

NLJuk::~NLJuk()
{
	delete m_client;
}

void NLJuk::update()
{
	m_playing = false;
	QString newTrack;
	if (!m_client->isValid())
	{
		delete m_client;
		m_client = new QDBusInterface("org.kde.juk", "/Player");
	}
	// see if JuK is registered with DBUS
	if( m_client->isValid() )
	{
		// see if it's playing
		QDBusReply<bool> playingReply = m_client->call("playing");
		if( playingReply.isValid() )
		{
			m_playing = playingReply.value();
		}

	
		QDBusReply<QString> albumReply = m_client->call( "trackProperty", QString("Album") );
		if( albumReply.isValid() )
		{
			m_album = albumReply.value();
		}

		QDBusReply<QString> artistReply = m_client->call( "trackProperty", QString("Artist") );
		if( artistReply.isValid() )
		{
			m_artist = artistReply.value();
		}

		QDBusReply<QString> titleReply = m_client->call( "trackProperty", QString("Title") );
		if( titleReply.isValid() )
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
		kDebug( 14307 ) << "Juk is not running!\n";
}

