/*
    msnsocket.cpp - Base class for the sockets used in MSN

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

#include "msnsocket.h"

#include <qregexp.h>
#include <qtimer.h>

#include <kdebug.h>
#include <kextsock.h>
#include <klocale.h>
#include <kmessagebox.h>

MSNSocket::MSNSocket()
{
	m_onlineStatus = Disconnected;
}

MSNSocket::~MSNSocket()
{
	if( m_onlineStatus != Disconnected )
		disconnect();
}

void MSNSocket::connect( const QString &server, uint port )
{
	if( m_onlineStatus == Connected || m_onlineStatus == Connecting )
	{
		kdDebug() << "MSNSocket::connect: WARNING: Already connected or "
			<< "connecting! Not connecting again." << endl;
		return;
	}

	if( m_onlineStatus == Disconnecting )
	{
		// Cleanup first.
		// FIXME: More generic!!!
		delete m_socket;
	}

	setOnlineStatus( Connecting );
	m_id = 0;
	m_waitBlockSize = 0L;

	m_server = server;
	m_port = port;
	m_socket = new KExtendedSocket( server, port, 0x600000 );
	m_socket->enableRead( true );

	QObject::connect( m_socket, SIGNAL( readyRead() ),
		this, SLOT( slotDataReceived() ) );

	// This is only for async connect!
	//connect( socket, SIGNAL( connectionFailed( int ) ),
	//	this, SLOT( slotSocketError( int ) ) );

	aboutToConnect();
	m_socket->connect();
	doneConnect();
}

void MSNSocket::disconnect()
{
	delete m_socket;
	m_socket = 0L;
	m_buffer = QString::null;

	kdDebug() << "MSNSocket::disconnect: Socket closed" << endl;
	doneDisconnect();
}

void MSNSocket::aboutToConnect()
{
	/* Empty default implementation */
}

void MSNSocket::doneConnect()
{
	setOnlineStatus( Connected );
}

void MSNSocket::doneDisconnect()
{
	setOnlineStatus( Disconnected );
}

void MSNSocket::setOnlineStatus( MSNSocket::OnlineStatus status )
{
	if( m_onlineStatus == status )
		return;

	m_onlineStatus = status;
	emit( onlineStatusChanged( status ) );
}

/*void MSNSocket::slotSocketError(int error)
{
	if(!_silent)
	{
		switch(error)
		{
			case 0:
			{
				KMessageBox::error(0,i18n("Connection refused"));
				break;
			}
			case 1:
			{
				KMessageBox::error(0,i18n("Host not found"));
				break;
			}
			default:
			{
				KMessageBox::error(0,i18n("Socket error"));
			}
		}
	}
	emit connected(false);
	isConnected = false;
	kdDebug() << "Socket error: " << error << endl;
}*/

void MSNSocket::slotDataReceived()
{
	int avail = m_socket->bytesAvailable();
	int toRead = avail;
	if( avail == 0 )
	{
		kdDebug() << "MSNSocket::slotDataReceived:\n"
			"** WARNING ** bytesAvailable() returned 0!\n"
			"If you are running KDE 3.0, please upgrade to current CVS or to\n"
			"KDE 3.0.1 to fix a bug in KExtendedSocket always returning 0.\n"
			"Trying to read 4kb blocks instead, but be prepared for problems!"
			<< endl;
		toRead = 4096;
	}

	// incoming data
	char *buf = new char[ toRead + 1 ];
	int ret = m_socket->readBlock( buf, toRead );
	if( ret < 0 )
	{
		kdDebug() << "MSNSocket:slotDataReceived: WARNING: "
			"readBlock() returned " << ret << "!" <<endl;
	}
	else if( ret == 0 )
	{
		kdDebug() << "MSNSocket:slotDataReceived: WARNING: "
			"readBlock() returned no data!" <<endl;
	}
	else
	{
		if( avail )
		{
			if( ret != avail)
			{
				kdDebug() << "MSNSocket:slotDataReceived: WARNING: "
					<< avail << " bytes were reported available, "
					<< "but readBlock() returned only " << ret
					<< " bytes! Proceeding anyway." << endl;
			}
		}
		else
		{
			kdDebug() << "MSNSocket:slotDataReceived: Info: "
				<< "Read " << ret << " bytes into 4kb block." << endl;
		}

		buf[ ret ] = '\0'; // Make it properly null-terminated
		QString data = QString::fromUtf8( buf );
		kdDebug() << "MSNSocket::slotDataReceived: Received '" <<
			data << "'" << endl;

		m_buffer += data; // fill the buffer with the received data
		slotReadLine();
	}

	// Cleanup
	delete[] buf;
}

