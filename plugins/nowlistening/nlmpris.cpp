/*
	nlmpris.cpp

    Kopete Now Listening To plugin

    Copyright (c) 2010 by Volker Härtel <cyberbeat@gmx.de>

    Kopete (c) 2002,2003 by the Kopete developers  <kopete-devel@kde.org>

	Purpose:
	This class abstracts the interface to mpris by
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

#include "nlmpris.h"

#include <kdebug.h>


#include <QtDBus/QtDBus>

struct mprisPlayerStatus
{
    int state;  // 0 = Playing, 1 = Paused, 2 = Stopped.
    int random; // 0 = Playing linearly, 1 = Playing randomly.
    int repeat; // 0 = Go to the next element once the current has finished playing, 1 = Repeat the current element
    int repeatPlayList; // 0 = Stop playing once the last element has been played, 1 = Never give up playing
};

Q_DECLARE_METATYPE( mprisPlayerStatus )

QDBusArgument &operator << ( QDBusArgument &arg, const mprisPlayerStatus &status )
{
    arg.beginStructure();
    arg << status.state;
    arg << status.random;
    arg << status.repeat;
    arg << status.repeatPlayList;
    arg.endStructure();
    return arg;
}

const QDBusArgument &operator >> ( const QDBusArgument &arg, mprisPlayerStatus &status )
{
    arg.beginStructure();
    arg >> status.state;
    arg >> status.random;
    arg >> status.repeat;
    arg >> status.repeatPlayList;
    arg.endStructure();
    return arg;
}

NLmpris::NLmpris() : NLMediaPlayer()
{
	m_type = Audio;
	m_name = "MPRIS compatible player";
	m_client = 0;
	qDBusRegisterMetaType<mprisPlayerStatus>();
}

NLmpris::~NLmpris()
{
	delete m_client;
}

void NLmpris::update()
{
	m_playing = false;

	if( m_client == 0 || !m_client->isValid() ) {

		QStringList services;
		const QDBusConnection& sessionConn( QDBusConnection::sessionBus() );

		// Check if the connection is successful
		if( sessionConn.isConnected() )
		{
			const QDBusConnectionInterface* bus = sessionConn.interface();
			const QDBusReply<QStringList>& reply( bus->registeredServiceNames() );

			if( reply.isValid() )
			{
				// Search for "org.mpris" string
				services = reply.value().filter( "org.mpris." );
			}
		}

		// If no service was found then return false and unlock the mutex
		if( services.isEmpty() )
		{
			return;
		}

		// Start the d-bus interface, needed to check the application status and make calls to it
		if (m_client != 0){
			delete m_client;
			m_client = 0;
		}
		m_client = new QDBusInterface( services.at(0), "/Player", "org.freedesktop.MediaPlayer" );
		QDBusInterface dbusMprisRoot  ( services.at(0), "/", "org.freedesktop.MediaPlayer" );

		// See if the application is registered.
		if( ! m_client->isValid() )
		{
			return;
		}


		if (! dbusMprisRoot.isValid() )
		{
			m_name = QString( "MPRIS compatible player" );
		}
		else
		{
			// Identity is part of /, not /Player.
			QDBusReply<QString> playerName = dbusMprisRoot.call("Identity");
			m_name = playerName.value();

		}

	}

	// see if it's playing
	QDBusReply <mprisPlayerStatus> mprisStatus = m_client->call ( "GetStatus" );
	if ( mprisStatus.value().state == 0 )
	{
		m_playing = true;
	}

	QDBusReply<QVariantMap> metaDataReply = m_client->call ( "GetMetadata" );
	if ( !metaDataReply.isValid() )
	{
		return;
	}

	const QVariantMap &metaData = metaDataReply.value();

	// Fetch title
	const QString newTrack = metaData["title"].toString();

	if ( newTrack != m_track )
	{
		m_newTrack = true;
		m_track = newTrack;
	}

	// Fetch album
	m_album = metaData["album"].toString();

	// Fetch artist
	m_artist = metaData["artist"].toString();

}
