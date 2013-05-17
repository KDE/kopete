/*
	oscarclientstream.cpp - Kopete Oscar Protocol
	
	Copyright (c) 2004 Matt Rogers <mattr@kde.org>
	Copyright (c) 2007 Roman Jarosz <kedgedev@centrum.cz>
	
	Based on code Copyright (c) 2004 SuSE Linux AG <http://www.suse.com>
	Based on Iris, Copyright (C) 2003  Justin Karneges <justin@affinix.com>
	
	Kopete (c) 2002-2007 by the Kopete developers <kopete-devel@kde.org>
	
	*************************************************************************
	*                                                                       *
	* This library is free software; you can redistribute it and/or         *
	* modify it under the terms of the GNU Lesser General Public            *
	* License as published by the Free Software Foundation; either          *
	* version 2 of the License, or (at your option) any later version.      *
	*                                                                       *
	*************************************************************************
*/

#include "oscarclientstream.h"

#include <QtCore/QQueue>
#include <QtCore/QTimer>
#include <QtNetwork/QHostAddress>

#include <kdebug.h>

#include "connection.h"
#include "coreprotocol.h"
#include "transfer.h"

#define LIBOSCAR_DEBUG 0

void cs_dump( const QByteArray &bytes );

class ClientStream::Private
{
public:
	Private()
	{
		socket = 0;
		connection = 0;
		newTransfers = false;
	}

	QString host;
	quint16 port;
	QString name;

	QSslSocket *socket;
	CoreProtocol client;
	Connection* connection;

	bool newTransfers;

	QQueue<Transfer*> in;

	QTimer noopTimer; // used to send icq keepalive
	int noop_time;
};

ClientStream::ClientStream( QSslSocket *socket, QObject *parent )
: Stream( parent ), d(new Private())
{
	d->socket = socket;

	connect( d->socket, SIGNAL(connected()), SLOT(socketConnected()) );
	connect( d->socket, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(socketError(QAbstractSocket::SocketError)) );

	connect(d->socket, SIGNAL(disconnected()), SLOT(socketDisconnected()));
	connect(d->socket, SIGNAL(readyRead()), SLOT(socketReadyRead()));
	connect(d->socket, SIGNAL(bytesWritten(qint64)), SLOT(socketBytesWritten(qint64)));
	
	
	connect( &d->client, SIGNAL(outgoingData(QByteArray)),
	         SLOT (cp_outgoingData(QByteArray)) );
	connect( &d->client, SIGNAL(incomingData()),
	         SLOT (cp_incomingData()) );

	d->noop_time = 0;
	connect(&d->noopTimer, SIGNAL(timeout()), SLOT(doNoop()));
}

ClientStream::~ClientStream()
{
	d->noopTimer.stop();

	if ( d->socket->isOpen() )
	{
		kDebug(OSCAR_RAW_DEBUG) << "Socket open, disconnecting...";
		d->socket->disconnectFromHost();
	
		if ( !d->socket->waitForDisconnected( 10000 ) )
		{
			kDebug(OSCAR_RAW_DEBUG) << "Disconnection error!";
			d->socket->close();
		}
	}

	delete d->socket;
	delete d;
}

void ClientStream::connectToServer( const QString& host, quint16 port, bool encrypted, const QString& name )
{
	d->noopTimer.stop();
	if ( d->socket->isOpen() )
	{
		kDebug(OSCAR_RAW_DEBUG) << "Socket open, disconnecting...";
		d->socket->disconnectFromHost();
		
		if ( !d->socket->waitForDisconnected( 10000 ) )
		{
			kDebug(OSCAR_RAW_DEBUG) << "Disconnection error!";
			d->socket->close();
		}
	}
	d->client.reset();
	d->in.clear();
	d->newTransfers = false;
	
	d->host = host;
	d->port = port;
	d->name = name;

	kDebug(OSCAR_RAW_DEBUG) << "Connect to: host" << host << "port" << port << "encrypted" << encrypted << "name" << name;

	if ( encrypted )
	{
		d->socket->ignoreSslErrors();
		d->socket->setPeerVerifyMode(QSslSocket::VerifyNone);
		if ( name.isEmpty() )
			d->socket->connectToHostEncrypted( d->host, d->port );
		else
			d->socket->connectToHostEncrypted( d->host, d->port, d->name );
	} else
	{
		d->socket->connectToHost( d->host, d->port );
	}
}

bool ClientStream::isOpen() const
{
	return d->socket->isOpen();
}

void ClientStream::setNoopTime( int mills )
{
	d->noop_time = mills;

	if( d->noop_time == 0 )
	{
		d->noopTimer.stop();
		return;
	}
	
	if( !d->socket->isOpen() )
		return;
	
	d->noopTimer.start( d->noop_time );
}

