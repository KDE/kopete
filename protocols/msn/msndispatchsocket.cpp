/*
    msndispatchsocket.cpp - Socket for the MSN Dispatch Server

    Copyright (c) 2002-2003 by Martijn Klingens      <klingens@kde.org>
    Copyright (c) 2002-2003 by Olivier Goffart       <ogoffart@tiscalinet.be>

    Kopete    (c) 2002-2003 by the Kopete developers <kopete-devel@kde.org>

    Portions of this code are taken from KMerlin,
              (c) 2001      by Olaf Lueg             <olueg@olsd.de>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "msnaccount.h"
#include "msndispatchsocket.h"

MSNDispatchSocket::MSNDispatchSocket( MSNAccount *account, const QString &msnId, QObject *parent )
: MSNAuthSocket( msnId, parent), m_account( account )
{
}

MSNDispatchSocket::~MSNDispatchSocket()
{
}

void MSNDispatchSocket::connect()
{
	MSNAuthSocket::connect( m_account->serverName(), m_account->serverPort() );
}

void MSNDispatchSocket::parseCommand( const QString &cmd, uint id, const QString &data )
{
	if ( cmd == "XFR" )
	{
		// Got our notification server
		QString host = data.section( ' ', 1, 1 );
		QString server = host.section( ':', 0, 0 );
		uint port = host.section( ':', 1, 1 ).toUInt();
		setOnlineStatus( Connected );
		emit receivedNotificationServer( server, port );
		disconnect();
	}
	else
	{
		// Let the base class handle the rest
		MSNAuthSocket::parseCommand( cmd, id, data );
	}
}

#include "msndispatchsocket.moc"

// vim: set noet ts=4 sts=4 sw=4:

