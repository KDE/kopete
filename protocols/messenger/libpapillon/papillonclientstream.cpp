/*
   papillonclientstream.cpp - Represent a stream with the Notification server.

   Copyright (c) 2006 by Michaël Larouche <michael.larouche@kdemail.net>

   *************************************************************************
   *                                                                       *
   * This library is free software; you can redistribute it and/or         *
   * modify it under the terms of the GNU Lesser General Public            *
   * License as published by the Free Software Foundation; either          *
   * version 2 of the License, or (at your option) any later version.      *
   *                                                                       *
   *************************************************************************
*/
#include "papillonclientstream.h"

// Qt includes
#include <QQueue>
#include <QPointer>
#include <QtDebug>

// Papillon includes.
#include "transfer.h"
#include "coreprotocol.h"
#include "connector.h"
#include "bytestream.h"

namespace Papillon 
{

class ClientStream::Private
{
public:
	Private()
	 : connector(0)
	{}

	Connector *connector;
	ByteStream *byteStream;
	CoreProtocol protocol;

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
	QByteArray a;
	a = d->byteStream->read();

	d->protocol.addIncomingData(a);
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
