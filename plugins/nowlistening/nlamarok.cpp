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
#include <qstring.h>

#include "nlmediaplayer.h"
#include "nlamarok.h"

NLamaroK::NLamaroK( DCOPClient *client ) : NLMediaPlayer()
{
	m_client = client;
	m_type = Audio;
	m_name = "Amarok";
}

void NLamaroK::update()
{
	m_playing = false;
	m_newTrack = false;
	QString newTrack;
	QByteArray data, replyData;
	QCString replyType;
	QString result;

	// see if amaroK is  registered with DCOP
	if ( !m_client->isApplicationRegistered( "amarok" ) )
	{
		kdDebug ( 14307 ) << "AmaroK is not running!\n" << endl;
		return;
	}

	// see if it's playing
	// use status() call first, if not supported (amaroK 1.0 or earlier), use isPlaying
	
	if ( !m_client->call( "amarok", "player", "status()", data,
	      replyType, replyData ) )
	{
		kdDebug( 14307 ) << k_funcinfo << " DCOP status() returned error, falling back to isPlaying()." << endl;
		if ( !m_client->call( "amarok", "player", "isPlaying()", data,
					replyType, replyData ) )
		{
			kdDebug( 14307 ) << k_funcinfo << " DCOP error on Amarok." << endl;
		}
		else
		{
			QDataStream reply( replyData, IO_ReadOnly );
			if ( replyType == "bool" ) {
				reply >> m_playing;
			}
		}
	}
	else
	{
		int status = 0;

		QDataStream reply( replyData, IO_ReadOnly );
		if ( replyType == "int" ) {
			reply >> status;
			kdDebug( 14307 ) << k_funcinfo << "Amarok status()=" << status << endl;
		}

		if ( status ) 
		{
			m_playing = true;
		}
	}

	if ( m_client->call( "amarok", "player", "title()", data,
				replyType, replyData ) )
	{
		QDataStream reply( replyData, IO_ReadOnly );

		if ( replyType == "QString" ) {
			reply >> newTrack;
		}
	}

	if ( newTrack != m_track )
	{
		m_newTrack = true;
		m_track = newTrack;
	}

	if ( m_client->call( "amarok", "player", "album()", data,
				replyType, replyData ) )
	{
		QDataStream reply( replyData, IO_ReadOnly );

		if ( replyType == "QString" ) {
			reply >> m_album;
		}
	}

	if ( m_client->call( "amarok", "player", "artist()", data,
				replyType, replyData ) )
	{
		QDataStream reply( replyData, IO_ReadOnly );

		if ( replyType == "QString" ) {
			reply >> m_artist;
		}
	}
}

