/*
    nlnoatun.cpp

    Kopete Now Listening To plugin

    Copyright (c) 2002 by Will Stephenson <will@stevello.free-online.co.uk>

    Kopete    (c) 2002 by the Kopete developers  <kopete-devel@kde.org>
	
	Purpose: 
	This class abstracts the interface to Noatun by
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
#include "nlmediaplayer.h"
#include "nlnoatun.h"

NLNoatun::NLNoatun( DCOPClient *client ) : NLMediaPlayer()
{
	m_client = client;
	m_name = "noatun";
	// FIXME - detect current media type in update()
	m_type = Audio;
}

void NLNoatun::update()
{
	// Thanks mETz for telling me about Noatun's currentProperty()
	m_playing = false;
	QString newTrack;
	// see if it's registered with DCOP
	QCString appname = find();
	if ( !appname.isEmpty() )
	{
		// see if it's playing
		QByteArray data, replyData;
		QCString replyType;
		if ( !m_client->call( appname, "Noatun", "state()", data,
					replyType, replyData ) )
		{
			kdDebug( 14307 ) <<  "NLNoatun::update() DCOP error on " << appname << endl;
		}
		else
		{
			QDataStream reply( replyData, IO_ReadOnly );
			if ( replyType == "int" ) {
				int state = 0;
				reply >> state;
				m_playing = ( state == 2 );
				//kdDebug( 14307 ) << "checked if Noatun is playing!" << endl;
			}
		}
		// poll it for its current songtitle, artist and album
		// Using properties
		m_artist = currentProperty( appname, "author" );
		m_album = currentProperty( appname, "album" );
		QString title = currentProperty( appname, "title" );
		// if properties not set ( no id3 tags... ) fallback to filename
		if ( !title.isEmpty() )
			newTrack = title;
		else
			// Using the title() method
			if ( !m_client->call( appname, "Noatun",
						"title()", data, replyType, replyData ) )
				kdDebug( 14307 ) <<  "NLNoatun::update() DCOP error on " << appname 
					<< endl;
			else {
				QDataStream reply( replyData, IO_ReadOnly );
				if ( replyType == "QString" ) {
					reply >> newTrack;
				} else
					kdDebug( 14307 ) << "NLNoatun::update(), title() returned unexpected reply type!" << endl;
			}
		// if the current track title has changed
		if ( newTrack != m_track )
		{
			m_newTrack = true;
			m_track = newTrack;
		}
		else
			m_newTrack = false;
		kdDebug( 14307 ) << "NLNoatun::update() - found "<< appname << " - "
			<< m_track << endl;

	}
	else
		kdDebug( 14307 ) << "NLNoatun::update() - noatun not found" << endl;
}

QCString NLNoatun::find() const
{
	QCString app = "noatun";
	if ( !m_client->isApplicationRegistered( app ) )
	{
		// looking for a registered app prefixed with 'app'
		QCStringList allApps = m_client->registeredApplications();
		QCStringList::iterator it;
		for ( it = allApps.begin(); it != allApps.end(); it++ )
		{
			//kdDebug( 14307 ) << ( *it ) << endl;
			if ( ( *it ).left( 6 ) == app )
			{
				app = ( *it );
				break;
			}
		}
		// not found, set app to ""
		if ( it == allApps.end() )
			app = "";
	}
	return app;
}
		
QString NLNoatun::currentProperty( QCString appname, QString property ) const
{
	QByteArray data, replyData;
	QCString replyType;
	QDataStream arg( data, IO_WriteOnly );
	QString result = "";
	arg << property;
	if ( !m_client->call( appname, "Noatun",
				"currentProperty(QString)", data, replyType, replyData ) )
	{
		kdDebug( 14307 ) <<  "NLNoatun::currentProperty() DCOP error on "
			<< appname << endl;
	}	
	else
	{
		QDataStream reply( replyData, IO_ReadOnly );
		if ( replyType == "QString" )
		{
			reply >> result;
		}
	}
	return result;
}
// vim: set noet ts=4 sts=4 sw=4:
