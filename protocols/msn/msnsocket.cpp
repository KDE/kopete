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
//#include "msnprotocol.h"

#include <qregexp.h>
#include <qtimer.h>

#include <kdebug.h>
#include <kextsock.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kurl.h>


MSNSocket::MSNSocket(QObject* parent)  : QObject (parent)
{
	m_onlineStatus = Disconnected;
	m_socket = 0L;
}

MSNSocket::~MSNSocket()
{
/*	if( m_onlineStatus != Disconnected )
		disconnect();*/
	doneDisconnect();
	delete m_socket;
	/*if(m_socket)
		m_socket->deleteLater();*/
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
		kdDebug() << "MSNSocket::connect: WARNING: status is set to 'Disconnecting'" << endl;
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
	if(m_socket)
		m_socket->closeNow();
	else
		emit socketClosed(-1);
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
	kdDebug() << "MSNSocket::slotSocketError: error: " << error << endl;

	m_socket->cancelAsyncConnect();

	QString errormsg = i18n( "There was an error while connecting to the MSN server.\nError message:\n" );

	if ( m_lookupStatus == Failed )
		errormsg += i18n( "Unable to lookup %1" ).arg( m_socket->host() );
	else
		errormsg += KExtendedSocket::strError( error, m_socket->systemError() );

	KMessageBox::error( 0, errormsg, i18n( "MSN Plugin - Kopete" ) );

	// Emit the signal even if we still were disconnected.
//	if ( onlineStatus() == Disconnected )
//		emit( onlineStatusChanged( Disconnected ) );

	setOnlineStatus( Disconnected );

	delete m_socket;
	m_socket = 0L;


	emit connectionFailed();
	emit socketClosed(-1); //like the socket is closed
}

void MSNSocket::slotDataReceived()
{
	int avail = m_socket->bytesAvailable();
	int toRead = avail;
	if( avail == 0 )
	{
		kdDebug() << "MSNSocket::slotDataReceived:\n"
			"** WARNING ** bytesAvailable() returned 0!\n"
			"If you are running KDE 3.0.0, please upgrade to a newer KDE\n"
			"version. This fixes a KExtendedSocket bug always returning 0.\n"
			"Trying to read 4KB blocks instead, but be prepared for problems!"
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
	char *buf = new char[ toRead +1];
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
		kdDebug() << "MSNSocket::slotDataReceived: Received '" <<			buf << "'" << endl;
    
		m_buffer.add(buf,ret); // fill the buffer with the received data


		slotReadLine();
	}

	// Cleanup
	delete[] buf;
}

void MSNSocket::slotReadLine()
{
	// See if we have pending changes in the queue...
	if( !m_sendQueue.isEmpty() )
	{
		kdDebug() << "MSNSocket::slotReadLine: Send queue not empty,"
			" attempting to flush first item. m_lastId: " << m_lastId << endl;
		for(QMap<uint, QCString>::Iterator it = m_sendQueue.begin(); it!=m_sendQueue.end() ; ++it)
		{
			if( m_lastId >= it.key() - 1 )
			{
				kdDebug() << "MSNSocket::slotReadLine: Flushing entry from send queue: "
					<< it.data() << endl;
				m_socket->writeBlock( it.data(), it.data().length() );
				m_sendQueue.remove( it );
			}
		}
	}

	// We have data, first check if it's meant for a block read, otherwise
	// parse the first line (which will recursively parse the other lines)
	if( !pollReadBlock() )
	{
		if(m_buffer.size()>=3 && m_buffer.data()[0]=='\0')
		{
			bytesReceived(m_buffer.take(3));
			return;
		}

		int index = -1;
		for (unsigned int x = 0;(x + 1) < m_buffer.size();x++)
		{
			if ((m_buffer[x] == '\r') &&
			    (m_buffer[x+1] == '\n'))
			{
				index = x;
				break;
			}
		}
		if( index != -1 )
		{
			// Don't block the GUI while parsing data, only do a single line!
			QTimer::singleShot( 0, this, SLOT( slotReadLine() ) );
			//(placed here and not in the end on the fuction for prevent crash)

			QString command = QString::fromUtf8(m_buffer.take(index+2), index);
			command.replace( QRegExp( "\r\n" ), "" );
			kdDebug() << "MSNSocket::slotReadLine: " << command << endl;
			
			parseLine(command);
			//WARNING: since here, this can be deleted (when disconnecitng)
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

//	kdDebug() << "MSNSocket::readBlock: Preparing for block read of size " << len << endl;

	// Try to return the data now, if available. Otherwise slotDataReady
	// will do this whenever all data is there.
	pollReadBlock();
}

bool MSNSocket::pollReadBlock()
{
	if( !m_waitBlockSize )
		return false;
	else if( m_buffer.size() < m_waitBlockSize )
	{
		kdDebug() << "MSNSocket::pollReadBlock: Waiting for data. Received: "
			<< m_buffer.size() << ", required: " << m_waitBlockSize << endl;
		return true;
	}

	QByteArray baBlock = m_buffer.take( m_waitBlockSize );
	QString block = QString::fromUtf8(baBlock, m_waitBlockSize);


//	kdDebug() << "MSNSocket::pollReadBlock: Successfully read block of size " << m_waitBlockSize << endl;

	m_waitBlockSize = 0;
	emit blockRead( baBlock );
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

//	kdDebug() << "MSNSocket::parseCommand: Parsing command " << cmd <<
//		" (ID " << id << "): '" << data << "'" << endl;

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
	if(code==500)
	{
		QString msg = i18n( "Internal server error!\n"
				"The server is maybe down for maintenance. Try later " );

		KMessageBox::error( 0, msg, i18n( "MSN Plugin - Kopete" ) );
		return;
	}

	QString msg =
		i18n( "Unhandled MSN error code %1 (in response to "
			"transaction ID %2).\n"
			"Please mail kopete-devel@kde.org to ask for an implementation, "
			"or send in a patch yourself.\n"
			"See http://www.hypothetic.org/docs/msn/basics.php for a "
			"description of all error codes." ).arg( code ).arg( id );
	//disconnect();
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

	if(!m_socket ||  m_onlineStatus == Disconnected )
	{
		kdDebug() << "MSNSocket::slotSocketClosed: socket already deleted or already disconnected" << endl;
		return;
	}

	doneDisconnect();

//	kdDebug() << "MSNSocket::slotSocketClosed: delete socket " << m_socket << endl;
	m_buffer = Buffer(0);
	delete m_socket;
	//m_socket->deleteLater();
	m_socket = 0L;

	emit( socketClosed( state ) );
}



/** Used in MSNFileTransferSocket */
void MSNSocket::bytesReceived(const QByteArray &)
{
	kdDebug() << "MSNSocket::bytesReceived : WARNING: unknow bytes were received" <<endl  ;
}



MSNSocket::Buffer::Buffer(unsigned int sz) : QByteArray(sz) {}

MSNSocket::Buffer::~Buffer() {}

void MSNSocket::Buffer::add(char *str, unsigned int sz)
{
	char *b=new char[size()+sz];
	for(unsigned int f=0;f<size(); f++)
		b[f]=data()[f];
	for(unsigned int f=0;f<sz; f++)
		b[size()+f]=str[f];

	assign(b,size()+sz);
}

QByteArray MSNSocket::Buffer::take(unsigned int sz)
{
	if(size()<sz)
	{
		kdDebug() << "MSNSocket::Buffer::take : [WARNING] buffer size ("<< size()<<") < asked size (" << sz <<") " << endl;
		return QByteArray();
	}
	char *str=new char[sz];
	for (unsigned int f=0;f<sz;f++)
		str[f] = data()[f];

	QByteArray rep=QByteArray().assign(str,sz);

	str=new char[size()-sz+1];
	for(unsigned int f=0;f<size()-sz;f++)
		str[f]=data()[sz+f];
	str[size()-sz]='\0';
	assign(str,size()-sz);

	return rep;
}


#include "msnsocket.moc"


/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

