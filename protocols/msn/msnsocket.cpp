/*
    msnsocket.cpp - Base class for the sockets used in MSN

    Copyright (c) 2002-2003 by Martijn Klingens       <klingens@kde.org>
    Copyright (c) 2002-2004 by Olivier Goffart        <ogoffart@tiscalinet.be>

    Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

    Portions of this code are taken from KMerlin,
              (c) 2001      by Olaf Lueg              <olueg@olsd.de>

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
//#include "msnprotocol.h"

#include <qregexp.h>
#include <qtimer.h>

#include <kdebug.h>
#include <kbufferedsocket.h>
#include <kserversocket.h>
#include <kresolver.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kurl.h>

#include "kopeteuiglobal.h"

using namespace KNetwork;

MSNSocket::MSNSocket(QObject* parent)  : QObject (parent)
{
	m_onlineStatus = Disconnected;
	m_socket = 0L;
}

MSNSocket::~MSNSocket()
{
	//if ( m_onlineStatus != Disconnected )
	//	disconnect();
	doneDisconnect();
	if ( m_socket )
		m_socket->deleteLater();
}

void MSNSocket::connect( const QString &server, uint port )
{
	if ( m_onlineStatus == Connected || m_onlineStatus == Connecting )
	{
		kdWarning( 14140 ) << k_funcinfo << "Already connected or connecting! Not connecting again." << endl;
		return;
	}

	if( m_onlineStatus == Disconnecting )
	{
		// Cleanup first.
		// FIXME: More generic!!!
		kdWarning( 14140 ) << k_funcinfo << "We're still disconnecting! Deleting socket the hard way first." << endl;
		delete m_socket;
	}

	setOnlineStatus( Connecting );
	m_id = 0;
	//m_lastId = 0;
	m_waitBlockSize = 0;
	m_buffer = Buffer( 0 );

	//m_sendQueue.clear();

	m_server = server;
	m_port = port;
	m_socket = new KBufferedSocket( server, QString::number(port) );
	 //can this prevent the kopete frezee? (http://lists.kde.org/?l=kopete-devel&m=107117795131722&w=2)

	m_socket->enableRead( true );

	// enableWrite eats the CPU, and we only need it when the queue is
	// non-empty, so disable it until we have actual data in the queue
	m_socket->enableWrite( false );

	QObject::connect( m_socket, SIGNAL( readyRead() ),             this, SLOT( slotDataReceived() ) );
	QObject::connect( m_socket, SIGNAL( readyWrite() ),            this, SLOT( slotReadyWrite() ) );
	QObject::connect( m_socket, SIGNAL( hostFound() ),	       this, SLOT( slotHostFound() ) );
	QObject::connect( m_socket, SIGNAL( connected( const KResolverEntry&) ), this, SLOT( slotConnectionSuccess() ) );
	QObject::connect( m_socket, SIGNAL( gotError( int ) ),         this, SLOT( slotSocketError( int ) ) );
	QObject::connect( m_socket, SIGNAL( closed( ) ),               this, SLOT( slotSocketClosed( ) ) );

	aboutToConnect();

	// start the asynchronous connection
	m_socket->connect();
}

void MSNSocket::disconnect()
{
	if ( m_socket )
		m_socket->closeNow();
	else
		slotSocketClosed();
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
	if ( m_onlineStatus == status )
		return;

	m_onlineStatus = status;
	emit onlineStatusChanged( status );
}

void MSNSocket::slotSocketError( int error )
{
	kdDebug( 14140 ) << k_funcinfo << "Error: " << error << endl;

	QString errormsg = i18n( "There was an error while connecting to the MSN server.\nError message:\n" );
	if ( error == KSocketBase::LookupFailure )
		errormsg += i18n( "Unable to lookup %1" ).arg( m_socket->peerResolver().nodeName() );
	else
	        // FIXME when there is a method to show up error strings!
          	errormsg += i18n( "Error %1" ).arg( m_socket->error() );

	//delete m_socket;
	m_socket->deleteLater();
	m_socket = 0L;

	setOnlineStatus( Disconnected );
	emit connectionFailed();
	//like if the socket is closed
	emit socketClosed();

	KMessageBox::queuedMessageBox( Kopete::UI::Global::mainWidget(), KMessageBox::Error, errormsg, i18n( "MSN Plugin" ) );
}

void MSNSocket::slotDataReceived()
{
	int avail = m_socket->bytesAvailable();
	if ( avail < 0 )
	{
		// error!
		kdWarning( 14140 ) << k_funcinfo << "bytesAvailable() returned " << avail
		 	<< ". This should not happen!" << endl
			<< "Are we disconnected? Backtrace:" << endl << kdBacktrace() << endl;
		return;
	}

	// incoming data
	char *buf = new char[ avail ];
	int ret = m_socket->readBlock( buf, avail );
	if ( ret < 0 )
	{
		kdWarning( 14140 ) << k_funcinfo << "readBlock() returned " << ret << "!" <<endl;
	}
	else if ( ret == 0 )
	{
		kdWarning( 14140 ) << k_funcinfo << "readBlock() returned no data!" <<endl;
	}
	else
	{
		if ( avail )
		{
			if ( ret != avail)
			{
				kdWarning( 14140 ) << k_funcinfo << avail << " bytes were reported available, "
					<< "but readBlock() returned only " << ret << " bytes! Proceeding anyway." << endl;
			}
		}
		else
		{
			kdDebug( 14140 ) << k_funcinfo << "Read " << ret << " bytes into 4kb block." << endl;
		}

		// Simple check to avoid dumping the binary data from the icons and emoticons to kdDebug:
		// all MSN commands start with one or more uppercase characters.
		// For now just check the first three chars, let's see how accurate it is.
		// Additionally, if we receive an MSN-P2P packet, strip off anything after the P2P header.
		QString rawData = QString( QCString( buf, avail ) ).stripWhiteSpace().replace(
			QRegExp( "(P2P-Dest:.[a-zA-Z@.]*).*" ), "\\1\n\n(Stripped binary data)" );

		bool isBinary = false;
		for ( uint i = 0; i < 3 ; ++i )
		{
			if ( (rawData[ i ] < 'A' || rawData[ i ] > 'Z') && (rawData[ i ] < '0' || rawData[ i ] > '9')  )
				isBinary = true;
		}

		if ( isBinary )
			kdDebug( 14141 ) << k_funcinfo << "(Stripped binary data)" << endl;
		else
			kdDebug( 14141 ) << k_funcinfo << rawData << endl;

		// fill the buffer with the received data
		m_buffer.add( buf, ret );

		slotReadLine();
	}
	// Cleanup
	delete[] buf;
}

void MSNSocket::slotReadLine()
{
	// We have data, first check if it's meant for a block read, otherwise
	// parse the first line (which will recursively parse the other lines)
	if ( !pollReadBlock() )
	{
		if ( m_buffer.size() >= 3 && ( m_buffer.data()[ 0 ] == '\0' || m_buffer.data()[ 0 ]== '\1' ) )
		{
			bytesReceived( m_buffer.take( 3 ) );
			QTimer::singleShot( 0, this, SLOT( slotReadLine() ) );
			return;
		}

		int index = -1;
		for ( uint x = 0; m_buffer.size() > x + 1; ++x )
		{
			if ( ( m_buffer[ x ] == '\r' ) && ( m_buffer[ x + 1 ] == '\n' ) )
			{
				index = x;
				break;
			}
		}

		if ( index != -1 )
		{
			QString command = QString::fromUtf8( m_buffer.take( index + 2 ), index );
			command.replace( "\r\n", "" );
			//kdDebug( 14141 ) << k_funcinfo << command << endl;

			// Don't block the GUI while parsing data, only do a single line!
			// (Done before parseLine() to prevent a potential crash)
			QTimer::singleShot( 0, this, SLOT( slotReadLine() ) );

			parseLine( command );
			// WARNING: At this point 'this' can be deleted (when disconnecting)
		}
	}
}

void MSNSocket::readBlock( uint len )
{
	if ( m_waitBlockSize )
	{
		kdWarning( 14140 ) << k_funcinfo << "Cannot wait for data block: still waiting for other block of size "
			<< m_waitBlockSize << "! Data will not be returned." << endl;
		return;
	}

	m_waitBlockSize = len;

	//kdDebug( 14140 ) << k_funcinfo << "Preparing for block read of size " << len << endl;

	// Try to return the data now, if available. Otherwise slotDataReady
	// will do this whenever all data is there.
	pollReadBlock();
}

bool MSNSocket::pollReadBlock()
{
	if ( !m_waitBlockSize )
	{
		return false;
	}
	else if ( m_buffer.size() < m_waitBlockSize )
	{
		kdDebug( 14140 ) << k_funcinfo << "Waiting for data. Received: " << m_buffer.size() << ", required: " << m_waitBlockSize << endl;
		return true;
	}

	QByteArray baBlock = m_buffer.take( m_waitBlockSize );
	QString block = QString::fromUtf8( baBlock, m_waitBlockSize );

	//kdDebug( 14140 ) << k_funcinfo << "Successfully read block of size " << m_waitBlockSize << endl;

	m_waitBlockSize = 0;
	emit blockRead( block );
	emit blockRead( baBlock );

	return false;
}

void MSNSocket::parseLine( const QString &str )
{
	QString cmd  = str.section( ' ', 0, 0 );
	QString data = str.section( ' ', 2 ).replace( "\r\n" , "" );

	bool isNum;
	uint id = str.section( ' ', 1, 1 ).toUInt( &isNum );

	// In some rare cases, like the 'NLN' / 'FLN' commands no id at all
	// is sent. Here it's actually a real parameter...
	if ( !isNum )
		data = str.section( ' ', 1, 1 ) + " " + data;

	//if ( isNum && id )
	//	m_lastId = id;

	//kdDebug( 14140 ) << k_funcinfo << "Parsing command " << cmd << " (ID " << id << "): '" << data << "'" << endl;

	data.replace( "\r\n", "" );
	bool isError;
	uint errorCode = cmd.toUInt( &isError );
	if ( isError )
		handleError( errorCode, id );
	else
		parseCommand( cmd, id, data );
}

void MSNSocket::handleError( uint code, uint /* id */ )
{
	QString msg;

	switch ( code )
	{
/*
		// We cant show message for error we don't know what they are or not related to the correct socket
		//  Theses following messages are not so instructive
	case 205:
		msg = i18n ( "An invalid username has been specified.\nPlease correct it, and try to reconnect.\n" );
		break;
	case 201:
		msg = i18n ( "Fully Qualified domain name missing.\n" );
		break;
	case 207:
		msg = i18n ( "You are already logged in!\n" );
		break;
	case 208:
		msg = i18n ( "You specified an invalid username.\nPlease correct it, and try to reconnect.\n");
		break;
	case 209:
		msg = i18n ( "Your nickname is invalid. Please check it, correct it,\nand try to reconnect.\n" );
		break;
	case 210:
		msg = i18n ( "Your list has reached its maximum capacity.\nNo more contacts can be added, unless you remove some first.\n" );
		break;
	case 216:
		 msg = i18n ( "This user is not in your contact list.\n " );
		break;
	case 300:
		msg = i18n ( "Some required fields are missing. Please fill them in and try again.\n" );
		break;
	case 302:
		msg = i18n ( "You are not logged in.\n" );
		break;
*/
	case 500:
		disconnect();
		msg = i18n ( "An internal server error occurred. Please try again later." );
		break;
	case 502:
		msg = i18n ( "It is no longer possible to perform this operation. The MSN server does not allow it anymore." );
		break;
	case 600:
	case 910:
	case 912:
	case 922:
		disconnect();
		msg = i18n ( "The MSN server is busy. Please try again later." );
		break;
	case 601:
	case 604:
	case 605:
	case 914:
	case 915:
	case 916:
	case 917:
		disconnect();
		msg = i18n ( "The server is not available at the moment. Please try again later." );
		break;
	default:
		// FIXME: if the error causes a disconnect, it will crash, but we can't disconnect every time
		msg = i18n( "Unhandled MSN error code %1 \n"
			"Please fill a bug report with a detailed description and if possible the last console debug output." ).arg( code );
			// "See http://www.hypothetic.org/docs/msn/basics.php for a description of all error codes."
		break;
	}

	if ( !msg.isEmpty() )
		KMessageBox::queuedMessageBox( Kopete::UI::Global::mainWidget(), KMessageBox::Error, msg, i18n( "MSN Plugin" ) );

	return;
}

