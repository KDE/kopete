/*
    msnsocket.cpp - Base class for the sockets used in MSN

    Copyright (c) 2002-2003 by Martijn Klingens       <klingens@kde.org>
    Copyright (c) 2002-2005 by Olivier Goffart        <ogoffart at kde.org>
    Copyright (c) 2005		by Gregg Edghill 		  <gregg.edghill@gmail.com>

    Kopete    (c) 2002-2005 by the Kopete developers  <kopete-devel@kde.org>

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
#include <kconfig.h>
#include <kbufferedsocket.h>
#include <kserversocket.h>
#include <kresolver.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kurl.h>

#include "kopeteuiglobal.h"

using namespace KNetwork;

class MimeMessage
{
	public:
		MimeMessage(const QString &msg) : message(msg) {}

		QString getValue(const QString &key)
		{
			QRegExp rx(key+": ([^\r\n]+)");
			rx.search(message);
			return rx.cap(1);
		}
	private:
		QString message;
};

MSNSocket::MSNSocket(QObject* parent)  : QObject (parent)
{
	m_onlineStatus = Disconnected;
	m_socket = 0L;
	m_useHttp = false;
	m_timer  = 0L;
}

MSNSocket::~MSNSocket()
{
	//if ( m_onlineStatus != Disconnected )
	//	disconnect();
	delete m_timer;
	m_timer = 0L;
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

	if(!m_useHttp)
		m_socket = new KBufferedSocket( server, QString::number(port) );
	else {
		m_socket = new KBufferedSocket( m_gateway, "80" );
	}

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

	if(m_useHttp)
	{
		if(m_timer == 0L)
		{
			m_timer = new QTimer(this, "Http poll timer");
			// Connect the slot HttpPoll with the timer timeout signal.
			QObject::connect(m_timer, SIGNAL(timeout()), this, SLOT(slotHttpPoll()));
		}
	}

	aboutToConnect();

	// start the asynchronous connection
	m_socket->connect();
}

void MSNSocket::disconnect()
{
	if(m_useHttp)
		if(m_timer->isActive()) {
			// If the timer is still active, stop the timer.
			m_timer->stop();
		}

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
	kdWarning( 14140 ) << k_funcinfo << "Error: " << error << " (" << m_socket->errorString() << ")" << endl;

	if(!KSocketBase::isFatalError(error))
		return;
		//we only care about fatal error

	QString errormsg = i18n( "There was an error while connecting to the MSN server.\nError message:\n" );
	if ( error == KSocketBase::LookupFailure )
		errormsg += i18n( "Unable to lookup %1" ).arg( m_socket->peerResolver().nodeName() );
	else
		errormsg +=  m_socket->errorString() ;

	//delete m_socket;
	m_socket->deleteLater();
	m_socket = 0L;

	setOnlineStatus( Disconnected );
	emit connectionFailed();
	//like if the socket is closed
	emit socketClosed();

	emit errorMessage( ErrorConnectionError, errormsg );
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

	// incoming data, plus an extra char where we pretend a NUL is so the conversion
	// to QCString doesn't go over the end of the allocated memory.
	char *buffer = new char[ avail + 1 ];
	int ret = m_socket->readBlock( buffer, avail );

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


		QString rawData;

		if(m_useHttp)
		{
			bool error = false;
			QByteArray bytes;

			// Check if all data has arrived.
			rawData = QString(QCString(buffer, avail + 1));
			bool headers = (rawData.find(QRegExp("HTTP/\\d\\.\\d (\\d+) ([^\r\n]+)")) != -1);

			if(headers)
			{
				// The http header packet arrived.
				int endOfHeaders = rawData.find("\r\n\r\n");
				if((endOfHeaders + 4) == avail)
				{
					// Only the response headers data is included.
					QRegExp re("Content-Length: ([^\r\n]+)");
					if(re.search(rawData) != -1)
					{
						bool valid;
						int l = re.cap(1).toInt(&valid);
						if(valid && l > 0)
						{
							// The packet contains the headers but does not contain the content data;
							// buffer the data received and read again.
							m_buffer.add(buffer, avail);

							delete[] buffer;
							// Update how much data remains.
							m_remaining = l;
							return;
						}
					}
				}
			}
			else
			{
				// Write the received data to the buffer.
				m_buffer.add(buffer, avail);

				m_remaining -= avail;
				if(m_remaining != 0)
				{
					// We have not received all the content data, read again.
					delete[] buffer;
					return;
				}

				// At this point, we have all the bytes returned from the web request.
				bytes = m_buffer.take(m_buffer.size());
			}
			
			if(bytes.size() == 0) 
			{
				// The response headers and the content came in one packet.
				bytes.assign(buffer, avail);
			}


			// Create the web response object from the response bytes.
			WebResponse response(bytes);

			if(response.getStatusCode() == 100) {
				return;
			}

			if(response.getStatusCode() == 200)
			{
				// If we received a valid response, read the required headers.
				// Retrieve the X-MSN-Messenger header.
				QString header = response.getHeaders()->getValue("X-MSN-Messenger");

				QStringList parts = QStringList::split(";", header.replace(" ", ""));
				if(!header.isNull() && (parts.count() >= 2))
				{
					if(parts[0].find("SessionID", 0) != -1)
					{
						// Assign the session id.
						m_sessionId = parts[0].section("=", 1, 1);
					}else
						error = true;

					if(parts[1].find("GW-IP", 0) != -1)
					{
						// Assign the gateway IP address.
						m_gwip = parts[1].section("=", 1, 1);
					}else
						error = true;


					if(parts.count() > 2)
						if((parts[2].find("Session", 0) != -1) && (parts[2].section("=", 1, 1) == "close"))
						{
							// The http session has been closed by the server, disconnect.
							kdDebug(14140) << k_funcinfo << "Session closed." << endl;
							m_bCanPoll = false;
							disconnect();
							return;
						}
				}else
					error = true;

				// Retrieve the content length header.
				header = response.getHeaders()->getValue("Content-Length");

				if(!header.isNull())
				{
					bool valid;
					int length = header.toInt(&valid);
					if(valid && (length == 0))
					{
						// If the response content length is zero, there is nothing to do.
						m_pending  = false;
						return;
					}

					if(valid && (length > 0))
					{
						// Otherwise, if the content length is greater than zero, get the web response stream.
						QDataStream *stream = response.getResponseStream();
						buffer = new char[length];
						// Read the web response content.
						stream->readRawBytes(buffer, length);
						ret = length;
					}else
						error = true;
				}else
					error = true;
			}else
				error = true;

			if(error)
			{
				kdDebug(14140) << k_funcinfo << "Http error: " << response.getStatusCode() << " "
					<< response.getStatusDescription() << endl;

				// If we encountered an error, disconnect and return.
				m_bCanPoll = false;
				// Disconnect from the service.
				disconnect();
				return;
			}
		}

		// Simple check to avoid dumping the binary data from the icons and emoticons to kdDebug:
		// all MSN commands start with one or more uppercase characters.
		// For now just check the first three chars, let's see how accurate it is.
		// Additionally, if we receive an MSN-P2P packet, strip off anything after the P2P header.
		rawData = QString( QCString( buffer, ((!m_useHttp)? avail : ret) + 1 ) ).stripWhiteSpace().replace(
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
		m_buffer.add( buffer, ret );

		slotReadLine();

		if(m_useHttp) {
			// Set data pending to false.
			m_pending  = false;
		}
	}

	// Cleanup.
	delete[] buffer;
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

	QByteArray block = m_buffer.take( m_waitBlockSize );

	//kdDebug( 14140 ) << k_funcinfo << "Successfully read block of size " << m_waitBlockSize << endl;

	m_waitBlockSize = 0;
	emit blockRead( block);

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
	kdDebug(14140) << k_funcinfo << endl;
	QString msg;
	ErrorType type = ErrorServerError;
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
		msg = i18n ( "An internal server error occurred. Please try again later." );
		type = MSNSocket::ErrorCannotConnect;
		break;
	case 502:
		msg = i18n ( "It is no longer possible to perform this operation. The MSN server does not allow it anymore." );
		type = MSNSocket::ErrorServerError;
		break;
	case 600:
	case 910:
	case 912:
	case 921:
	case 922:
		msg = i18n ( "The MSN server is busy. Please try again later." );
		type = MSNSocket::ErrorConnectionError;
		break;
	case 601:
	case 604:
	case 605:
	case 914:
	case 915:
	case 916:
	case 917:
		msg = i18n ( "The server is not available at the moment. Please try again later." );
		type = MSNSocket::ErrorCannotConnect;
		break;
	// Server error
	default:
		// FIXME: if the error causes a disconnect, it will crash, but we can't disconnect every time
		msg = i18n( "Unhandled MSN error code %1 \n"
			"Please fill a bug report with a detailed description and if possible the last console debug output." ).arg( code );
			// "See http://www.hypothetic.org/docs/msn/basics.php for a description of all error codes."
		break;
	}

	if ( !msg.isEmpty() )
		emit errorMessage( type, msg );

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


	// the command will be sent in slotReadyWrite
	QByteArray bytes;
	const uint length = data.length();
	bytes.duplicate(data.data(), length);
	if(!body.isEmpty())
	{
		uint l = body.size() - (binary ? 0 : 1);
		bytes.resize(length + l);
		for(uint i=0; i < l; i++)
			bytes[length + i] = body[i];
	}

	// Add the request to the queue.
	m_sendQueue.append(bytes);
	m_socket->enableWrite(true);

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
		// If the command queue is not empty, retrieve the first command.
		QValueList<QByteArray>::Iterator it = m_sendQueue.begin();

		if(m_useHttp)
		{
			// If web response data is not pending, send the http request.
			if(!m_pending)
			{
				m_pending = true;
				// Temporarily disable http polling.
				m_bCanPoll = false;
				// Set the host to the msn gateway by default.
				QString host = m_gateway;
				QString query; // Web request query string.

				if(m_bIsFirstInTransaction)
				{
					query.append("Action=open&Server=");
					query.append(m_type);

					query += "&IP=" + m_server;

					m_bIsFirstInTransaction = false;
				}
				else
				{
					// If this is not the first request sent in the transaction,
					// only add the session Id.
					host = m_gwip;
					query += "SessionID=" + m_sessionId;
				}

				// Create the web request headers.
				QString s = makeHttpRequestString(host, query, (*it).size());

				uint length = s.length();
				// Create the web request bytes.
				QByteArray bytes(length + (*it).size());

				// Copy the request headers into the request bytes.
				for(uint i=0; i < length; i++)
					bytes[i] = s.ascii()[i];
				// Copy the request body into the request bytes.
				for(uint i=0; i < (*it).size(); i++)
					bytes[length + i] = (*it)[i];

				kdDebug( 14141 ) << k_funcinfo << "Sending http command: " << QString(*it).stripWhiteSpace() << endl;

				// Write the request bytes to the socket.
				m_socket->writeBlock(bytes.data(), bytes.size());

				// Remove the request from the request queue.
				m_sendQueue.remove(it);

				if(m_sendQueue.isEmpty())
				{
					// Disable sending requests.
					m_socket->enableWrite(false);
					// If the request queue is empty, poll the server.
					m_bCanPoll = true;
				}
			}
		}
		else
		{
			// Otherwise, send the command normally.

			// Simple check to avoid dumping the binary data from the icons and emoticons to kdDebug:
			// When sending an MSN-P2P packet, strip off anything after the P2P header.
			QString debugData = QString( *it ).stripWhiteSpace().replace(
				QRegExp( "(P2P-Dest:.[a-zA-Z@.]*).*" ), "\\1\n\n(Stripped binary data)" );
			kdDebug( 14141 ) << k_funcinfo << "Sending command: " << debugData << endl;

			m_socket->writeBlock( *it, ( *it ).size() );
			m_sendQueue.remove( it );

			// If the queue is empty agalin stop waiting for readyWrite signals
			// because of the CPU usage
			if ( m_sendQueue.isEmpty() )
				m_socket->enableWrite( false );
		}
	}
	else
	{
		m_socket->enableWrite( false );

		if(m_useHttp)
		{
			// If the request queue is empty, poll the server.
			m_bCanPoll = true;
		}
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
		unsigned short character = str[i].unicode();

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
	QString str2 = KURL::decode_string( str, 106 );
	//remove msn+ colors code
	str2 = str2.replace( QRegExp("[\\x1-\\x8]"), "" ); // old msn+ colors
	// added by kaoul <erwin.kwolek at gmail.com>
	str2 = str2.replace( QRegExp("\\xB7[&@\'#0]"),""); // dot ...
	str2 = str2.replace( QRegExp("\\xB7\\$,?\\d{1,2}"),""); // dot dollar (comma)? 0-99

	return str2;
}

void MSNSocket::slotConnectionSuccess()
{
	if(m_useHttp)
	{
		// If we are connected, set the data pending flag to false,
		// and disable http polling.
		m_pending  = false;
		m_bCanPoll = false;
		// If we are connected, start the timer.
		m_timer->start(2000, false);
	}

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

void MSNSocket::slotHttpPoll()
{
	if(m_pending || !m_bCanPoll){
		// If data is pending or poll has been temporary disabled, return.
		return;
	}

	// Create the http request headers.
	const QCString headers = makeHttpRequestString(m_gwip, "Action=poll&SessionID=" + m_sessionId, 0).utf8();
	m_socket->writeBlock(headers, headers.length());
	// Wait for the response.
	m_pending = true;
	m_socket->enableWrite(true);
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

bool MSNSocket::setUseHttpMethod( bool useHttp )
{
	if( m_useHttp == useHttp )
		return true;

	if( useHttp ) {
		QString s = QString( this->className() ).lower();
		if( s == "msnnotifysocket" )
			m_type = "NS";
		else if( s == "msnswitchboardsocket" )
			m_type = "SB";
		else
			m_type = QString::null;

		if( m_type.isNull() )
			return false;

		m_bCanPoll = false;
		m_bIsFirstInTransaction = true;
		m_pending = false;
		m_remaining = 0;
		m_gateway = "gateway.messenger.hotmail.com";
	}

	if ( m_onlineStatus != Disconnected )
		disconnect();

	m_useHttp = useHttp;

	return true;
}

bool MSNSocket::useHttpMethod() const
{
	return m_useHttp;
}

bool MSNSocket::accept( KServerSocket *server )
{
	if ( m_socket )
	{
		kdWarning( 14140 ) << k_funcinfo << "Socket already exists!" << endl;
		return false;
	}

	m_socket = static_cast<KBufferedSocket*>(server->accept());

	if ( !m_socket )
	{
//		kdWarning( 14140 ) << k_funcinfo << "Socket not created.  Error nb" << server->error() << " : " << server->errorString() << endl;
		return false;
	}

	kdDebug( 14140 ) << k_funcinfo << "incoming connection accepted" << endl;

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

QString MSNSocket::makeHttpRequestString(const QString& host, const QString& query, uint contentLength)
{
	QString s(
		"POST http://" + host + "/gateway/gateway.dll?" + query + " HTTP/1.1\r\n" +
		"Accept: */*\r\n" +
		"Accept-Language: en-us\r\n" +
		"User-Agent: MSMSGS\r\n" +
		"Host: " + host + "\r\n" +
		"Proxy-Connection: Keep-Alive\r\n" +
		"Connection: Keep-Alive\r\n" +
		"Pragma: no-cache\r\n" +
		"Content-Type: application/x-msn-messenger\r\n" +
		"Content-Length: " + QString::number(contentLength) + "\r\n" +
		"\r\n");
	return s;
}