QHostAddress ClientStream::localAddress() const
{
	return d->socket->localAddress();
}

int ClientStream::error() const
{
	return d->socket->error();
}

QString ClientStream::errorString() const
{
	return d->socket->errorString();
}

void ClientStream::close()
{
	if( d->socket->isOpen() )
	{
		processNext();
		d->socket->disconnectFromHost();
	}
}

void ClientStream::setConnection( Connection *c )
{
	d->connection = c;
}

Connection* ClientStream::connection() const
{
	return d->connection;
}

bool ClientStream::transfersAvailable() const
{
	return ( !d->in.isEmpty() );
}

Transfer* ClientStream::read()
{
	if( d->in.isEmpty() )
		return 0; //first from queue...
	else 
		return d->in.dequeue();
}

void ClientStream::write( Transfer *request )
{
	d->client.outgoingTransfer( request );
}
	
void cs_dump( const QByteArray &bytes )
{
#if 0
	qDebug( "contains: %i bytes ", bytes.count() );
	uint count = 0;
	while ( count < bytes.count() )
	{
		int dword = 0;
		for ( int i = 0; i < 8; ++i )
		{
			if ( count + i < bytes.count() )
				printf( "%02x ", bytes[ count + i ] );
			else
				printf( "   " );
			if ( i == 3 )
				printf( " " );
		}
		printf(" | ");
		dword = 0;
		for ( int i = 0; i < 8; ++i )
		{
			if ( count + i < bytes.count() )
			{
				int j = bytes [ count + i ];
				if ( j >= 0x20 && j <= 0x7e ) 
					printf( "%2c ", j );
				else
					printf( "%2c ", '.' );
			}
			else
				printf( "   " );
			if ( i == 3 )
				printf( " " );
		}
		printf( "\n" );
		count += 8;
	}
	printf( "\n" );
#endif
	Q_UNUSED( bytes );
}

void ClientStream::cp_outgoingData( const QByteArray& outgoingBytes )
{
	// take formatted bytes from CoreProtocol and put them on the wire
	d->socket->write( outgoingBytes );
}

void ClientStream::cp_incomingData()
{
	Transfer * incoming = d->client.incomingTransfer();
	if ( incoming )
	{
		d->in.enqueue( incoming );
		d->newTransfers = true;
		doReadyRead();
	}
	else
		kDebug(OSCAR_RAW_DEBUG) << 
			"client signalled incomingData but none was available, state is: " <<
			d->client.state() << endl;
}


void ClientStream::socketConnected()
{
	kDebug(OSCAR_RAW_DEBUG) ;

	if ( d->noop_time )
		d->noopTimer.start( d->noop_time );

	emit connected();
}

void ClientStream::socketDisconnected()
{
	kDebug(OSCAR_RAW_DEBUG) ;

	d->noopTimer.stop();
	d->client.reset();
	emit disconnected();
}

void ClientStream::socketError( QAbstractSocket::SocketError socketError )
{
	kDebug(OSCAR_RAW_DEBUG) << " error: " << int(socketError);

	d->noopTimer.stop();

	if ( socketError == QAbstractSocket::RemoteHostClosedError )
		d->socket->abort();
	else
		d->socket->close();

	d->client.reset();

	emit Stream::error( socketError );
}

void ClientStream::socketReadyRead()
{
	QByteArray buffer = d->socket->readAll();

#if LIBOSCAR_DEBUG
	QByteArray cs(buffer.data(), buffer.size()+1);
	kDebug(OSCAR_RAW_DEBUG) << "recv: " << buffer.size() << "bytes";
	cs_dump( buffer );
#endif

	d->client.addIncomingData( buffer );
}

void ClientStream::socketBytesWritten( qint64 bytes )
{
#if LIBOSCAR_DEBUG
 	kDebug(OSCAR_RAW_DEBUG) << bytes << " bytes written";
	Q_UNUSED( bytes );
#else
	Q_UNUSED( bytes );
#endif
}

void ClientStream::doReadyRead()
{
	emit readyRead();
}

void ClientStream::processNext()
{
	if( !d->in.isEmpty() ) 
	{
		QTimer::singleShot(0, this, SLOT(doReadyRead()));
	}
}

void ClientStream::doNoop()
{
	if ( !d->socket->isOpen() )
		return;
	
	FLAP f = { 0x05, d->connection->flapSequence(), 0 };
	Buffer* b = new Buffer(); //deleted in Transfer destructor
	Transfer* t = new FlapTransfer( f, b ); //deleted after being sent
	write( t );
}

#include "oscarclientstream.moc"

//kate: tab-width 4; indent-mode csands;
