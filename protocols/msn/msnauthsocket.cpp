/*
    msnauthsocket.cpp - Socket that does the initial handshake as used by
                        both MSNAuthSocket and MSNNotifySocket

    Copyright (c) 2002-2003 by Martijn Klingens      <klingens@kde.org>
    Copyright (c) 2002-2004 by Olivier Goffart       <ogoffart@tiscalinet.be>

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

#include "msnauthsocket.h"

#include "kopeteuiglobal.h"

#include <kaboutdata.h>
#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>

#include <sys/utsname.h>

MSNAuthSocket::MSNAuthSocket( const QString &msnId, QObject *parent )
: MSNSocket( parent )
{
	m_msnId = msnId;
	m_msgBoxShown = false;
	m_badPassword = false;
}

MSNAuthSocket::~MSNAuthSocket()
{
}

void MSNAuthSocket::reconnect()
{
	//connect( server(),port() );
}

void MSNAuthSocket::handleError( uint code, uint id )
{
	switch ( code )
	{
	case 600:
	case 910:
	case 912:
	{
		disconnect();
/*
		// FIXME: when disconnecting, this is deleted. impossible to reconnect
		// QTimer::singleShot( 10, this, SLOT( reconnect() ) );
		if ( !m_msgBoxShown )
		{
			m_msgBoxShown = true;
*/
		KMessageBox::queuedMessageBox( Kopete::UI::Global::mainWidget(), KMessageBox::Information,
			 i18n( "The MSN server is busy.\nPlease retry connecting later." ), i18n( "MSN Plugin" ), KMessageBox::Notify );
		break;
	}

	case 911:
	{
		//QString msg = i18n( "Authentication failed.\n"
		//	"Check your username and password in the MSN Preferences dialog." );

		m_badPassword = true;
		disconnect();
		//KMessageBox::error( Kopete::UI::Global::mainWidget(), msg, i18n( "MSN Plugin" ) );
		break;
	}
	default:
		MSNSocket::handleError( code, id );
	}
}

void MSNAuthSocket::parseCommand( const QString &cmd, uint id, const QString &data )
{
	if ( cmd == "VER" )
	{
		sendCommand( "CVR", "0x0409 winnt 5.1 i386 MSNMSGR 6.0.0602 MSMSGS " + m_msnId );
/*
		struct utsname utsBuf;
		uname ( &utsBuf );

		sendCommand( "CVR", i18n( "MS Local code, see http://www.microsoft.com/globaldev/reference/oslocversion.mspx", "0x0409" ) +
			" " + escape( utsBuf.sysname ) + " " + escape( utsBuf.release ) + " " + escape( utsBuf.machine ) + " Kopete " +
			escape( kapp->aboutData()->version() ) + " Kopete " + m_msnId );
*/
	}
	else if ( cmd == "CVR" ) //else if ( cmd == "INF" )
	{
		sendCommand( "USR", "TWN I " + m_msnId );
	}
	else
	{
		kdDebug( 14140 ) << k_funcinfo << "Unimplemented command '" << cmd << " " << id << " " << data << "' from server!" << endl;
	}
}

void MSNAuthSocket::doneConnect()
{
	kdDebug( 14140 ) << k_funcinfo << "Negotiating server protocol version" << endl;
	sendCommand( "VER", "MSNP9" );
}

#include "msnauthsocket.moc"

// vim: set noet ts=4 sts=4 sw=4:

