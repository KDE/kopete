/*
   connection.cpp - Connection with a Messenger service.

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
#include "Papillon/Connection"

// Qt includes
#include <QtDebug>

// Papillon includes
#include "Papillon/Task"
#include "Papillon/ClientStream"
#include "Papillon/Transfer"
#include "Papillon/Client"

namespace Papillon 
{

class Connection::Private
{
public:
	Private()
	 : rootTask(0), stream(0), client(0), transactionId(0),
	   isConnected(false)
	{}

	~Private()
	{
		delete rootTask;
		delete stream;
	}

	Task *rootTask;
	ClientStream *stream;
	Client *client;

	int transactionId;
	bool isConnected;
	QString server;
	int port;
};

Connection::Connection(ClientStream *stream, QObject *parent)
 : QObject(parent), d(new Private)
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

QObject *Connection::parent()
{
	return d->parent;
}

void Connection::setParent(QObject *parent)
{
	d->parent = parent;
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

void Connection::disconnectFromServer()
{
	d->stream->close();
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
		qDebug() << PAPILLON_FUNCINFO << "Dispatch received transfer to tasks.";
		dispatchTransfer( readTransfer );
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
	qDebug() << PAPILLON_FUNCINFO << "We are connected to" << d->server;

	d->isConnected = true;
	emit connected();
}

void Connection::slotDisconnected()
{
	qDebug() << PAPILLON_FUNCINFO << "We got disconnected from" << d->server;

	d->isConnected = false;
	emit disconnected();
}

}

#include "connection.moc"
