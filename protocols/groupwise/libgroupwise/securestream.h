/*
    securestream.h - Kopete Groupwise Protocol
    Combines a ByteStream with TLS and SASL
  
    Copyright (c) 2006,2007 Novell, Inc	 	 	 http://www.opensuse.org
    Copyright (c) 2004      SUSE Linux AG	 	 http://www.suse.com
    
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

#ifndef SECURESTREAM_H
#define SECURESTREAM_H

#include <QtCrypto>
#include "tlshandler.h"
#include"bytestream.h"

#define USE_TLSHANDLER

#ifdef USE_TLSHANDLER
	class TLSHandler;
#endif
class CompressionHandler;

class SecureStream : public ByteStream
{
	Q_OBJECT
public:
	enum Error { ErrTLS = ErrCustom, ErrSASL };
	SecureStream(ByteStream *s);
	~SecureStream();

	void startTLSClient(QCA::TLS *t, const QByteArray &spare=QByteArray());
	void startTLSServer(QCA::TLS *t, const QByteArray &spare=QByteArray());
	void setLayerCompress(const QByteArray &spare=QByteArray());
	void setLayerSASL(QCA::SASL *s, const QByteArray &spare=QByteArray());
#ifdef USE_TLSHANDLER
	void startTLSClient(TLSHandler *t, const QString &server, const QByteArray &spare=QByteArray());
#endif

	void closeTLS();
	int errorCode() const;

	// reimplemented
	bool isOpen() const;
	void write(const QByteArray &);
	int bytesToWrite() const;

signals:
	void tlsHandshaken();
	void tlsClosed();

private slots:
	void bs_readyRead();
	void bs_bytesWritten(int);

	void layer_tlsHandshaken();
	void layer_tlsClosed(const QByteArray &);
	void layer_readyRead(const QByteArray &);
	void layer_needWrite(const QByteArray &);
	void layer_error(int);

private:
	void linkLayer(QObject *);
	int calcPrebytes() const;
	void insertData(const QByteArray &a);
	void writeRawData(const QByteArray &a);
	void incomingData(const QByteArray &a);

	class Private;
	Private * const d;
};

class LayerTracker
{
public:
	struct Item
	{
		int plain;
		int encoded;
	};
USE_TLSHANDLER
	LayerTracker();

	void reset();
	void addPlain(int plain);
	void specifyEncoded(int encoded, int plain);
	int finished(int encoded);

	int p;
	QList<Item> list;
};


class SecureLayer : public QObject
{
	Q_OBJECT
public:
	SecureLayer(QCA::TLS *t);
	SecureLayer(QCA::SASL *s);
#ifdef USE_TLSHANDLER
	SecureLayer(TLSHandler *t);
#endif
	SecureLayer(CompressionHandler *t);
	void init();
	void write(const QByteArray &a);
	void writeIncoming(const QByteArray &a);
	int finished(int plain);

	enum { TLS, SASL, TLSH, Compression };
	int type;
	union {
		QCA::TLS *tls;
		QCA::SASL *sasl;
#ifdef USE_TLSHANDLER
		TLSHandler *tlsHandler;
#endif
		CompressionHandler *compressionHandler;
	} p;
	LayerTracker layer;
	bool tls_done;
	int prebytes;

signals:
        void tlsHandshaken();
        void tlsClosed(const QByteArray &);
        void readyRead(const QByteArray &);
        void needWrite(const QByteArray &);
        void error(int);

private slots:
        void tls_handshaken();
        void tls_readyRead();
        void tls_readyReadOutgoing(int plainBytes);
        void tls_closed();
        void tls_error(int x);
        void sasl_readyRead();
        void sasl_readyReadOutgoing();
        void sasl_error();
        void compressionHandler_readyRead();
        void compressionHandler_readyReadOutgoing();
        void compressionHandler_error();
#ifdef USE_TLSHANDLER
	void tlsHandler_success();
	void tlsHandler_fail();
	void tlsHandler_closed();
	void tlsHandler_readyRead(const QByteArray &a);
	void tlsHandler_readyReadOutgoing(const QByteArray &a, int plainBytes);
#endif
	
};

#endif
