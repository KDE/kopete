/*
    nlxmms.cpp

    Kopete Now Listening To plugin

    Copyright (c) 2002 by Will Stephenson <will@stevello.free-online.co.uk>

    Kopete    (c) 2002 by the Kopete developers  <kopete-devel@kde.org>
	
	Purpose: 
	This class abstracts the interface to the X Multimedia System (xmms) by
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

#ifdef HAVE_XMMS

#include "nlxmms.h"
#include <kdebug.h>
#include <xmmsctrl.h> // need to fix Makefile.am for this?
#include "nlmediaplayer.h"

NLXmms::NLXmms() : NLMediaPlayer()
{
	m_name = "Xmms";
}


void NLXmms::update()
{
	//look for running xmms
	if ( xmms_remote_get_version( 0 ) )
	{
		QString newTrack;
		// see if it's playing
		if ( xmms_remote_is_playing( 0 ) && !xmms_remote_is_paused( 0 ) )
		{
			m_playing = true;

			// get the artist and album title
			// get the song title
			newTrack = xmms_remote_get_playlist_title( 0, xmms_remote_get_playlist_pos( 0 ) );
			//kDebug( 14307 ) << "NLXmms::update() - track is: " << m_track;
			m_artist = newTrack.section( " - ", 0, 0 );
			newTrack = newTrack.section( " - ", -1, -1 );
		}
		else
			m_playing = false;
		// check if it's a new song
		if ( newTrack != m_track )
		{
			m_newTrack = true;
			m_track = newTrack;
		}
		else
			m_newTrack = false;
		kDebug( 14307 ) << " - found xmms - " << m_track;
	}
	else
		kDebug( 14307 ) << " - xmms not found";
}

#endif 
// vim: set noet ts=4 sts=4 sw=4:
