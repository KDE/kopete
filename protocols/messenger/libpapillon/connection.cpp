/*
   connection.cpp - Represent a transfer between the Messenger server.

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
#include "connection.h"

// Qt includes
#include <QtDebug>

// Papillon includes
#include "task.h"
#include "papillonclientstream.h"
#include "transfer.h"

namespace Papillon 
{

class Connection::Private
{
public:
	Private()
	 : rootTask(0), stream(0), transactionId(0),
	   isConnected(false)
	{}

	~Private()
	{
		delete rootTask;
	}

	Task *rootTask;
	ClientStream *stream;

	int transactionId;
	bool isConnected;
	QString server;
	int port;
};

Connection::Connection(ClientStream *stream)
 : QObject(), d(new Private)
{
	d->rootTask = new Task(this, true);
	d->stream = stream;

	connect(d->stream, SIGNAL(readyRead()), this, SLOT(transferReceived()));
	connect(d->stream, SIGNAL(connected()), this, SLOT(slotConnected()));
	connect(d->stream, SIGNAL(connectionClosed()), this, SLOT(slotDisconnected()));
}

Connection::~Connection()
{
	delete d;
}

Task *Connection::rootTask()
{
	return d->rootTask;
}

int Connection::transactionId()
{
	return ++d->transactionId;
}

bool Connection::isConnected()
{
	return d->isConnected;
}

void Connection::connectToServer(const QString &server, quint16 port)
{
	d->server = server;
	d->port = port;
	
	d->stream->connectToServer(d->server, d->port);
}

void Connection::send(Transfer *transfer)
{
	d->stream->write(transfer);
}

void Connection::transferReceived()
{
	Transfer *readTransfer = d->stream->read();
	if(readTransfer)
	{
		dispatchTransfer( d->stream->read() );
	}
	else
	{
		qDebug() << PAPILLON_FUNCINFO << "Got a null Transfer, investigate.";
	}
}

void Connection::dispatchTransfer(Transfer *transfer)
{
	if( !d->rootTask->take(transfer) )
	{
		qDebug() << PAPILLON_FUNCINFO << "Root task refused the transfer." << "Transfer was:" << transfer->toString();
	}
	
	delete transfer;
}

void Connection::slotConnected()
{
	d->isConnected = true;
}

void Connection::slotDisconnected()
{
	d->isConnected = false;
}

}

#include "connection.moc"