int MSNSocket::sendCommand( const QString &cmd, const QString &args, bool addId, const QByteArray &body, bool binary )
{
	if ( !m_socket )
	{
		kdWarning( 14140 ) << k_funcinfo << "m_socket == NULL!" << endl;
		return -1;
	}

	QCString data = cmd.utf8();
	if ( addId )
		data += " " + QString::number( m_id ).utf8();

	if ( !args.isEmpty() )
		data += " " + args.utf8();

	// Add length in bytes, not characters
	if ( !body.isEmpty() )
		data += " " + QString::number( body.size() - (binary ? 0 : 1 ) ).utf8();

	data += "\r\n";

	if ( !binary )
	{
		if( !body.isEmpty() )
			data += QCString( body, body.size() + 1 );

		//kdDebug( 14141 ) << k_funcinfo << "Sending command " << data << endl;

		// the command will be sent in slotReadyWrite
		m_sendQueue.append( data );
		m_socket->enableWrite( true );
	}
	else
	{
		QByteArray data2( data.length() + body.size() );
		// FIXME: Why not use QByteArray::copy() for the first loop? - Martijn
		for ( uint f = 0; f < data.length(); f++ )
			data2[ f ] = data[ f ];
		for ( uint f = 0; f < body.size(); f++ )
			data2[ data.length() + f ] = body[ f ];

		sendBytes( data2 ) ;

		//kdDebug( 14141 ) << k_funcinfo << data2.data() << endl;
	}

	if ( addId )
	{
		++m_id;
		return m_id - 1;
	}
	return 0;
}

