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

QCString NLNoatun::find()
{
	QCString app = "noatun";
	if ( !m_client->isApplicationRegistered( app ) )
	{
		// looking for a registered app prefixed with 'app'
		QCStringList allApps = m_client->registeredApplications();
		QCStringList::iterator it;
		for ( it = allApps.begin(); it != allApps.end(); it++ )
		{
			//kdDebug() << ( *it ) << endl;
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
		
void NLNoatun::update()
{
	// TODO: Fix to use currentProperty - author, title etc
	// Thanks mETz
	//
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
			kdDebug( ) <<  "NLNoatun::update() DCOP error on " << appname << endl;
		}
		else
		{
			QDataStream reply( replyData, IO_ReadOnly );
			if ( replyType == "int" ) {
				int state = 0;
				reply >> state;
				m_playing = ( state == 2 );
				//kdDebug() << "checked if Noatun is playing!" << endl;
			}
		}
		// poll it for its current artist and album
		// 'data' here is an unused input parameter
		// Get the song title 
		if ( !m_client->call( appname, "Noatun",
					"title()", data, replyType, replyData ) )
			kdDebug( ) <<  "NLNoatun::update() DCOP error on " << appname 
				<< endl;
		else {
			QDataStream reply( replyData, IO_ReadOnly );
			if ( replyType == "QString" ) {
				reply >> newTrack;
				//kdDebug() << "NLNoatun::update() the result is: " 
				//	<< newTrack.latin1() << endl;
				// this assumes the format of title() stays the same and
				// song titles do not contain []-()
				// take the artist as the text within the leftmost
				// enclosing pair of []
				m_artist = newTrack.mid( newTrack.find( '[' ) + 1,
						newTrack.findRev( ']' ) - 1 );
				// take the track to be everything after the -, less the
				// rightmost pair of ()
				int lpos = newTrack.find(  '-' ) + 2;
				newTrack = newTrack.mid ( lpos,
						newTrack.findRev( '(' ) - lpos - 1 );
				kdDebug() << newTrack << endl;
				// should be ok for You Never Close Your Eyes ( Any More )
				//newTrack = newTrack.right( newTrack.length() -
			//			newTrack.find( '-' ) + 1 );
//				int rpos = newTrack.length() - newTrack.find(  '-' ) + 1 ;
//				kdDebug() << "NLNOATUN::UPDATE " << newTrack.find ( '-' ) + 1 << " " << newTrack.length() << " " << newTrack.right( newTrack.length() - newTrack.find (  '-' ) - 1 ) <<endl;
//				int lpos = newTrack.findRev( '(' - 1 ) ;
//				kdDebug() << "NLNOATUN::UPDATE " << newTrack.findRev( '(' - 1 ) << " " << newTrack.length() << endl;
			} else
				kdDebug() << "NLNoatun::update(), title() returned unexpected reply type!" << endl;
		}
		// if the current track title has changed
		if ( newTrack != m_track )
		{
			m_newTrack = true;
			m_track = newTrack;
		}
		else
			m_newTrack = false;
		kdDebug() << "NLNoatun::update() - found "<< appname << " - "
			<< m_track << endl;

	}
	else
		kdDebug() << "NLNoatun::update() - noatun not found" << endl;
}
// vim: set noet ts=4 sts=4 sw=4:
