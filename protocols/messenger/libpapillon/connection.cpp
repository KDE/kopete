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

	Task *rootTask;
	ClientStream *stream;

	int transactionId;
	bool isConnected;
};

Connection::Connection(ClientStream *stream)
 : QObject(), d(new Private)
{
	d->rootTask = new Task(this, true);
	d->stream = stream;

	connect(stream, SIGNAL(readyRead()), this, SLOT(transferReceived()));
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

void Connection::send(Transfer *transfer)
{
	d->stream->write(transfer);
}

void Connection::transferReceived()
{
	dispatchTransfer( d->stream->read() );
}

void Connection::dispatchTransfer(Transfer *transfer)
{
	if( !d->rootTask->take(transfer) )
	{
		// TODO: Emit this error "Root task refused the transfer." and do a Transfer dump here.
	}
	
	delete transfer;
}

}

#include "connection.moc"
