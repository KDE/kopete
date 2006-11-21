/*
   securestream.cpp - Secure stream based on TLS/SSL.

   Copyright (c) 2006 by Michaël Larouche <larouche@kde.org>

   *************************************************************************
   *                                                                       *
   * This library is free software; you can redistribute it and/or         *
   * modify it under the terms of the GNU Lesser General Public            *
   * License as published by the Free Software Foundation; either          *
   * version 2 of the License, or (at your option) any later version.      *
   *                                                                       *
   *************************************************************************
*/
#include "Papillon/Http/SecureStream"

// Qt includes
#include <QtDebug>
#include <QtCore/QLatin1String>

// QCA include
#include <QtCrypto>

// Papillon includes
#include "Papillon/Base/Connector"
#include "Papillon/Base/ByteStream"

namespace Papillon
{

class SecureStream::Private
{
public:
	Private()
	 : connector(0), byteStream(0), tlsHandler(0)
	{}

	~Private()
	{
		delete tlsHandler;
	}

	Connector *connector;
	ByteStream *byteStream;
	QCA::TLS *tlsHandler;

	QString server;
	QByteArray readBuffer;
	
	SecureStream::ErrorCode errorCode;
	QString errorString;
};

SecureStream::SecureStream(Connector *connector)
 : QObject(0), d(new Private)
{
	d->connector = connector;
	
	Q_ASSERT( QCA::isSupported("tls") );
	d->tlsHandler = new QCA::TLS;

	connect(d->connector, SIGNAL(connected()), this, SLOT(streamConnected()));

	connect(d->tlsHandler, SIGNAL(handshaken()), this, SLOT(tlsHandshaken()));
	connect(d->tlsHandler, SIGNAL(readyRead()), this, SLOT(tlsReadyRead()));
	connect(d->tlsHandler, SIGNAL(readyReadOutgoing()), this, SLOT(tlsReadyWrite()));
	connect(d->tlsHandler, SIGNAL(closed()), this, SLOT(slotDisconnected()));
	connect(d->tlsHandler, SIGNAL(error()), this, SLOT(tlsError()));
}

SecureStream::~SecureStream()
{
	delete d;
}

void SecureStream::connectToServer(const QString &server)
{
	d->server = server;
	d->connector->connectToServer(server, 443);
}

void SecureStream::disconnectFromServer()
{
	disconnect(d->byteStream, SIGNAL(readyRead()), this, SLOT(streamReadyRead()));
	disconnect(d->byteStream, SIGNAL(connectionClosed()), this, SLOT(slotDisconnected()));
	d->connector->done();
}

SecureStream::ErrorCode SecureStream::errorCode() const
{
	return d->errorCode;
}

QString SecureStream::errorString() const
{
	return d->errorString;
}

QByteArray SecureStream::read()
{
	QByteArray copy = d->readBuffer;
	d->readBuffer.clear();

	return copy;
}

void SecureStream::write(const QByteArray &data)
{
	d->tlsHandler->write(data);
}

void SecureStream::streamConnected()
{
	// Setting up the bytestream here, after being connected.
	d->byteStream = d->connector->stream();
	connect(d->byteStream, SIGNAL(readyRead()), this, SLOT(streamReadyRead()));
	connect(d->byteStream, SIGNAL(connectionClosed()), this, SLOT(slotDisconnected()));

	qDebug() << PAPILLON_FUNCINFO << "Stream is connected. Begin TLS handshake.";

	if( QCA::haveSystemStore() )
	{
		d->tlsHandler->setTrustedCertificates( QCA::systemStore() );
	}
	else
	{
		qDebug() << PAPILLON_FUNCINFO << "No root certification found for TLS/SSL.";
	}

	d->tlsHandler->startClient( d->server );
}

void SecureStream::streamReadyRead()
{
	d->tlsHandler->writeIncoming( d->byteStream->read() );
}

void SecureStream::tlsHandshaken()
{
	QCA::TLS::IdentityResult result = d->tlsHandler->peerIdentityResult();
	
	qDebug() << PAPILLON_FUNCINFO << QString("Successful TLS/SSL handshake using %1 (%2 of %3 bits)").arg(d->tlsHandler->cipherSuite()).arg(d->tlsHandler->cipherBits()).arg(d->tlsHandler->cipherMaxBits());
 
	QString errorString;
	SecureStream::ErrorCode errorCode;
	switch(result)
	{
		case QCA::TLS::Valid:
			errorString = QLatin1String("Certificate Valid.");
			errorCode = SecureStream::ErrorValidCertificate;
			break;
		case QCA::TLS::HostMismatch:
			errorString = QLatin1String("Error: Wrong Certificate");
			errorCode = SecureStream::ErrorWrongCertificate;
			break;
		case QCA::TLS::InvalidCertificate:
			errorString = QLatin1String("Error: Invalid certificate.");
			errorCode = SecureStream::ErrorInvalidCertificate;
			break;
		default:
			errorString = QLatin1String("Error: No certificate");
			errorCode = SecureStream::ErrorNoCertificate;
			break;
	}
	qDebug() << PAPILLON_FUNCINFO << errorString;

	d->errorString = errorString;
	d->errorCode = errorCode;

	emit connected();
}

void SecureStream::tlsReadyRead()
{
	d->readBuffer = d->tlsHandler->read();

	emit readyRead();
}

void SecureStream::tlsReadyWrite()
{
	d->byteStream->write( d->tlsHandler->readOutgoing() );
}

void SecureStream::tlsError()
{
	QCA::TLS::Error errorCode = d->tlsHandler->errorCode();
	if( errorCode == QCA::TLS::ErrorHandshake )
	{
		d->errorCode = SecureStream::ErrorHandshakeFailed;
		d->errorString = QLatin1String("TLS Handshake failed.");
	}
	else
	{
		d->errorCode = SecureStream::ErrorUnknown;
		d->errorString = QLatin1String("Unknown TLS error.");
	}

	emit error();
}

void SecureStream::slotDisconnected()
{
	d->errorCode = SecureStream::ErrorDisconnected;
	d->errorString = QLatin1String("Disconnected");

	emit disconnected();
}

}

#include "securestream.moc"
