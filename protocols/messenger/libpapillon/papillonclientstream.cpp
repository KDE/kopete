/*
   papillonclientstream.cpp - Represent a stream with a Messenger server.

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
#include "Papillon/ClientStream"

// Qt includes
#include <QtCore/QQueue>
#include <QtCore/QPointer>
#include <QtDebug>

// Papillon includes.
#include "Papillon/Transfer"
#include "Papillon/MessengerCoreProtocol"
#include "Papillon/Base/Connector"
#include "Papillon/Base/ByteStream"

namespace Papillon 
{

class ClientStream::Private
{
public:
	Private()
	 : connector(0)
	{}

	~Private()
	{}

	Connector *connector;
	ByteStream *byteStream;
	MessengerCoreProtocol protocol;

	QQueue<Transfer*> transferQueue;
};

ClientStream::ClientStream(Connector *connector, QObject *parent)
 : Stream(parent), d(new Private)
{
	d->connector = connector;
	connect(d->connector, SIGNAL(connected()), this, SLOT(slotConnectorConnected()));
	connect( &d->protocol, SIGNAL( outgoingData( const QByteArray& ) ), this, SLOT(slotProtocolOutgoingData(const QByteArray&)) );
	connect( &d->protocol, SIGNAL( incomingData() ), this, SLOT(slotProtocolIncomingData()) );
}


ClientStream::~ClientStream()
{
	delete d;
}

void ClientStream::connectToServer(const QString &server, quint16 port)
{
	d->connector->connectToServer(server, port);
}

void ClientStream::slotConnectorConnected()
{
	d->byteStream = d->connector->stream();
	connect(d->byteStream, SIGNAL(connectionClosed()), SLOT(slotByteStreamConnectionClosed()));
	connect(d->byteStream, SIGNAL(readyRead()), SLOT(slotByteStreamReadyRead()));
	connect(d->byteStream, SIGNAL(bytesWritten(int)), SLOT(slotByteStreamBytesWritten(int)));

	QByteArray spare = d->byteStream->read();

	QPointer<QObject> self = this;
	emit connected();

	if(!self)
		return;
}

void ClientStream::reset(bool all)
{
	// reset connector
	if(d->byteStream) 
	{
		d->byteStream->close();
		d->byteStream = 0;
	}
	d->connector->done();

	// reset state machine
	d->protocol.reset();

	if(all)
		d->transferQueue.clear();
}

void ClientStream::slotProtocolIncomingData()
{
	Transfer * incoming = d->protocol.incomingTransfer();
	if( incoming )
	{
		d->transferQueue.enqueue( incoming );
		emit readyRead();
	}
}

void ClientStream::slotProtocolOutgoingData(const QByteArray &data)
{
	d->byteStream->write(data);
}

void ClientStream::slotByteStreamConnectionClosed()
{
	emit connectionClosed();
}

void ClientStream::slotByteStreamReadyRead()
{
	d->protocol.addIncomingData( d->byteStream->read() );
}

void ClientStream::slotByteStreamBytesWritten(int bytes)
{
	
}

void ClientStream::close()
{
	d->connector->done();
}

int ClientStream::errorCondition() const
{
	return 0;
}

QString ClientStream::errorText() const
{
	return QString();
}


bool ClientStream::transfersAvailable() const
{
	return !d->transferQueue.isEmpty();
}

Transfer *ClientStream::read()
{
	if( d->transferQueue.isEmpty() )
		return 0;
	else
		return d->transferQueue.dequeue();
}

void ClientStream::write(Transfer *transfer)
{
	qDebug() << PAPILLON_FUNCINFO << "Sending:" << transfer->toString().replace("\r\n", "");

	d->protocol.outgoingTransfer(transfer);
}

}

#include "papillonclientstream.moc"
