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
#include <kurl.h>

MSNSocket::MSNSocket()
{
	m_onlineStatus = Disconnected;
	m_socket = 0L;
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
	m_lastId = 0;
	m_waitBlockSize = 0L;

	m_lookupStatus = Processing;

	m_sendQueue.clear();

	m_server = server;
	m_port = port;
	m_socket = new KExtendedSocket( server, port, 0x600000 );
	m_socket->enableRead( true );

	QObject::connect( m_socket, SIGNAL( readyRead() ),
		this, SLOT( slotDataReceived() ) );
	QObject::connect( m_socket, SIGNAL( connectionSuccess() ),
		this, SLOT( slotConnectionSuccess() ) );

	QObject::connect( m_socket, SIGNAL( connectionFailed( int ) ),
		this, SLOT( slotSocketError( int ) ) );

	QObject::connect( m_socket, SIGNAL( lookupFinished ( int ) ),
		this, SLOT( slotLookupFinished( int ) ) );

	QObject::connect( m_socket, SIGNAL( closed ( int ) ),
		this, SLOT( slotSocketClosed( int ) ) );

	aboutToConnect();
	m_socket->startAsyncConnect();
}

void MSNSocket::disconnect()
{
	delete m_socket;
	m_socket = 0L;
	m_buffer = "";

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

void MSNSocket::slotSocketError( int error )
{
	m_socket->cancelAsyncConnect();
	kdDebug() << "MSNSocket::slotSocketError: error: " << error << endl;

	QString errormsg = i18n( "There was an error while connecting to the MSN server.\nError message:\n" );

	if ( m_lookupStatus == Failed )
		errormsg += i18n( "Unable to lookup %1" ).arg( m_socket->host() );
	else
		errormsg += KExtendedSocket::strError( error, m_socket->systemError() );

	KMessageBox::error( 0, errormsg, i18n( "MSN Plugin - Kopete" ) );

	// Emit the signal even if we still were disconnected.
	if ( onlineStatus() == Disconnected )
		emit( onlineStatusChanged( Disconnected ) );

	setOnlineStatus( Disconnected );

	emit connectionFailed();
}

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
	else if ( avail == -1 )
	{
		// error!
		kdDebug() << "MSNSocket::slotDataReceived: bytesAvailable() returned"
			" -1. Are we disconnected?" << endl;
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
		kdDebug() << "MSNSocket::slotDataReceived: Received '" <<
			buf << "'" << endl;

		m_buffer += buf; // fill the buffer with the received data
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
			QString command = QString::fromUtf8(m_buffer.left(index));
			m_buffer = m_buffer.remove( 0, index + 2 );
			command.replace( QRegExp( "\r\n" ), "" );
			kdDebug() << "MSNSocket::slotReadLine: " << command << endl;

			parseLine(command);

			// See if we have pending changes in the queue...
			if( !m_sendQueue.isEmpty() )
			{
				kdDebug() << "MSNSocket::slotReadLine: Send queue not "
					<< "empty, attempting to flush first item. m_lastId: "
					<< m_lastId << endl;
				QMap<uint, QCString>::Iterator it = m_sendQueue.begin();
				if( m_lastId >= it.key() - 1 )
				{
					kdDebug() << "MSNSocket::slotReadLine: "
						<< "Flushing entry from send queue: "
						<< it.data() << endl;
					m_socket->writeBlock( it.data(), it.data().length() );
					m_sendQueue.remove( it );
				}
			}

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

	kdDebug() << "MSNSocket::readBlock: Preparing for block read of size "
		<< len << endl;

	// Try to return the data now, if available. Otherwise slotDataReady
	// will do this whenever all data is there.
	pollReadBlock();
}

bool MSNSocket::pollReadBlock()
{
	if( !m_waitBlockSize )
		return false;
	else if( m_buffer.length() < m_waitBlockSize )
	{
		kdDebug() << "MSNSocket::pollReadBlock: Waiting for data. Received: "
			<< m_buffer.length() << ", required: " << m_waitBlockSize << endl;
		return true;
	}

	QString block;
	block = QString::fromUtf8(m_buffer.left( m_waitBlockSize ));
	m_buffer = m_buffer.remove( 0, m_waitBlockSize );

	kdDebug() << "MSNSocket::pollReadBlock: Successfully read block of size "
			<< m_waitBlockSize << endl;

	m_waitBlockSize = 0;
	emit blockRead( block );

	return false;
}

void MSNSocket::parseLine( const QString &str )
{
	QString cmd  = str.section( ' ', 0, 0 );
	QString data = str.section( ' ', 2 ).replace( QRegExp( "\r\n" ), "" );

	bool isNum;
	uint id = str.section( ' ', 1, 1 ).toUInt( &isNum );

	// In some rare cases, like the 'NLN' / 'FLN' commands no id at all
	// is sent. Here it's actually a real parameter...
	if( !isNum )
		data = str.section( ' ', 1, 1 ) + " " + data;

	if( isNum && id )
		m_lastId = id;

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
	bool addId, const QString &body )
{
	QCString data = cmd.utf8();
	if( addId )
		data += " " + QString::number( m_id ).utf8();

	if( !args.isEmpty() )
		data += " " + args.utf8();

	// Add length in bytes, not characters
	if( !body.isEmpty() )
		data += " " + QString::number( body.utf8().length() ).utf8();

	data += "\r\n";

	if( !body.isEmpty() )
		data += body.utf8();

	kdDebug() << "MSNSocket::sendCommand: Sending command " << data << endl;


	// If the last confirmed Id is the last we sent, sent directly.
	// Otherwise, queue. Command without Id are always sent.
	// In case of queuing it is reasonable to assume the server will send
	// a response, so the queue handling can be done in the read handler.
	if( !m_lastId || !addId || ( m_lastId && m_lastId >= m_id - 1 ) )
		m_socket->writeBlock( data, data.length() );
	else
	{
		kdDebug() << "MSNSocket::sendCommand: Not sending directly. Entry "
			<< "added to send queue with id " << m_id << endl;
		m_sendQueue.insert( m_id, data );
	}

	m_id++;
}

QString MSNSocket::escape( const QString &str )
{
	return ( KURL::encode_string(str) );
}

QString MSNSocket::unescape( const QString &str )
{
	return ( KURL::decode_string(str) );
}

void MSNSocket::slotConnectionSuccess()
{
	kdDebug() << "MSNSocket::slotConnectionSuccess" << endl;
	doneConnect();
}

void MSNSocket::slotLookupFinished( int count )
{
	if ( count == 0 )
		m_lookupStatus = Failed;
	else
		m_lookupStatus = Success;
}

void MSNSocket::slotSocketClosed( int state )
{
	kdDebug() << "MSNSocket::slotSocketClosed: socket closed. State: 0x" <<
		QString::number( state, 16 ) << endl;

	setOnlineStatus( Disconnected );
	
	emit( socketClosed( state ) );
}
#include "msnsocket.moc"

// vim: set noet ts=4 sts=4 sw=4:

