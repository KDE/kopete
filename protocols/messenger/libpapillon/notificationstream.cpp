/*
   notificationstream.cpp - Represent a stream with the Notification server.

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
#include "notificationstream.h"

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

class NotificationStream::Private
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

NotificationStream::NotificationStream(Connector *connector, QObject *parent)
 : Stream(parent), d(new Private)
{
	d->connector = connector;
	connect(d->connector, SIGNAL(connected()), this, SLOT(slotConnectorConnected()));
	connect( &d->protocol, SIGNAL( outgoingData( const QByteArray& ) ), this, SLOT(slotProtocolOutgoingData(const QByteArray&)) );
	connect( &d->protocol, SIGNAL( incomingData() ), this, SLOT(slotProtocolIncomingData()) );
}


NotificationStream::~NotificationStream()
{
	delete d;
}


void NotificationStream::connectToServer(const QString &server, quint16 port)
{
	d->connector->connectToServer(server, port);
}

void NotificationStream::slotConnectorConnected()
{
	d->byteStream = d->connector->stream();
	connect(d->byteStream, SIGNAL(connectionClosed()), SLOT(slotByteStreamConnectionClosed()));
	//connect(d->byteStream, SIGNAL(delayedCloseFinished()), SLOT(bs_delayedCloseFinished()));
	connect(d->byteStream, SIGNAL(readyRead()), SLOT(slotByteStreamReadyRead()));
	connect(d->byteStream, SIGNAL(bytesWritten(int)), SLOT(slotByteStreamBytesWritten(int)));
	//connect(d->byteStream, SIGNAL(error(int)), SLOT(bs_error(int)));

	QByteArray spare = d->byteStream->read();

	QPointer<QObject> self = this;
	emit connected();

	if(!self)
		return;
}

void NotificationStream::reset(bool all)
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

void NotificationStream::slotProtocolIncomingData()
{
	Transfer * incoming = d->protocol.incomingTransfer();
	if( incoming )
	{
		d->transferQueue.enqueue( incoming );
		emit readyRead();
	}
}

void NotificationStream::slotProtocolOutgoingData(const QByteArray &data)
{
	d->byteStream->write(data);
}

void NotificationStream::slotByteStreamConnectionClosed()
{
	emit connectionClosed();
}

void NotificationStream::slotByteStreamReadyRead()
{
	QByteArray a;
	a = d->byteStream->read();

	d->protocol.addIncomingData(a);
}

void NotificationStream::slotByteStreamBytesWritten(int bytes)
{
	
}

void NotificationStream::close()
{
	d->connector->done();
}

int NotificationStream::errorCondition() const
{
	return 0;
}

QString NotificationStream::errorText() const
{
	return QString();
}


bool NotificationStream::transfersAvailable() const
{
	return !d->transferQueue.isEmpty();
}

Transfer *NotificationStream::read()
{
	if( d->transferQueue.isEmpty() )
		return 0;
	else
		return d->transferQueue.dequeue();
}

void NotificationStream::write(Transfer *transfer)
{
	qDebug() << "NotificationStream::write():" << "Sending:" << transfer->toString().replace("\r\n", "");

	d->protocol.outgoingTransfer(transfer);
}

}

#include "notificationstream.moc"
