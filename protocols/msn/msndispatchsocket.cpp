/*
    msndispatchsocket.cpp - Socket for the MSN Dispatch Server

    Copyright (c) 2002 by Martijn Klingens       <klingens@kde.org>
    Kopete    (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

    Portions of this code are taken from KMerlin,
              (c) 2001 by Olaf Lueg              <olueg@olsd.de>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "msndispatchsocket.h"

#include <qtimer.h>

#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>

MSNDispatchSocket::MSNDispatchSocket( const QString &msnId )
{
	m_msnId = msnId;
	m_msgBoxShown = false;
}

MSNDispatchSocket::~MSNDispatchSocket()
{
}

void MSNDispatchSocket::connect()
{
	QObject::connect( this, SIGNAL( onlineStatusChanged( OnlineStatus ) ),
		this, SLOT( slotStatusChanged( OnlineStatus ) ) );
	MSNSocket::connect( "messenger.hotmail.com", 1863 );
}

void MSNDispatchSocket::handleError( uint code, uint id )
{
	if( code != 600 )
	{
		MSNSocket::handleError( code, id );
		return;
	}

	disconnect();
	QTimer::singleShot( 10, this, SLOT( connect() ) );

	if( !m_msgBoxShown )
	{
		QString msg =
			i18n( "The MSN server is busy.\n"
				"Trying to reconnect every 10 seconds. Please be pateint..." );
		m_msgBoxShown = true;

		KMessageBox::information( 0, msg, i18n( "MSN Plugin - Kopete" ) );
	}
}

void MSNDispatchSocket::parseCommand( const QString &cmd, uint id,
	const QString &data )
{
	if( cmd == "VER" )
	{
		kdDebug() << "MSNDispatchSocket: Requesting authentication method"
			<< endl;
		sendCommand( "INF" );
	}
	else if( cmd == "INF" )
	{
		kdDebug() << "MSNDispatchSocket: Requesting MD5 authentication "
			<< "for Passport " << m_msnId << endl;
		sendCommand( "USR", "MD5 I " + m_msnId );
	}
	else if( cmd == "XFR" )
	{
		// Got our notification server
		QString host = data.section( ' ', 1, 1 );
		QString server = host.section( ':', 0, 0 );
		uint port = host.section( ':', 1, 1 ).toUInt();
		disconnect();
		emit receivedNotificationServer( server, port );
	}
	else
	{
		kdDebug() << "MSNDispatchSocket::parseCommand: Unexpected response '"
			<< cmd << " " << id << " " << data << "' from server!" << endl;
	}
}

void MSNDispatchSocket::slotStatusChanged( OnlineStatus status )
{
	if( status == Connected )
	{
		kdDebug() << "MSNDispatchSocket: Negotiating server protocol version"
			<< endl;
		sendCommand( "VER", "MSNP7 MSNP6 MSNP5 MSNP4 CVR0" );
	}
}

#include "msndispatchsocket.moc"

// vim: set noet ts=4 sts=4 sw=4:

