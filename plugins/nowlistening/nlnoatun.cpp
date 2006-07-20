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

#include <QLatin1String>

#include "nlmediaplayer.h"
#include "nlnoatun.h"

#include <dbus/qdbus.h>

NLNoatun::NLNoatun() : NLMediaPlayer()
{
	m_client = new QDBusInterfacePtr("org.kde.noatun", "/Noatun");
	m_name = "noatun";
	// FIXME - detect current media type in update()
	m_type = Audio;
}

QDBusInterface *NLNoatun::client()
{
	return m_client->interface();
}

void NLNoatun::update()
{
	// Thanks mETz for telling me about Noatun's currentProperty()
	m_playing = false;
	QString newTrack;

	// TODO: Port to Noatun D-BUS Interface
	if ( client()->isValid() )
	{
		// see if it's playing
		QDBusReply<int> stateReply = client()->call("state");
		if( stateReply.isSuccess() )
		{
			m_playing = ( stateReply.value() == 2 );
		}

		// poll it for its current songtitle, artist and album
		// Using properties
		m_artist = currentProperty( QLatin1String("author") );
		m_album = currentProperty( QLatin1String("album") );
		QString title = currentProperty( QLatin1String("title") );
		// if properties not set ( no id3 tags... ) fallback to filename
		if ( !title.isEmpty() )
			newTrack = title;
		else
		{
			QDBusReply<QString> titleReply = client()->call("title");
			if( titleReply.isSuccess() )
			{
				newTrack = titleReply.value();
			}
		}

		// if the current track title has changed
		if ( newTrack != m_track )
		{
			m_newTrack = true;
			m_track = newTrack;
		}
		else
			m_newTrack = false;
// 		kDebug( 14307 ) << "NLNoatun::update() - found "<< appname << " - "
// 			<< m_track << endl;

	}
	else
		kDebug( 14307 ) << "NLNoatun::update() - noatun not found" << endl;
}

		
QString NLNoatun::currentProperty(const QString &property)
{
	QString result;

	QDBusReply<QString> propertyReply = client()->call("currentProperty", property);
	
	if( propertyReply.isSuccess() )
	{
		result = propertyReply.value();
	}

	return result;
}
// vim: set noet ts=4 sts=4 sw=4:
