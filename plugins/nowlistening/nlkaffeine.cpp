/*
    nlkaffeine.cpp

    Kopete Now Listening To plugin

    Copyright (c) 2004 by Will Stephenson <will@stevello.free-online.co.uk>
    Kopete
	Copyright (c) 2002,2003,2004 by the Kopete developers  <kopete-devel@kde.org>
	
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
#include "nlkaffeine.h"

NLKaffeine::NLKaffeine( DCOPClient *client ) : NLMediaPlayer()
{
	m_client = client;
	m_type = Video;
	m_name = "Kaffeine";
}

void NLKaffeine::update()
{
	m_playing = false;
	QString newTrack;

	// see if kaffeine is  registered with DCOP
	if ( m_client->isApplicationRegistered( "kaffeine" ) )
	{
		// see if it's playing
		QByteArray data, replyData;
		QCString replyType;
		QString result;
		if ( !m_client->call( "kaffeine", "Kaffeine", "isPlaying()", data,
					replyType, replyData ) )
		{
			kdDebug( 14307 ) << k_funcinfo << " DCOP error on Kaffeine." << endl;
		}
		else
		{
			QDataStream reply( replyData, IO_ReadOnly );
			if ( replyType == "bool" ) {
				reply >> m_playing;
				kdDebug( 14307 ) << "checked if Kaffeine is playing!" << endl;
			}
		}

		if ( m_client->call( "kaffeine", "Kaffeine", "getTitle()", data,
					replyType, replyData ) )
		{
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