void MSNSocket::slotReadyWrite()
{
	if ( !m_sendQueue.isEmpty() )
	{
		QValueList<QCString>::Iterator it = m_sendQueue.begin();
		kdDebug( 14141 ) << k_funcinfo << "Sending command: " << QString( *it ).stripWhiteSpace() << endl;
		m_socket->writeBlock( *it, ( *it ).length() );
		m_sendQueue.remove( it );
		emit commandSent();

		// If the queue is empty again stop waiting for readyWrite signals
		// because of the CPU usage
		if ( m_sendQueue.isEmpty() )
			m_socket->enableWrite( false );
	}
	else
	{
		m_socket->enableWrite( false );
	}
}

QString MSNSocket::escape( const QString &str )
{
	//return ( KURL::encode_string( str, 106 ) );
	//It's not needed to encode everything. The official msn client only encode spaces and %
	//If we encode more, the size can be longer than excepted.

	int old_length= str.length();
	QChar *new_segment = new QChar[ old_length * 3 + 1 ];
	int new_length = 0;

	for	( int i = 0; i < old_length; i++ )
	{
		unsigned char character = str[i];
		
		 /*character == ' '  || character == '%' || character == '\t' 
			|| characters == '\n' || character == '\r'*/
		/*	|| character == '<' || character == '>' ||  character == '\\'
			|| character == '^' || character == '&' || character == '*'*/ 
		
		if( character <= 32 || character == '%' )
		{
			new_segment[ new_length++ ] = '%';

			unsigned int c = character / 16;
			c += (c > 9) ? ('A' - 10) : '0';
			new_segment[ new_length++ ] = c;

			c = character % 16;
			c += (c > 9) ? ('A' - 10) : '0';
			new_segment[ new_length++ ] = c;
		}
		else
			new_segment[ new_length++ ] = str[i];
	}

	QString result = QString(new_segment, new_length);
	delete [] new_segment;
	return result;
}

