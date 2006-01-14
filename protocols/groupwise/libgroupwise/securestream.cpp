/*
    securestream.h - Kopete Groupwise Protocol
    Combines a ByteStream with TLS and SASL
  
    Copyright (c) 2004      SUSE Linux AG	 	 http://www.suse.com
    
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

/*
  Note: SecureStream depends on the underlying security layers to signal
    plain-to-encrypted results immediately (as opposed to waiting for the
    event loop) so that the user cannot add/remove security layers during
    this conversion moment.  QCA::TLS and QCA::SASL behave as expected,
    but future layers might not.
*/

#include<qguardedptr.h>
#include<qvaluelist.h>
#include<qtimer.h>

#include"securestream.h"

//----------------------------------------------------------------------------
// LayerTracker
//----------------------------------------------------------------------------
LayerTracker::LayerTracker()
{
	p = 0;
}

void LayerTracker::reset()
{USE_TLSHANDLER
	p = 0;
	list.clear();
}

void LayerTracker::addPlain(int plain)
{
	p += plain;
}

void LayerTracker::specifyEncoded(int encoded, int plain)
{
	// can't specify more bytes than we have
	if(plain > p)
		plain = p;
	p -= plain;
	Item i;
	i.plain = plain;
	i.encoded = encoded;
	list += i;
}

int LayerTracker::finished(int encoded)
{
	int plain = 0;
	for(QValueList<Item>::Iterator it = list.begin(); it != list.end();) {
		Item &i = *it;

		// not enough?
		if(encoded < i.encoded) {
			i.encoded -= encoded;
			break;
		}

		encoded -= i.encoded;
		plain += i.plain;
		it = list.remove(it);
	}
	return plain;
}

//----------------------------------------------------------------------------
// SecureStream
//----------------------------------------------------------------------------

SecureLayer::SecureLayer(QCA::TLS *t)
{
	type = TLS;
	p.tls = t;
	init();
	connect(p.tls, SIGNAL(handshaken()), SLOT(tls_handshaken()));
	connect(p.tls, SIGNAL(readyRead()), SLOT(tls_readyRead()));
	connect(p.tls, SIGNAL(readyReadOutgoing(int)), SLOT(tls_readyReadOutgoing(int)));
	connect(p.tls, SIGNAL(closed()), SLOT(tls_closed()));
	connect(p.tls, SIGNAL(error(int)), SLOT(tls_error(int)));
}

SecureLayer::SecureLayer(QCA::SASL *s)
{
	type = SASL;
	p.sasl = s;
	init();
	connect(p.sasl, SIGNAL(readyRead()), SLOT(sasl_readyRead()));
	connect(p.sasl, SIGNAL(readyReadOutgoing(int)), SLOT(sasl_readyReadOutgoing(int)));
	connect(p.sasl, SIGNAL(error(int)), SLOT(sasl_error(int)));
}

#ifdef USE_TLSHANDLER
SecureLayer::SecureLayer(TLSHandler *t)
{
	type = TLSH;
	p.tlsHandler = t;
	init();
	connect(p.tlsHandler, SIGNAL(success()), SLOT(tlsHandler_success()));
	connect(p.tlsHandler, SIGNAL(fail()), SLOT(tlsHandler_fail()));
	connect(p.tlsHandler, SIGNAL(closed()), SLOT(tlsHandler_closed()));
	connect(p.tlsHandler, SIGNAL(readyRead(const QByteArray &)), SLOT(tlsHandler_readyRead(const QByteArray &)));
	connect(p.tlsHandler, SIGNAL(readyReadOutgoing(const QByteArray &, int)), SLOT(tlsHandler_readyReadOutgoing(const QByteArray &, int)));
}
#endif

void SecureLayer::init()
{
	tls_done = false;
	prebytes = 0;
}

void SecureLayer::write(const QByteArray &a)
{
	layer.addPlain(a.size());
	switch(type) {
		case TLS:  { p.tls->write(a); break; }
		case SASL: { p.sasl->write(a); break; }
#ifdef USE_TLSHANDLER
		case TLSH: { p.tlsHandler->write(a); break; }
#endif
	}
}

void SecureLayer::writeIncoming(const QByteArray &a)
{
	switch(type) {
		case TLS:  { p.tls->writeIncoming(a); break; }
		case SASL: { p.sasl->writeIncoming(a); break; }
#ifdef USE_TLSHANDLER
			case TLSH: { p.tlsHandler->writeIncoming(a); break; }
#endif
	}
}

int SecureLayer::finished(int plain)
{
	int written = 0;

	// deal with prebytes (bytes sent prior to this security layer)
	if(prebytes > 0) {
		if(prebytes >= plain) {
			written += plain;
			prebytes -= plain;
			plain = 0;
		}
		else {
			written += prebytes;
			plain -= prebytes;
			prebytes = 0;
		}
	}

	// put remainder into the layer tracker
	if(type == SASL || tls_done)
		written += layer.finished(plain);

	return written;
}

void SecureLayer::tls_handshaken()
{
	tls_done = true;
	tlsHandshaken();
}

void SecureLayer::tls_readyRead()
{
	QByteArray a = p.tls->read();
	readyRead(a);
}

void SecureLayer::tls_readyReadOutgoing(int plainBytes)
{
	QByteArray a = p.tls->readOutgoing();
	if(tls_done)
		layer.specifyEncoded(a.size(), plainBytes);
	needWrite(a);
}

void SecureLayer::tls_closed()
{
	QByteArray a = p.tls->readUnprocessed();
	tlsClosed(a);
}

void SecureLayer::tls_error(int x)
{
	error(x);
}

void SecureLayer::sasl_readyRead()
{
	QByteArray a = p.sasl->read();
	readyRead(a);
}

void SecureLayer::sasl_readyReadOutgoing(int plainBytes)
{
	QByteArray a = p.sasl->readOutgoing();
	layer.specifyEncoded(a.size(), plainBytes);
	needWrite(a);
}

void SecureLayer::sasl_error(int x)
{
	error(x);
}

#ifdef USE_TLSHANDLER
void SecureLayer::tlsHandler_success()
{
	tls_done = true;
	tlsHandshaken();
}

void SecureLayer::tlsHandler_fail()
{
	error(0);
}

void SecureLayer::tlsHandler_closed()
{
	tlsClosed(QByteArray());
}

void SecureLayer::tlsHandler_readyRead(const QByteArray &a)
{
	readyRead(a);
}

void SecureLayer::tlsHandler_readyReadOutgoing(const QByteArray &a, int plainBytes)
{
	if(tls_done)
		layer.specifyEncoded(a.size(), plainBytes);
	needWrite(a);
}
#endif

class SecureStream::Private
{
public:
	ByteStream *bs;
	QPtrList<SecureLayer> layers;
	int pending;
	int errorCode;
	bool active;
	bool topInProgress;

	bool haveTLS() const
	{
		QPtrListIterator<SecureLayer> it(layers);
		for(SecureLayer *s; (s = it.current()); ++it) {
			if(s->type == SecureLayer::TLS
#ifdef USE_TLSHANDLER
			|| s->type == SecureLayer::TLSH
#endif
			) {
				return true;
			}
		}
		return false;
	}

	bool haveSASL() const
	{
		QPtrListIterator<SecureLayer> it(layers);
		for(SecureLayer *s; (s = it.current()); ++it) {
			if(s->type == SecureLayer::SASL)
				return true;
		}
		return false;
	}
};

SecureStream::SecureStream(ByteStream *s)
:ByteStream(0)
{
	d = new Private;

	d->bs = s;
	connect(d->bs, SIGNAL(readyRead()), SLOT(bs_readyRead()));
	connect(d->bs, SIGNAL(bytesWritten(int)), SLOT(bs_bytesWritten(int)));

	d->layers.setAutoDelete(true);
	d->pending = 0;
	d->active = true;
	d->topInProgress = false;
}

SecureStream::~SecureStream()
{
	delete d;
}

void SecureStream::linkLayer(QObject *s)
{
	connect(s, SIGNAL(tlsHandshaken()), SLOT(layer_tlsHandshaken()));
	connect(s, SIGNAL(tlsClosed(const QByteArray &)), SLOT(layer_tlsClosed(const QByteArray &)));
	connect(s, SIGNAL(readyRead(const QByteArray &)), SLOT(layer_readyRead(const QByteArray &)));
	connect(s, SIGNAL(needWrite(const QByteArray &)), SLOT(layer_needWrite(const QByteArray &)));
	connect(s, SIGNAL(error(int)), SLOT(layer_error(int)));
}

int SecureStream::calcPrebytes() const
{
	int x = 0;
	QPtrListIterator<SecureLayer> it(d->layers);
	for(SecureLayer *s; (s = it.current()); ++it)
		x += s->prebytes;
	return (d->pending - x);
}

void SecureStream::startTLSClient(QCA::TLS *t, const QByteArray &spare)
{
	if(!d->active || d->topInProgress || d->haveTLS())
		return;

	SecureLayer *s = new SecureLayer(t);
	s->prebytes = calcPrebytes();
	linkLayer(s);
	d->layers.append(s);
	d->topInProgress = true;

	insertData(spare);
}

void SecureStream::startTLSServer(QCA::TLS *t, const QByteArray &spare)
{
	if(!d->active || d->topInProgress || d->haveTLS())
		return;

	SecureLayer *s = new SecureLayer(t);
	s->prebytes = calcPrebytes();
	linkLayer(s);
	d->layers.append(s);
	d->topInProgress = true;

	insertData(spare);
}

void SecureStream::setLayerSASL(QCA::SASL *sasl, const QByteArray &spare)
{
	if(!d->active || d->topInProgress || d->haveSASL())
		return;

	SecureLayer *s = new SecureLayer(sasl);
	s->prebytes = calcPrebytes();
	linkLayer(s);
	d->layers.append(s);

	insertData(spare);
}

#ifdef USE_TLSHANDLER
void SecureStream::startTLSClient(TLSHandler *t, const QString &server, const QByteArray &spare)
{
	if(!d->active || d->topInProgress || d->haveTLS())
		return;

	SecureLayer *s = new SecureLayer(t);
	s->prebytes = calcPrebytes();
	linkLayer(s);
	d->layers.append(s);
	d->topInProgress = true;

	// unlike QCA::TLS, TLSHandler has no return value
	s->p.tlsHandler->startClient(server);

	insertData(spare);
}
#endif

void SecureStream::closeTLS()
{
	SecureLayer *s = d->layers.getLast();
	if(s) {
		if(s->type == SecureLayer::TLS)
			s->p.tls->close();
	}
}

int SecureStream::errorCode() const
{
	return d->errorCode;
}

bool SecureStream::isOpen() const
{
	return d->active;
}

void SecureStream::write(const QByteArray &a)
{
	if(!isOpen())
		return;

	d->pending += a.size();

	// send to the last layer
	SecureLayer *s = d->layers.getLast();
	if(s)
		s->write(a);
	else
		writeRawData(a);
}

int SecureStream::bytesToWrite() const
{
	return d->pending;
}

void SecureStream::bs_readyRead()
{
	QByteArray a = d->bs->read();

	// send to the first layer
	SecureLayer *s = d->layers.getFirst();
	if(s)
		s->writeIncoming(a);
	else
		incomingData(a);
}

void SecureStream::bs_bytesWritten(int bytes)
{
	QPtrListIterator<SecureLayer> it(d->layers);
	for(SecureLayer *s; (s = it.current()); ++it)
		bytes = s->finished(bytes);

	if(bytes > 0) {
		d->pending -= bytes;
		bytesWritten(bytes);
	}
}

void SecureStream::layer_tlsHandshaken()
{
	d->topInProgress = false;
	tlsHandshaken();
}

void SecureStream::layer_tlsClosed(const QByteArray &)
{
	d->active = false;
	d->layers.clear();
	tlsClosed();
}

void SecureStream::layer_readyRead(const QByteArray &a)
{
	SecureLayer *s = (SecureLayer *)sender();
	QPtrListIterator<SecureLayer> it(d->layers);
	while(it.current() != s)
		++it;

	// pass upwards
	++it;
	s = it.current();
	if(s)
		s->writeIncoming(a);
	else
		incomingData(a);
}

void SecureStream::layer_needWrite(const QByteArray &a)
{
	SecureLayer *s = (SecureLayer *)sender();
	QPtrListIterator<SecureLayer> it(d->layers);
	while(it.current() != s)
		++it;

	// pass downwards
	--it;
	s = it.current();
	if(s)
		s->write(a);
	else
		writeRawData(a);
}

void SecureStream::layer_error(int x)
{
	SecureLayer *s = (SecureLayer *)sender();
	int type = s->type;
	d->errorCode = x;
	d->active = false;
	d->layers.clear();
	if(type == SecureLayer::TLS)
		error(ErrTLS);
	else if(type == SecureLayer::SASL)
		error(ErrSASL);
#ifdef USE_TLSHANDLER
	else if(type == SecureLayer::TLSH)
		error(ErrTLS);
#endif
}

void SecureStream::insertData(const QByteArray &a)
{
	if(!a.isEmpty()) {
		SecureLayer *s = d->layers.getLast();
		if(s)
			s->writeIncoming(a);
		else
			incomingData(a);
	}
}

void SecureStream::writeRawData(const QByteArray &a)
{
	d->bs->write(a);
}

void SecureStream::incomingData(const QByteArray &a)
{
	appendRead(a);
	//qDebug( "SecureStream::incomingData() got %i bytes ", a.size() );

	if(bytesAvailable())
		readyRead();
}

#include "securestream.moc"
