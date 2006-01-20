/*
	oscarclientstream.cpp - Kopete Oscar Protocol
	
	Copyright (c) 2004 Matt Rogers <matt.rogers@kdemail.net>
	
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



#include <qapplication.h>  // for qdebug
#include <qguardedptr.h> 
#include <qobject.h>
#include <qptrqueue.h>
#include <qtimer.h>

#include <kdebug.h>

#include "bytestream.h"
#include "connector.h"
#include "coreprotocol.h"
#include "transfer.h"

#include "yahooclientstream.h"
#include "yahootypes.h"

void cs_dump( const QByteArray &bytes );

enum {
	Idle,
	Connecting,
	Active,
	Closing
};

enum {
	Client,
	Server
};

class ClientStream::Private
{
public:
	Private()
	{
		conn = 0;
		bs = 0;
		
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
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;
	
	d = new Private;
	d->mode = Client;
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
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;
	d->reset();
	d->noopTimer.stop();

	// client
	if(d->mode == Client) {
		
		// reset connector
		if(d->bs) {
			d->bs->close();
			d->bs = 0;
		}
		d->conn->done();

		// reset state machine
		d->client.reset();
	}
	if(all)
		d->in.clear();
}

void ClientStream::connectToServer(const QString& server, bool auth)
{
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;
	reset(true);
	d->state = Connecting;
	d->doAuth = auth;
	d->server = server;

	d->conn->connectToServer( d->server );
}

void ClientStream::continueAfterWarning()
{
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;
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

	if(d->state != Active)
		return;

	if(d->noop_time == 0) {
		d->noopTimer.stop();
		return;
	}
	d->noopTimer.start(d->noop_time);
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

bool ClientStream::transfersAvailable() const
{
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;
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
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;
	// pass to CoreProtocol for transformation into wire format
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
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << "[data size: " << outgoingBytes.size() << "]" << endl;
	//cs_dump( outgoingBytes );
	d->bs->write( outgoingBytes );
}

void ClientStream::cp_incomingData()
{
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;
	Transfer * incoming = d->client.incomingTransfer();
	if ( incoming )
	{
		kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << " - got a new transfer" << endl;
		d->in.enqueue( incoming );
		d->newTransfers = true;
		emit doReadyRead();
	}
	else
		kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << " - client signalled incomingData but none was available, state is: "<< d->client.state() << endl;
}

/* Connector connected */
void ClientStream::cr_connected()
{
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;
	
	d->bs = d->conn->stream();
	connect(d->bs, SIGNAL(connectionClosed()), SLOT(bs_connectionClosed()));
	connect(d->bs, SIGNAL(delayedCloseFinished()), SLOT(bs_delayedCloseFinished()));
	connect(d->bs, SIGNAL(readyRead()), SLOT(bs_readyRead()));
	connect(d->bs, SIGNAL(bytesWritten(int)), SLOT(bs_bytesWritten(int)));
	connect(d->bs, SIGNAL(error(int)), SLOT(bs_error(int)));

	QByteArray spare = d->bs->read();

	QGuardedPtr<QObject> self = this;
	emit connected();
	if(!self)
		return;
}

void ClientStream::cr_error()
{
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;
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
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;
	// TODO
}

void ClientStream::bs_readyRead()
{
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;
	QByteArray a;
	//qDebug( "size of storage for incoming data is %i bytes.", a.size() );
	a = d->bs->read();

	//QCString cs(a.data(), a.size()+1);
	//qDebug("ClientStream: recv: %d [%s]\n", a.size(), cs.data());
	//kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << " recv: " << a.size()  <<" bytes" <<endl;
	//cs_dump( a );

	d->client.addIncomingData(a);
}

void ClientStream::bs_bytesWritten(int bytes)
{
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << " written: " << bytes  <<" bytes" <<endl;
}

void ClientStream::srvProcessNext()
{
}

void ClientStream::doReadyRead()
{
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;
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
}

void ClientStream::handleError()
{
}

#include "yahooclientstream.moc"