QString MSNSocket::unescape( const QString &str )
{
	//GRRRRR F*CKING MSN PLUS USERS! They insert these stupid color codes in their nickname, and messages are not correctly shown
	return KURL::decode_string( str, 106 ).replace( "\3", "" ).replace( "\4", "" ).replace( "\2", "" );
}

void MSNSocket::slotConnectionSuccess()
{
	//kdDebug( 14140 ) << k_funcinfo << endl;
	doneConnect();
}

void MSNSocket::slotHostFound()
{
        // nothing to do
}

void MSNSocket::slotSocketClosed()
{
        kdDebug( 14140 ) << k_funcinfo << "Socket closed. " << endl;

	if ( !m_socket ||  m_onlineStatus == Disconnected )
	{
		kdDebug( 14140 ) << k_funcinfo << "Socket already deleted or already disconnected" << endl;
		return;
	}

	doneDisconnect();

	m_buffer = Buffer( 0 );
	//delete m_socket;
	m_socket->deleteLater();
	m_socket = 0L;

	emit socketClosed();
}

// Used in MSNFileTransferSocket
// FIXME: Why is this here if it's only used for file transfer? - Martijn
void MSNSocket::bytesReceived( const QByteArray & /* data */ )
{
	kdWarning( 14140 ) << k_funcinfo << "Unknown bytes were received" << endl;
}