void MSNSocket::slotReadLine()
{
	// If there's no CR/LF waiting, just stop and wait for new data to arrive
	if( !m_buffer.contains( "\r\n" ) )
		return;

	// We have data, first check if it's meant for a block read, otherwise
	// parse the first line (which will recursively parse the other lines)
	if( !pollReadBlock() )
	{
		int index = m_buffer.find( "\r\n" );
		if( index != -1 )
		{
			QString command = m_buffer.left( index );
			m_buffer = m_buffer.remove( 0, index + 2 );
			command.replace( QRegExp( "\r\n" ), "" );
			kdDebug() << "MSNSocket::slotReadLine: " << command << endl;

			parseLine( command );

			// Don't block the GUI while parsing data, only do a single line!
			QTimer::singleShot( 0, this, SLOT( slotReadLine() ) );
		}
	}
}

void MSNSocket::readBlock( uint len )
{
	if( m_waitBlockSize )
	{
		kdDebug() << "MSNSocket::readBlock: WARNING: cannot wait for data "
			<< "block: still waiting for other block of size "
			<< m_waitBlockSize << "! Data will not be returned." << endl;
		return;
	}

	m_waitBlockSize = len;

	// Try to return the data now, if available. Otherwise slotDataReady
	// will do this whenever all data is there.
	pollReadBlock();
}

bool MSNSocket::pollReadBlock()
{
	if( !m_waitBlockSize )
		return false;
	else if( m_buffer.length() < m_waitBlockSize )
		return true;

	QString block;
	block = m_buffer.left( m_waitBlockSize );
	m_buffer = m_buffer.remove( 0, m_waitBlockSize );
	m_waitBlockSize = 0;
	emit blockRead( block );

	return false;
}

void MSNSocket::parseLine( const QString &str )
{
	QString cmd  = str.section( ' ', 0, 0 );
	QString data = str.section( ' ', 2 ).replace( QRegExp( "\r\n" ), "" );

	bool isNum;
	uint id = str.section( ' ', 1, 1 ).toUInt();

	// In some rare cases, like the 'NLN' / 'FLN' commands no id at all
	// is sent. Here it's actually a real parameter...
	if( !isNum )
		data = str.section( ' ', 1, 1 ) + " " + data;


	kdDebug() << "MSNSocket::parseCommand: Parsing command " << cmd <<
		" (ID " << id << "): '" << data << "'" << endl;

	data.replace( QRegExp( "\r\n" ), "" );
	bool isError;
	uint errorCode = cmd.toUInt( &isError );
	if( isError )
		handleError( errorCode, id );
	else
		parseCommand( cmd, id, data );
}

void MSNSocket::handleError( uint code, uint id )
{
	QString msg =
		i18n( "Unhandled MSN error code %1 (in response to "
			"transaction ID %2).\n"
			"Please mail kopete-devel@kde.org to ask for an implementation "
			"or send in a patch yourself ;-).\n"
			"See http://www.hypothetic.org/docs/msn/basics.php for a "
			"description of all error codes." ).arg( code ).arg( id );
	disconnect();
	KMessageBox::error( 0, msg, i18n( "MSN Plugin - Kopete" ) );
}

void MSNSocket::sendCommand( const QString &cmd, const QString &args,
	bool addNewLine )
{
	QString data = cmd + " " + QString::number( m_id );
	if( !args.isEmpty() )
		data += " " + args;
	if( addNewLine )
		data += "\r\n";

	kdDebug() << "MSNSocket::sendCommand: Sending command " << data;

	m_socket->writeBlock( data, data.length() );

	m_id++;
}

#include "msnsocket.moc"

// vim: set noet ts=4 sts=4 sw=4:

