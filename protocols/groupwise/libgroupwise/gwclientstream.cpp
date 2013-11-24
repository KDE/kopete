/*
    gwclientstream.cpp - Kopete Groupwise Protocol
  
    Copyright (c) 2006      Novell, Inc	 	 	 http://www.opensuse.org
    Copyright (c) 2004      SUSE Linux AG	 	 http://www.suse.com
    
    Based on Iris, Copyright (C) 2003  Justin Karneges <justin@affinix.com>
    encode_method from Gaim src/protocols/novell/nmconn.c
    Copyright (c) 2004 Novell, Inc. All Rights Reserved
    
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

//#include<qtextstream.h>
//#include<qguardedptr.h>
// #include<qca.h>
// #include<stdlib.h>
// #include"bytestream.h"
// #include"base64.h"
// #include"hash.h"
// #include"simplesasl.h"
// #include"securestream.h"
// #include"protocol.h"

#include "gwclientstream.h"

#include <qapplication.h>  // for qdebug
#include <qpointer.h> 
#include <qobject.h>
#include <QQueue>
#include <qtimer.h>
#include <QByteArray>

#include "bytestream.h"
#include "connector.h"
#include "coreprotocol.h"
#include "request.h"
#include "securestream.h"
#include "tlshandler.h"

//#include "iostream.h"

//#define LIBGW_DEBUG 1

void cs_dump( const QByteArray &bytes );

enum {
	Idle,
	Connecting,
	WaitVersion,
	WaitTLS,
	NeedParams,
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
		ss = 0;
		tlsHandler = 0;
		tls = 0;
//		sasl = 0;

		allowPlain = false;
		mutualAuth = false;
		haveLocalAddr = false;
/*		minimumSSF = 0;
		maximumSSF = 0;*/
		doBinding = true;

		in_rrsig = false;

		reset();
	}
	void reset()
	{
		state = Idle;
		notify = 0;
		newTransfers = false;
// 		sasl_ssf = 0;
		tls_warned = false;
		using_tls = false;
	}
	
	NovellDN id;
	QString server;
	bool oldOnly;
	bool allowPlain, mutualAuth;
	bool haveLocalAddr;
	QHostAddress localAddr;
	quint16 localPort;
// 	int minimumSSF, maximumSSF;
// 	QString sasl_mech;
	bool doBinding;

	bool in_rrsig;

	Connector *conn;
	ByteStream *bs;
	TLSHandler *tlsHandler;
	QCA::TLS *tls;
// 	QCA::SASL *sasl;
	SecureStream *ss;
	CoreProtocol client;
	//CoreProtocol srv;

	QString defRealm;

	int mode;
	int state;
	int notify;
	bool newTransfers;
// 	int sasl_ssf;
	bool tls_warned, using_tls;
	bool doAuth;

// 	QStringList sasl_mechlist;

	int errCond;
	QString errText;

	QQueue<Transfer *> in;

	QTimer noopTimer; // probably not needed
	int noop_time;
};

ClientStream::ClientStream(Connector *conn, TLSHandler *tlsHandler, QObject *parent)
:Stream(parent), d(new Private())
{
	d->mode = Client;
	d->conn = conn;
	connect( d->conn, SIGNAL(connected()), SLOT(cr_connected()) );
	connect( d->conn, SIGNAL(error()), SLOT(cr_error()) );
	connect( &d->client, SIGNAL(outgoingData(QByteArray)), SLOT (cp_outgoingData(QByteArray)) );
	connect( &d->client, SIGNAL(incomingData()), SLOT (cp_incomingData()) );

	d->noop_time = 0;
	connect(&d->noopTimer, SIGNAL(timeout()), SLOT(doNoop()));

	d->tlsHandler = tlsHandler;		// all the extra stuff happening in the larger ctor happens at connect time :)
}

ClientStream::~ClientStream()
{
	reset( true );
	delete d;
}

void ClientStream::reset(bool all)
{
	d->reset();
	d->noopTimer.stop();

	// delete securestream
	delete d->ss;
	d->ss = 0;

	// reset sasl
// 	delete d->sasl;
// 	d->sasl = 0;

	// client
	if(d->mode == Client) {
		// reset tls
		if(d->tlsHandler)
			d->tlsHandler->reset();

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
		while (!d->in.isEmpty())
			delete d->in.dequeue();
}

// Jid ClientStream::jid() const
// {
// 	return d->jid;
// }

void ClientStream::connectToServer(const NovellDN &id, bool auth)
{
	reset(true);
	d->state = Connecting;
	d->id = id;
	d->doAuth = auth;
	d->server = d->id.server;

	d->conn->connectToServer( d->server );
}

void ClientStream::continueAfterWarning()
{
	if(d->state == WaitVersion) {
		// if we don't have TLS yet, then we're never going to get it
		if(!d->tls_warned && !d->using_tls) {
			d->tls_warned = true;
			d->state = WaitTLS;
			emit warning(WarnNoTLS);
			return;
		}
		d->state = Connecting;
		processNext();
	}
	else if(d->state == WaitTLS) {
		d->state = Connecting;
		processNext();
	}
}

void ClientStream::accept()
{
/*	d->srv.host = d->server;
	processNext();*/
}

bool ClientStream::isActive() const
{
	return (d->state != Idle);
}

bool ClientStream::isAuthenticated() const
{
	return (d->state == Active);
}

// void ClientStream::setPassword(const QString &s)
// {
// 	if(d->client.old) {
// 		d->client.setPassword(s);
// 	}
// 	else {
// 		if(d->sasl)
// 			d->sasl->setPassword(s);
// 	}
// }

// void ClientStream::setRealm(const QString &s)
// {
// 	if(d->sasl)
// 		d->sasl->setRealm(s);
// }

void ClientStream::continueAfterParams()
{
/*	if(d->state == NeedParams) {
		d->state = Connecting;
		if(d->client.old) {
			processNext();
		}
		else {
			if(d->sasl)
				d->sasl->continueAfterParams();
		}
	}*/
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

// QDomElement ClientStream::errorAppSpec() const
// {
// 	return d->errAppSpec;cr_error
// }

// bool ClientStream::old() const
// {
// 	return d->client.old;
// }

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

void ClientStream::setAllowPlain(bool b)
{
	d->allowPlain = b;
}

void ClientStream::setRequireMutualAuth(bool b)
{
	d->mutualAuth = b;
}

// void ClientStream::setSSFRange(int low, int high)
// {
// 	d->minimumSSF = low;
// 	d->maximumSSF = high;
// }

// void ClientStream::setOldOnly(bool b)
// {
// 	d->oldOnly = b;
// }

bool ClientStream::transfersAvailable() const
{
	return ( !d->in.isEmpty() );
}

Transfer * ClientStream::read()
{
	if(d->in.isEmpty())
		return 0; //first from queue...
	else 
		return d->in.dequeue();
}

void ClientStream::write( Request *request )
{
	// pass to CoreProtocol for transformation into wire format
	d->client.outgoingTransfer( request );
}
	
void cs_dump( const QByteArray &bytes )
{
//#define GW_CLIENTSTREAM_DEBUG 1
#ifdef GW_CLIENTSTREAM_DEBUG
	CoreProtocol::debug( QString( "contains: %1 bytes " ).arg( bytes.count() ) );
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
#else
	Q_UNUSED( bytes );
#endif
}

void ClientStream::cp_outgoingData( const QByteArray& outgoingBytes )
{
	// take formatted bytes from CoreProtocol and put them on the wire
#ifdef LIBGW_DEBUG
	CoreProtocol::debug( "ClientStream::cp_outgoingData:" );
	cs_dump( outgoingBytes );
#endif	
	d->ss->write( outgoingBytes );
}

void ClientStream::cp_incomingData()
{
	CoreProtocol::debug( "ClientStream::cp_incomingData:" );
	Transfer * incoming = d->client.incomingTransfer();
	if ( incoming )
	{
		CoreProtocol::debug( " - got a new transfer" );
		d->in.enqueue( incoming );
		d->newTransfers = true;
		emit doReadyRead();
	}
	else
		CoreProtocol::debug( QString( " - client signalled incomingData but none was available, state is: %1" ).arg( d->client.state() ) );
}

void ClientStream::cr_connected()
{
	d->bs = d->conn->stream();
	connect(d->bs, SIGNAL(connectionClosed()), SLOT(bs_connectionClosed()));
	connect(d->bs, SIGNAL(delayedCloseFinished()), SLOT(bs_delayedCloseFinished()));

	QByteArray spare = d->bs->read();

	d->ss = new SecureStream(d->bs);
	connect(d->ss, SIGNAL(readyRead()), SLOT(ss_readyRead()));
	connect(d->ss, SIGNAL(bytesWritten(int)), SLOT(ss_bytesWritten(int)));
	connect(d->ss, SIGNAL(tlsHandshaken()), SLOT(ss_tlsHandshaken()));
	connect(d->ss, SIGNAL(tlsClosed()), SLOT(ss_tlsClosed()));
	connect(d->ss, SIGNAL(error(int)), SLOT(ss_error(int)));

	//d->client.startDialbackOut("andbit.net", "im.pyxa.org");
	//d->client.startServerOut(d->server);

// 	d->client.startClientOut(d->jid, d->oldOnly, d->conn->useSSL(), d->doAuth);
// 	d->client.setAllowTLS(d->tlsHandler ? true: false);
// 	d->client.setAllowBind(d->doBinding);
// 	d->client.setAllowPlain(d->allowPlain);

	/*d->client.jid = d->jid;
	d->client.server = d->server;
	d->client.allowPlain = d->allowPlain;
	d->client.oldOnly = d->oldOnly;
	d->client.sasl_mech = d->sasl_mech;
	d->client.doTLS = d->tlsHandler ? true: false;
	d->client.doBinding = d->doBinding;*/

	QPointer<QObject> self = this;
	emit connected();
	if(!self)
		return;

	// immediate SSL?
	if(d->conn->useSSL()) {
		CoreProtocol::debug( "CLIENTSTREAM: cr_connected(), starting TLS" );
		d->using_tls = true;
		d->ss->startTLSClient(d->tlsHandler, d->server, spare);
	}
	else {
/*		d->client.addIncomingData(spare);
		processNext();*/
	}
}

void ClientStream::cr_error()
{
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

void ClientStream::ss_readyRead()
{
	QByteArray a;
	a = d->ss->read();

#ifdef LIBGW_DEBUG
	QByteArray cs(a.data(), a.size()+1);
	CoreProtocol::debug( QString( "ClientStream: ss_readyRead() recv: %1 bytes" ).arg( a.size() ) );
	cs_dump( a );
#endif

	d->client.addIncomingData(a);
/*	if(d->notify & CoreProtocol::NRecv) { */
	//processNext();
}

void ClientStream::ss_bytesWritten(int bytes)
{
#ifdef LIBGW_DEBUG
	CoreProtocol::debug( QString( "ClientStream::ss_bytesWritten: %1 bytes written" ).arg( bytes ) );
#else
	Q_UNUSED( bytes );
#endif
}

void ClientStream::ss_tlsHandshaken()
{
	QPointer<QObject> self = this;
	emit securityLayerActivated(LayerTLS);
	if(!self)
		return;
	processNext();
}

void ClientStream::ss_tlsClosed()
{
	CoreProtocol::debug( "ClientStream::ss_tlsClosed()" );
	reset();
	emit connectionClosed();
}

void ClientStream::ss_error(int x)
{
	CoreProtocol::debug( QString( "ClientStream::ss_error() x=%1 ").arg( x ) );
	if(x == SecureStream::ErrTLS) {
		reset();
		d->errCond = TLSFail;
		emit error(ErrTLS);
	}
	else {
		reset();
		emit error(ErrSecurityLayer);
	}
}

void ClientStream::srvProcessNext()
{
}

void ClientStream::doReadyRead()
{
	//QPointer<QObject> self = this;
	emit readyRead();
	//if(!self)
	//	return;
	//d->in_rrsig = false;
}

void ClientStream::processNext()
{
	if( !d->in.isEmpty() ) {
		//d->in_rrsig = true;
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

#include "gwclientstream.moc"