MSNSocket::WebResponse::WebResponse(const QByteArray& bytes)
{
	m_statusCode = 0;
	m_stream     = 0;

	int     headerEnd;
  	QString header;
	QString data(QCString(bytes, bytes.size() + 1));

	// Parse the HTTP status header
	QRegExp re("HTTP/\\d\\.\\d (\\d+) ([^\r\n]+)");
	headerEnd  = data.find("\r\n");
	header     = data.left( (headerEnd == -1) ? 20 : headerEnd );

	re.search(header);
	m_statusCode = re.cap(1).toInt();
	m_statusDescription = re.cap(2);

	// Remove the web response status header.
	data = data.mid(headerEnd + 2, (data.find("\r\n\r\n") + 2) - (headerEnd + 2));
	// Create a MimeMessage, removing the HTTP status header
	m_headers = new MimeMessage(data);

	// Retrieve the contentlength header.
	header = m_headers->getValue("Content-Length");
	if(!header.isNull())
	{
		bool valid;
		int length = header.toInt(&valid);
		if(valid && length > 0)
		{
			// If the content length is valid, and not zero,
			// copy the web response content bytes.
			int offset = bytes.size() - length;

			QByteArray content(length);
			for(int i=0; i < length; i++)
				content[i] = bytes[offset + i];
			// Create the web response stream from the response content bytes.
			m_stream = new QDataStream(content, IO_ReadOnly);
		}
	}
}

MSNSocket::WebResponse::~WebResponse()
{
	delete m_headers;
	m_headers = 0;
	delete m_stream;
	m_stream = 0;
}

MimeMessage* MSNSocket::WebResponse::getHeaders()
{
	return m_headers;
}

QDataStream* MSNSocket::WebResponse::getResponseStream()
{
	return m_stream;
}

int MSNSocket::WebResponse::getStatusCode()
{
	return m_statusCode;
}

QString MSNSocket::WebResponse::getStatusDescription()
{
	return m_statusDescription;
}


#include "msnsocket.moc"

// vim: set noet ts=4 sts=4 sw=4:

