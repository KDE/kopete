/*
	oscarclientstream.cpp - Kopete Oscar Protocol
	
	Copyright (c) 2004 Matt Rogers <mattr@kde.org>
	
	Based on code Copyright (c) 2004 SuSE Linux AG <http://www.suse.com>
	Based on Iris, Copyright (C) 2003  Justin Karneges
	
	Kopete (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>
	
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

#include <qguardedptr.h> 
#include <qobject.h>
#include <qptrqueue.h>
#include <qtimer.h>

#include <kdebug.h>

#include "bytestream.h"
#include "connection.h"
#include "connector.h"
#include "coreprotocol.h"
#include "rateclassmanager.h"
#include "transfer.h"

#define LIBOSCAR_DEBUG 0

void cs_dump( const QByteArray &bytes );

enum {
	Idle,
	Connecting,
	Active,
	Closing
};

enum {
	ClientMode,
	ServerMode
};

class ClientStream::Private
{
public:
	Private()
	{
		conn = 0;
		bs = 0;
		connection = 0;
		
		username = QString::null;
		password = QString::null;
		server = QString::null;
		haveLocalAddr = false;
		doBinding = true;

		reset();
	}
	void reset()
	{
		state = Idle;
		notify = 0;
		newTransfers = false;
	}
	
	QString username;
	QString password;
	QString server;
	bool doAuth; //send the initial login sequences to get the cookie
	bool haveLocalAddr;
	QHostAddress localAddr;
	Q_UINT16 localPort;
	bool doBinding;

	Connector *conn;
	ByteStream *bs;
	CoreProtocol client;
	Connection* connection;

	QString defRealm;

	int mode;
	int state;
	int notify;
	bool newTransfers;
	
	int errCond;
	QString errText;

	QPtrQueue<Transfer> in;

	QTimer noopTimer; // used to send icq keepalive
	int noop_time;
};

ClientStream::ClientStream(Connector *conn, QObject *parent)
:Stream(parent)
{
	//qDebug("CLIENTSTREAM::ClientStream");

	d = new Private;
	d->mode = ClientMode;
	d->conn = conn;
	connect( d->conn, SIGNAL(connected()), SLOT(cr_connected()) );
	connect( d->conn, SIGNAL(error()), SLOT(cr_error()) );
	connect( &d->client, SIGNAL( outgoingData( const QByteArray& ) ), SLOT ( cp_outgoingData( const QByteArray & ) ) );
	connect( &d->client, SIGNAL( incomingData() ), SLOT ( cp_incomingData() ) );

	d->noop_time = 0;
	connect(&d->noopTimer, SIGNAL(timeout()), SLOT(doNoop()));
}

ClientStream::~ClientStream()
{
	reset();
	delete d;
}

void ClientStream::reset(bool all)
{
	d->reset();
	d->noopTimer.stop();

	// client
	if(d->mode == ClientMode)
	{
		// reset connector
		if ( d->bs )
		{
			d->bs->close();
			d->bs = 0;
		}
		if ( d->conn )
			d->conn->done();

		// reset state machine
		d->client.reset();
	}
	if(all)
		d->in.clear();
}

void ClientStream::connectToServer(const QString& server, bool auth)
{
	reset(true);
	d->state = Connecting;
	d->doAuth = auth;
	d->server = server;

	d->conn->connectToServer( d->server );
}

void ClientStream::continueAfterWarning()
{
/* unneeded?
	if(d->state == WaitVersion) {
		d->state = Connecting;
		processNext();
	}
	else if(d->state == WaitTLS) {
		d->state = Connecting;
		processNext();
	}
*/
}

void ClientStream::accept()
{

}

bool ClientStream::isActive() const
{
	return (d->state != Idle);
}

bool ClientStream::isAuthenticated() const
{
	return (d->state == Active);
}

void ClientStream::setNoopTime(int mills)
{
	d->noop_time = mills;

	if(d->noop_time == 0) {
		d->noopTimer.stop();
		return;
	}
	
	if( d->state != Active )
		return;
	
	d->noopTimer.start( d->noop_time );
}

void ClientStream::setLocalAddr(const QHostAddress &addr, Q_UINT16 port)
{
	d->haveLocalAddr = true;
	d->localAddr = addr;
	d->localPort = port;
}

int ClientStream::errorCondition() const
{
	return d->errCond;
}

QString ClientStream::errorText() const
{
	return d->errText;
}

void ClientStream::close()
{
	if(d->state == Active) {
		d->state = Closing;
//		d->client.shutdown();
		processNext();
	}
	else if(d->state != Idle && d->state != Closing) {
		reset();
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
	if(d->in.isEmpty())
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
	d->bs->write( outgoingBytes );
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
		kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << 
			"client signalled incomingData but none was available, state is: " <<
			d->client.state() << endl;
}

void ClientStream::cr_connected()
{
	d->bs = d->conn->stream();
	connect(d->bs, SIGNAL(connectionClosed()), SLOT(bs_connectionClosed()));
	connect(d->bs, SIGNAL(delayedCloseFinished()), SLOT(bs_delayedCloseFinished()));
	connect(d->bs, SIGNAL(readyRead()), SLOT(bs_readyRead()));
	connect(d->bs, SIGNAL(bytesWritten(int)), SLOT(bs_bytesWritten(int)));
	connect(d->bs, SIGNAL(error(int)), SLOT(bs_error(int)));

	d->state = Active;
	if ( d->noop_time )
		d->noopTimer.start( d->noop_time );

	QByteArray spare = d->bs->read();

	QGuardedPtr<QObject> self = this;
	emit connected();
	if(!self)
		return;
}

void ClientStream::cr_error()
{
	kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << endl;
	reset();
	emit error(ErrConnection);
}

void ClientStream::bs_connectionClosed()	
{
	reset();
	emit connectionClosed();
}

void ClientStream::bs_delayedCloseFinished()
{
	// we don't care about this (we track all important data ourself)
}

void ClientStream::bs_error(int)
{
	// TODO
}

void ClientStream::bs_readyRead()
{
	QByteArray a;
	//qDebug( "size of storage for incoming data is %i bytes.", a.size() );
	a = d->bs->read();

#if LIBOSCAR_DEBUG
	QCString cs(a.data(), a.size()+1);
	kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "recv: " << a.size() << "bytes" << endl;
	cs_dump( a );
#endif

	d->client.addIncomingData(a);
}

void ClientStream::bs_bytesWritten(int bytes)
{
#if LIBOSCAR_DEBUG
 	kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << bytes << " bytes written" << endl;
	Q_UNUSED( bytes );
#else
	Q_UNUSED( bytes );
#endif
}

void ClientStream::srvProcessNext()
{
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

bool ClientStream::handleNeed()
{
	return false;
}

void ClientStream::doNoop()
{
	if ( d->state != Active )
		return;
	
	FLAP f = { 0x05, d->connection->flapSequence(), 0 };
	Buffer* b = new Buffer(); //deleted in Transfer destructor
	Transfer* t = new FlapTransfer( f, b ); //deleted after being sent
	write( t );
}

void ClientStream::handleError()
{
}

#include "oscarclientstream.moc"

//kate: tab-width 4; indent-mode csands;
