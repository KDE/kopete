/*
    nlamarok.cpp

    Kopete Now Listening To plugin

    Copyright (c) 2002,2003 by Will Stephenson <will@stevello.free-online.co.uk>
    Kopete
	Copyright (c) 2002,2003 by the Kopete developers  <kopete-devel@kde.org>
	
	Purpose: 
	This class abstracts the interface to AmaroK by
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

NLAmaroK::NLAmaroK( DCOPClient *client ) : NLMediaPlayer()
{
	m_client = client;
	m_type = Audio;
	m_name = "AmaroK";
}

void NLAmaroK::update()
{
	m_playing = false;
	QString newTrack;

	// see if AmaroK is  registered with DCOP
	if ( m_client->isApplicationRegistered( "amarok" ) )
	{
		// see if it's playing
		QByteArray data, replyData;
		QCString replyType;
		QString result;
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
				kdDebug( 14307 ) << "checked if AmaroK is playing!" << endl;
			}
		}

		if ( m_client->call( "amarok", "player", "nowPlaying()", data,
					replyType, replyData ) )
		{
			m_playing = true;
			QDataStream reply( replyData, IO_ReadOnly );

			if ( replyType == "QString" ) {
				reply >> result;
				m_track = result;
			}
		}
	}
	else
		kdDebug ( 14307 ) << "AmaroK is not running!\n" << endl;
}

