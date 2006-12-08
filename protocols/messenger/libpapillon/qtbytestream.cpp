/*
   qtbytestream.cpp - ByteStream using QtNetwork's QTcpSocket.

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
#include "Papillon/QtByteStream"

// Qt includes
#include <QtNetwork/QTcpSocket>

namespace Papillon {

class QtByteStream::Private
{
public:
	QTcpSocket *socket;
	bool closing;
};

QtByteStream::QtByteStream(QObject *parent)
 : ByteStream(parent), d(new Private)
{
	d->closing = false;
	d->socket = new QTcpSocket(this);

	QObject::connect(d->socket, SIGNAL(connected()), this, SLOT(slotConnected()));
	QObject::connect(d->socket, SIGNAL(disconnected()), this, SLOT(slotConnectionClosed()));
	QObject::connect(d->socket, SIGNAL(readyRead()), this, SLOT(slotReadyRead()));
	QObject::connect(d->socket, SIGNAL(bytesWritten(qint64)), this, SLOT(slotBytesWritten(qint64)));
}


QtByteStream::~QtByteStream()
{
	delete d;
}

bool QtByteStream::connect(const QString &host, quint16 port)
{
	d->socket->connectToHost(host, port);
        return true;
}

bool QtByteStream::isOpen() const
{
	return d->socket->isValid();
}

void QtByteStream::close()
{
	d->closing = true;
	d->socket->close();
}

int QtByteStream::tryWrite()
{
	QByteArray writeData = takeWrite();
	
	d->socket->write( writeData );

	return writeData.size();
}

void QtByteStream::slotConnected()
{
	emit connected();
}

void QtByteStream::slotConnectionClosed()
{
	if( d->closing )
	{
		emit connectionClosed ();
	}
	else
	{
		emit delayedCloseFinished ();
	}
}

void QtByteStream::slotReadyRead()
{
	// stuff all available data into our buffers
	QByteArray readBuffer = d->socket->read( d->socket->bytesAvailable () );

	appendRead( readBuffer );

	emit readyRead();

}

void QtByteStream::slotBytesWritten(qint64 bytes)
{
	emit bytesWritten(bytes);
}

void QtByteStream::slotError(int)
{
	//emit error( error );	
}

}

#include "qtbytestream.moc"