void MSNSocket::sendBytes( const QByteArray &data )
{
	if ( !m_socket )
	{
		kdWarning( 14140 ) << k_funcinfo << "Not yet connected" << endl;
		return;
	}
	m_socket->writeBlock( data, data.size() );
	m_socket->enableWrite( true );
}

bool MSNSocket::accept( KServerSocket *server )
{
	if ( m_socket )
	{
		kdWarning( 14140 ) << k_funcinfo << "Socket already exists!" << endl;
		return false;
	}

	m_socket = static_cast<KBufferedSocket*>(server->accept());
	kdDebug( 14140 ) << k_funcinfo << "Result: " << (m_socket != 0L) << ", m_socket: " << m_socket << endl;

	if ( !m_socket )
		return false;

	setOnlineStatus( Connecting );

	m_id = 0;
	//m_lastId = 0;
	m_waitBlockSize = 0;

	m_socket->setBlocking( false );
	m_socket->enableRead( true );
	m_socket->enableWrite( true );

	QObject::connect( m_socket, SIGNAL( readyRead() ),             this, SLOT( slotDataReceived() ) );
	QObject::connect( m_socket, SIGNAL( readyWrite() ),            this, SLOT( slotReadyWrite() ) );
	QObject::connect( m_socket, SIGNAL( closed() ),                this, SLOT( slotSocketClosed() ) );
	QObject::connect( m_socket, SIGNAL( gotError( int ) ),         this, SLOT( slotSocketError( int ) ) );

	doneConnect();
	return true;
}

QString MSNSocket::getLocalIP()
{
	if ( !m_socket )
		return QString::null;

	const KSocketAddress address = m_socket->localAddress();

	QString ip = address.nodeName();

	kdDebug( 14140 ) << k_funcinfo << "IP: " << ip  <<endl;
	//delete address;
	return ip;
}

MSNSocket::Buffer::Buffer( unsigned int sz )
: QByteArray( sz )
{
}

MSNSocket::Buffer::~Buffer()
{
}

void MSNSocket::Buffer::add( char *str, unsigned int sz )
{
	char *b = new char[ size() + sz ];
	for ( uint f = 0; f < size(); f++ )
		b[ f ] = data()[ f ];
	for ( uint f = 0; f < sz; f++ )
		b[ size() + f ] = str[ f ];

	duplicate( b, size() + sz );
	delete[] b;
}

QByteArray MSNSocket::Buffer::take( unsigned blockSize )
{
	if ( size() < blockSize )
	{
		kdWarning( 14140 ) << k_funcinfo << "Buffer size " << size() << " < asked size " << blockSize << "!" << endl;
		return QByteArray();
	}

	QByteArray rep( blockSize );
	for( uint i = 0; i < blockSize; i++ )
		rep[ i ] = data()[ i ];

	char *str = new char[ size() - blockSize ];
	for ( uint i = 0; i < size() - blockSize; i++ )
		str[ i ] = data()[ blockSize + i ];
	duplicate( str, size() - blockSize );
	delete[] str;

	return rep;
}

#include "msnsocket.moc"

// vim: set noet ts=4 sts=4 sw=4:

