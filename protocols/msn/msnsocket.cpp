/*
    msnsocket.cpp - Base class for the sockets used in MSN

    Copyright (c) 2002 by Martijn Klingens       <klingens@kde.org>
    Copyright (c) 2002 by Olivier Goffart        <ogoffart@tiscalinet.be>
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
		kdDebug(14140) << "MSNSocket::connect: WARNING: Already connected or "
			<< "connecting! Not connecting again." << endl;
		return;
	}

	if( m_onlineStatus == Disconnecting )
	{
		// Cleanup first.
		// FIXME: More generic!!!
		kdDebug(14140) << "MSNSocket::connect: WARNING: status is set to 'Disconnecting'" << endl;
		delete m_socket;
	}

	setOnlineStatus( Connecting );
	m_id = 0;
//	m_lastId = 0;
	m_waitBlockSize = 0;
	m_buffer = Buffer(0);

	m_lookupStatus = Processing;

//	m_sendQueue.clear();

	m_server = server;
	m_port = port;
	m_socket = new KExtendedSocket( server, port, 0x600000 );
	m_socket->enableRead( true );
	// enableWrite eats the CPU, and we only need it when the queue is
	// non-empty, so disable it until we have actual data in the queue
	m_socket->enableWrite( false );

	QObject::connect( m_socket, SIGNAL( readyRead() ),
		this, SLOT( slotDataReceived() ) );
	QObject::connect( m_socket, SIGNAL( connectionSuccess() ),
		this, SLOT( slotConnectionSuccess() ) );
	QObject::connect( m_socket, SIGNAL( readyWrite () ),
		this, SLOT( slotReadyWrite() ) );


	QObject::connect( m_socket, SIGNAL( connectionFailed( int ) ),
		this, SLOT( slotSocketError( int ) ) );

	QObject::connect( m_socket, SIGNAL( lookupFinished ( int ) ),
		this, SLOT( slotLookupFinished( int ) ) );

	QObject::connect( m_socket, SIGNAL( closed ( int ) ),
		this, SLOT( slotSocketClosed( int ) ) );

	aboutToConnect();

	// FIXME KDE4?
	// Ideally we want to the full connection to MSN to be handled async,
	// but due to some design issues in QDns this fails if people with
	// dialup connections start Kopete before their internet connection.
	// The workaround from TrollTech is to not use QDns, but use the
	// libc gethostbyname call instead. The sync calls in KExtendedSocket
	// use this, only the async lookup uses DNS.
	// This is slightly annoying as it blocks the GUI for the duration
	// of the DNS lookup, but properly configured systems will hardly
	// notice that. Besides, there's nothing we can do about it...
	// For Qt 4/KDE 4 we can hopefully leave the lookup to the socket
	// again and remove the manual lookup call. This cannot be fixed
	// in Qt 3 unfortunately.
	m_socket->lookup();
	m_socket->startAsyncConnect();
}

void MSNSocket::disconnect()
{
	if(m_socket)
		m_socket->closeNow();
	else
		slotSocketClosed( -1 );
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
	kdDebug(14140) << "MSNSocket::slotSocketError: error: " << error << endl;

	m_socket->cancelAsyncConnect();


	QString errormsg = i18n( "There was an error while connecting to the MSN server.\nError message:\n" );
	if ( m_lookupStatus == Failed )
		errormsg += i18n( "Unable to lookup %1" ).arg( m_socket->host() );
	else
		errormsg += KExtendedSocket::strError( error, m_socket->systemError() );

	//delete m_socket;
	m_socket->deleteLater();
	m_socket = 0L;

	setOnlineStatus( Disconnected );
	emit connectionFailed();
	//like if the socket is closed
	emit( socketClosed( -1 ) );

	KMessageBox::queuedMessageBox( 0L, KMessageBox::Error,
		errormsg, i18n( "MSN Plugin" ) );
}

void MSNSocket::slotDataReceived()
{
	int avail = m_socket->bytesAvailable();
	if ( avail == -1 )
	{
		// error!
		kdDebug(14140) << "MSNSocket::slotDataReceived: bytesAvailable() returned"
			" -1. Are we disconnected?" << endl;
	}

	// incoming data
	char *buf = new char[ avail ];
	int ret = m_socket->readBlock( buf, avail );
	if( ret < 0 )
	{
		kdDebug(14140) << "MSNSocket:slotDataReceived: WARNING: "
			"readBlock() returned " << ret << "!" <<endl;
	}
	else if( ret == 0 )
	{
		kdDebug(14140) << "MSNSocket:slotDataReceived: WARNING: "
			"readBlock() returned no data!" <<endl;
	}
	else
	{
		if( avail )
		{
			if( ret != avail)
			{
				kdDebug(14140) << "MSNSocket:slotDataReceived: WARNING: "
					<< avail << " bytes were reported available, "
					<< "but readBlock() returned only " << ret
					<< " bytes! Proceeding anyway." << endl;
			}
		}
		else
		{
			kdDebug(14140) << "MSNSocket:slotDataReceived: Info: "
				<< "Read " << ret << " bytes into 4kb block." << endl;
		}

		kdDebug( 14140 ) << k_funcinfo << "Received: " << QString( QCString( buf, avail ) ).stripWhiteSpace() << endl;

		m_buffer.add(buf,ret); // fill the buffer with the received data

		slotReadLine();
	}
	// Cleanup
	delete[] buf;
}

void MSNSocket::slotReadLine()
{
	// We have data, first check if it's meant for a block read, otherwise
	// parse the first line (which will recursively parse the other lines)
	if( !pollReadBlock() )
	{
		if(m_buffer.size()>=3 && ( m_buffer.data()[0]=='\0' || m_buffer.data()[0]=='\1'))
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
			//(placed here and not in the end on the function for prevent crash)

			QString command = QString::fromUtf8(m_buffer.take(index+2), index);
			command.replace( "\r\n" , "" );
//			kdDebug(14140) << "MSNSocket::slotReadLine: " << command << endl;

			parseLine(command);
			//WARNING: since here, this can be deleted (when disconnecitng)
		}
	}
}

void MSNSocket::readBlock( uint len )
{
	if( m_waitBlockSize )
	{
		kdDebug(14140) << "MSNSocket::readBlock: WARNING: cannot wait for data "
			<< "block: still waiting for other block of size "
			<< m_waitBlockSize << "! Data will not be returned." << endl;
		return;
	}

	m_waitBlockSize = len;

//	kdDebug(14140) << "MSNSocket::readBlock: Preparing for block read of size " << len << endl;

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
		kdDebug(14140) << "MSNSocket::pollReadBlock: Waiting for data. Received: "
			<< m_buffer.size() << ", required: " << m_waitBlockSize << endl;
		return true;
	}

	QByteArray baBlock = m_buffer.take( m_waitBlockSize );
	QString block = QString::fromUtf8(baBlock, m_waitBlockSize);


//	kdDebug(14140) << "MSNSocket::pollReadBlock: Successfully read block of size " << m_waitBlockSize << endl;

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
	if( !isNum )
		data = str.section( ' ', 1, 1 ) + " " + data;

//	if( isNum && id )
//		m_lastId = id;

//	kdDebug(14140) << "MSNSocket::parseCommand: Parsing command " << cmd <<
//		" (ID " << id << "): '" << data << "'" << endl;

	data.replace(  "\r\n", "" );
	bool isError;
	uint errorCode = cmd.toUInt( &isError );
	if( isError )
		handleError( errorCode, id );
	else
		parseCommand( cmd, id, data );
}

void MSNSocket::handleError( uint code, uint /*id*/ )
{

	QString msg;

	switch (code)
	{
/*		// We cant show message for error we don't know what they are or not related to the correct socket
		//  Theses following messages are not so instructive
	case 205:
		msg = i18n ( "An invalid username has been specified. \n"
			    "Please correct it, and try to reconnect.\n" );
		break;
	case 201:
		msg = i18n ( "Fully Qualified domain name missing. \n" );
		break;
	case 207:
		msg = i18n ( "You are already logged in!\n" );
		break;
	case 208:
		msg = i18n ( "You specified an invalid username.\n"
			     "Please correct it, and try to reconnect.\n");
		break;
	case 209:
		msg = i18n ( "Your nickname is invalid. Please check it, correct it, \n"
			     "and try to reconnect.\n" );
		break;
	case 210:
		msg = i18n ( "Your list has reached its maximum capacity.\n"
			     "No more contacts can be added, unless you remove some first.\n" );
		break;
	case 216:
		 msg = i18n ( "This user is not in your contact list.\n " );
		break;
	case 300:
		msg = i18n ( "Some required fields are missing. Please fill them in and try again.\n" );
		break;
	case 302:
		msg = i18n ( "You are not logged in.\n" );
		break;*/
	case 500:
		disconnect();
		msg = i18n ( "An internal server error occurred. Please try again later.\n " );
		break;
	case 600:
		disconnect();
		msg = i18n ( "The server is busy. Please try again later.\n " );
		break;
	case 601:
		disconnect();
		msg = i18n ( "The server is not available for the moment. Please try again later.\n " );
		break;
	default:
		//FIXME: if the error cause a disconnection, it will crash, but we can't disconnect every time
		msg = i18n( "Unhandled MSN error code %1 \n"
			"Please fill a bug report with a detailed description and if possible the last console debug output. \n" ).arg( code );
			/*"See http://www.hypothetic.org/docs/msn/basics.php for a description of all error codes."*/
		break;
	}

	if (!msg.isEmpty())
	{
		KMessageBox::queuedMessageBox( 0L, KMessageBox::Error, msg, i18n( "MSN Plugin" ) );
	}

	return;
}

int MSNSocket::sendCommand( const QString &cmd, const QString &args,
	bool addId, const QByteArray &body , bool binary)
{
	if(!m_socket)
	{
		kdDebug(14140) << "MSNSocket::sendCommand : WARNING: socket=0L" <<endl;
		return -1;
	}

	QCString data = cmd.utf8();
	if( addId )
		data += " " + QString::number( m_id ).utf8();

	if( !args.isEmpty() )
		data += " " + args.utf8();

	// Add length in bytes, not characters
	if( !body.isEmpty() )
		data += " " + QString::number( body.size() - (binary?0:1) ).utf8();

	data += "\r\n";

	if(!binary)
	{
		if( !body.isEmpty() )
			data += QCString(body,body.size()+1);

	//	kdDebug(14140) << "MSNSocket::sendCommand: Sending command " << data << endl;

		// the command will be sent in slotReadyWrite
		m_sendQueue.append( data );
		m_socket->enableWrite( true );
	}
	else
	{
		QByteArray data2(data.length() + body.size() );
		for(unsigned int f=0; f< data.length(); f++)
			data2[f]=data[f];
		for(unsigned int f=0; f< body.size(); f++)
			data2[data.length() + f]=body[f];

		sendBytes( data2 ) ;

		kdDebug(14140) << "MSNSocket::sendCommand : " << data2.data() <<endl;
	}

	if(addId)
	{
		m_id++;
		return m_id-1;
	}
	return 0;
}

void MSNSocket::slotReadyWrite()
{
	if( !m_sendQueue.isEmpty() )
	{
		QValueList<QCString>::Iterator it = m_sendQueue.begin();
		kdDebug(14140) << "MSNSocket::slotReadyWrite: Sending command " << QString(*it).stripWhiteSpace() << endl;
		m_socket->writeBlock( *it, (*it).length() );
		m_sendQueue.remove( it );
		emit( commandSent() );

		// If the queue is empty again stop waiting for readyWrite signals
		// because of the CPU usage
		if( m_sendQueue.isEmpty() )
			m_socket->enableWrite( false );
	}
	else
		m_socket->enableWrite( false );
}


QString MSNSocket::escape( const QString &str )
{
	return ( KURL::encode_string( str, 106 ) );
}

QString MSNSocket::unescape( const QString &str )
{
	//GRRRRR FU*CKING MSN PLUS USERS!  they insert theses stupid colors code in their nickname, and message are not correctly showed
	return KURL::decode_string( str , 106 ).replace("\3" , "").replace("\4" , "").replace("\2" , "");
}

void MSNSocket::slotConnectionSuccess()
{
	kdDebug(14140) << "MSNSocket::slotConnectionSuccess" << endl;
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
	kdDebug(14140) << "MSNSocket::slotSocketClosed: socket closed. State: 0x" <<
		QString::number( state, 16 ) << endl;

	if(!m_socket ||  m_onlineStatus == Disconnected )
	{
		kdDebug(14140) << "MSNSocket::slotSocketClosed: socket already deleted or already disconnected" << endl;
		return;
	}

	doneDisconnect();

	m_buffer = Buffer(0);
	//delete m_socket;
	m_socket->deleteLater();
	m_socket = 0L;

	emit( socketClosed( state ) );
}


/** Used in MSNFileTransferSocket */
void MSNSocket::bytesReceived(const QByteArray &)
{
	kdDebug(14140) << "MSNSocket::bytesReceived : WARNING: unknow bytes were received" <<endl  ;
}

void MSNSocket::sendBytes(const QByteArray &data)
{
	if(!m_socket)
	{
		kdDebug(14140) << "MSNSocket::sendBytes: WARNING: not yet connected" <<endl  ;
		return;
	}
	m_socket->writeBlock( data, data.size() );
	m_socket->enableWrite( true );
}


bool MSNSocket::accept(KExtendedSocket *server)
{
	if(m_socket)
	{
		kdDebug(14140) << "MSNSocket::accept : WARNING: Socket already exists" <<endl  ;
		return false;
	}
	int acceptResult = server->accept(m_socket);
	kdDebug(14140) << "MSNSocket::accept: result: "<< acceptResult << "  m_socket create : " <<m_socket <<endl  ;
	if(acceptResult !=0)
		return false;
	if(!m_socket)
		return false;

	setOnlineStatus(Connecting);

	m_id = 0;
//	m_lastId = 0;
	m_waitBlockSize = 0;
	m_lookupStatus = Processing;

	m_socket->setBlockingMode(false);
	m_socket->enableRead(true);
	m_socket->enableWrite(true);
	m_socket->setBufferSize(-1, -1);

	QObject::connect( m_socket, SIGNAL( readyRead() ),
		this, SLOT( slotDataReceived() ) );
	QObject::connect( m_socket, SIGNAL( connectionFailed( int ) ),
		this, SLOT( slotSocketError( int ) ) );
	QObject::connect( m_socket, SIGNAL( closed ( int ) ),
		this, SLOT( slotSocketClosed( int ) ) );
	QObject::connect( m_socket, SIGNAL( readyWrite () ),
		this, SLOT( slotReadyWrite() ) );

	m_socket->setSocketFlags( KExtendedSocket::anySocket | KExtendedSocket::inputBufferedSocket | KExtendedSocket::outputBufferedSocket );

	doneConnect();
	return true;
}

QString MSNSocket::getLocalIP()
{
	if(!m_socket)
		return QString::null;

	const KSocketAddress *address= m_socket->localAddress();
	if ( !address  )
	{
		kdDebug(14140) << "MSNFileTransferSocket::getLocalIP: ip not found" <<endl;
		return QString::null;
	}
	QString ip = address->pretty();
	ip = ip.replace( "-", " " );
	if ( ip.contains(" ") )
	{
		ip = ip.left( ip.find(" ") );
	}
	kdDebug(14140) << "MSNFileTransferSocket::getLocalIP: ip: "<< ip  <<endl;
//	delete address;
	return ip;
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

	duplicate(b,size()+sz);
	delete[] b;
}

QByteArray MSNSocket::Buffer::take( unsigned blockSize )
{
	if( size() < blockSize )
	{
		kdWarning() << "MSNSocket::Buffer::take: [WARNING] buffer size "
			<< size() <<" < asked size " << blockSize << "!" << endl;
		return QByteArray();
	}

	QByteArray rep( blockSize );
	for( unsigned i = 0; i < blockSize; i++ )
		rep[ i ] = data()[ i ];

	char *str = new char[ size() - blockSize ];
	for( unsigned i = 0; i < size() - blockSize; i++ )
		str[ i ] = data()[ blockSize + i ];
	duplicate( str, size() - blockSize );
	delete[] str;

	return rep;
}

#include "msnsocket.moc"


