/*
	nlmpris2.cpp

    Kopete Now Listening To plugin

    Copyright (c) 2012 by Volker Härtel <cyberbeat@gmx.de>

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

#include "nlmpris2.h"

#include <kdebug.h>


#include <QtDBus/QtDBus>


NLmpris2::NLmpris2() : NLMediaPlayer()
{
	m_type = Audio;
	m_name = "MPRIS2 compatible player";
	m_client = 0;
}

NLmpris2::~NLmpris2()
{
	delete m_client;
}

void NLmpris2::update()
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
				// Search for "org.mpris.MediaPlayer2" string
				services = reply.value().filter( "org.mpris.MediaPlayer2" );
			}
		}

		// If no service was found then return false and unlock the mutex
		if( services.isEmpty() )
		{
			return;
		}
		// Start the d-bus interface, needed to check the application status and make calls to it
		if ( m_client != 0 ){
			delete m_client;
			m_client = 0;
		}
		m_client = new QDBusInterface( services.at(0), "/org/mpris/MediaPlayer2", "org.freedesktop.DBus.Properties" );
		QDBusInterface dbusMprisRoot ( services.at(0), "/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2");

		// See if the application is registered.
		if( !m_client->isValid() )
		{
			return;
		}

		if ( !dbusMprisRoot.isValid() )
		{
			m_name = QString( "MPRIS2 compatible player" );
		}
		else
		{
			m_name = dbusMprisRoot.property( "Identity" ).toString();
		}
	}

	// see if it's playing
	QDBusReply <QVariant> mprisStatus = m_client->call ( "Get", "org.mpris.MediaPlayer2.Player", "PlaybackStatus" );
	if (!mprisStatus.isValid())
	{
		return;
	}
	m_playing = ( mprisStatus.value().toString() == "Playing" );

	QDBusReply<QVariant> metaDataReply = m_client->call( "Get", "org.mpris.MediaPlayer2.Player", "Metadata" );
	if ( !metaDataReply.isValid() )
	{
		return;
	}
	if ( !metaDataReply.value().canConvert<QDBusArgument>() )
		return;
	QVariantMap metaData;
	QDBusArgument arg = metaDataReply.value().value<QDBusArgument>();
	arg >> metaData;

	// Fetch title
	const QString newTrack = metaData["xesam:title"].toString();

	if ( newTrack != m_track )
	{
		m_newTrack = true;
		m_track = newTrack;
	}

	// Fetch album
	m_album = metaData["xesam:album"].toString();

	// Fetch artist
	m_artist = metaData["xesam:artist"].toString();

}
