/*
	oscarclientstream.cpp - Kopete Oscar Protocol
	
	Copyright (c) 2004 Matt Rogers <matt.rogers@kdemail.net>
	
	Based on code Copyright (c) 2004 SuSE Linux AG <http://www.suse.com>
	Based on Iris, Copyright (C) 2003  Justin Karneges <justin@affinix.com>
	
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

#include "yahooclientstream.h"

#include <QApplication>  // for qdebug
#include <QPointer> 
#include <QObject>
#include <QTimer>
#include <QQueue>

#include "yahoo_protocol_debug.h"

#include "bytestream.h"
#include "connector.h"
#include "coreprotocol.h"
#include "transfer.h"

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
		
		username.clear();
		password.clear();
		server.clear();
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
	quint16 localPort;
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

	QQueue<Transfer *> in;

	QTimer noopTimer; // used to send icq keepalive
	int noop_time;
};

ClientStream::ClientStream(Connector *conn, QObject *parent)
:Stream(parent), d(new Private())
{
	qCDebug(YAHOO_PROTOCOL_LOG) ;
	
	d->mode = Client;
	d->conn = conn;
	connect( d->conn, SIGNAL(connected()), SLOT(cr_connected()) );
	connect( d->conn, SIGNAL(error()), SLOT(cr_error()) );
	connect( &d->client, SIGNAL(outgoingData(QByteArray)), SLOT (cp_outgoingData(QByteArray)) );
	connect( &d->client, SIGNAL(incomingData()), SLOT (cp_incomingData()) );

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
	qCDebug(YAHOO_PROTOCOL_LOG) ;
	d->reset();
	d->noopTimer.stop();

	// client
	if(d->mode == Client) {
		
		// reset connector
		if(d->bs) {
			disconnect(d->bs, 0, this, 0);
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
	qCDebug(YAHOO_PROTOCOL_LOG) ;
	reset(true);
	d->state = Connecting;
	d->doAuth = auth;
	d->server = server;

	d->conn->connectToServer( d->server );
}

void ClientStream::continueAfterWarning()
{
	qCDebug(YAHOO_PROTOCOL_LOG) ;
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

void ClientStream::setLocalAddr(const QHostAddress &addr, quint16 port)
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
	qCDebug(YAHOO_PROTOCOL_LOG) ;
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
	qCDebug(YAHOO_PROTOCOL_LOG) ;
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
	if ( !d->bs )
		return;
	
	// take formatted bytes from CoreProtocol and put them on the wire
	qCDebug(YAHOO_PROTOCOL_LOG) << "[data size: " << outgoingBytes.size() << "]";
	//cs_dump( outgoingBytes );
	d->bs->write( outgoingBytes );
}

void ClientStream::cp_incomingData()
{
// 	qCDebug(YAHOO_PROTOCOL_LOG) ;
	Transfer * incoming = d->client.incomingTransfer();
	if ( incoming )
	{
// 		qCDebug(YAHOO_PROTOCOL_LOG) << " - got a new transfer";
		d->in.enqueue( incoming );
		d->newTransfers = true;
		emit doReadyRead();
	}
	else
		qCDebug(YAHOO_PROTOCOL_LOG) << " - client signalled incomingData but none was available, state is: "<< d->client.state();
}

/* Connector connected */
void ClientStream::cr_connected()
{
	qCDebug(YAHOO_PROTOCOL_LOG) ;
	
	d->bs = d->conn->stream();
	connect(d->bs, SIGNAL(connectionClosed()), SLOT(bs_connectionClosed()));
	connect(d->bs, SIGNAL(delayedCloseFinished()), SLOT(bs_delayedCloseFinished()));
	connect(d->bs, SIGNAL(readyRead()), SLOT(bs_readyRead()));
	connect(d->bs, SIGNAL(bytesWritten(int)), SLOT(bs_bytesWritten(int)));
	connect(d->bs, SIGNAL(error(int)), SLOT(bs_error(int)));

	QByteArray spare = d->bs->read();

	QPointer<QObject> self = this;
	emit connected();
	if(!self)
		return;
}

void ClientStream::cr_error()
{
	qCDebug(YAHOO_PROTOCOL_LOG) ;
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
	qCDebug(YAHOO_PROTOCOL_LOG) ;
	// TODO
}

void ClientStream::bs_readyRead()
{
// 	qCDebug(YAHOO_PROTOCOL_LOG) ;
	QByteArray a;
	//qDebug( "size of storage for incoming data is %i bytes.", a.size() );
	a = d->bs->read();

	//QCString cs(a.data(), a.size()+1);
	//qDebug("ClientStream: recv: %d [%s]\n", a.size(), cs.data());
	//qCDebug(YAHOO_PROTOCOL_LOG) << " recv: " << a.size()  <<" bytes";
	//cs_dump( a );

	d->client.addIncomingData(a);
}

void ClientStream::bs_bytesWritten(int bytes)
{
	qCDebug(YAHOO_PROTOCOL_LOG) << " written: " << bytes  <<" bytes";
}

void ClientStream::srvProcessNext()
{
}

void ClientStream::doReadyRead()
{
// 	qCDebug(YAHOO_PROTOCOL_LOG) ;
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

