/*
    msnauthsocket.cpp - Socket that does the initial handshake as used by
                         both MSNAuthSocket and MSNNotifySocket

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

#include "msnauthsocket.h"

#include <qtimer.h>

#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>

MSNAuthSocket::MSNAuthSocket( const QString &msnId )
{
	m_msnId = msnId;
	m_msgBoxShown = false;
}

MSNAuthSocket::~MSNAuthSocket()
{
}

void MSNAuthSocket::reconnect()
{
	connect( server(),port() );
}

void MSNAuthSocket::handleError( uint code, uint id )
{
	if( code != 600 )
	{
		MSNSocket::handleError( code, id );
		return;
	}

	disconnect();
	QTimer::singleShot( 10, this, SLOT( reconnect() ) );

	if( !m_msgBoxShown )
	{
		QString msg =
			i18n( "The MSN server is busy.\n"
				"Trying to reconnect every 10 seconds. Please be pateint..." );
		m_msgBoxShown = true;

		KMessageBox::information( 0, msg, i18n( "MSN Plugin - Kopete" ) );
	}
}

void MSNAuthSocket::parseCommand( const QString &cmd, uint id,
	const QString &data )
{
	if( cmd == "VER" )
	{
		kdDebug() << "MSNAuthSocket: Requesting authentication method"
			<< endl;
		sendCommand( "INF" );
	}
	else if( cmd == "INF" )
	{
		kdDebug() << "MSNAuthSocket: Requesting MD5 authentication "
			<< "for Passport " << m_msnId << endl;
		sendCommand( "USR", "MD5 I " + m_msnId);
	}
	else
	{
		kdDebug() << "MSNAuthSocket::parseCommand: Unexpected response '"
			<< cmd << " " << id << " " << data << "' from server!" << endl;
	}
}

void MSNAuthSocket::doneConnect()
{
	kdDebug() << "MSNAuthSocket: Negotiating server protocol version"
		<< endl;
	sendCommand( "VER", "MSNP7 MSNP6 MSNP5 MSNP4 CVR0" );
}

#include "msnauthsocket.moc"

// vim: set noet ts=4 sts=4 sw=4:

