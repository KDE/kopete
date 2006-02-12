/*
   qtconnector.cpp - Connector using QtNetwork.

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
#include "qtconnector.h"

#include "qtbytestream.h"

namespace Papillon {

class QtConnector::Private
{
public:
	QtByteStream *byteStream;
	QString server;
	quint16 port;
};

QtConnector::QtConnector(QObject *parent)
 : Connector(), d(new Private)
{
	d->byteStream = new QtByteStream(this);
	init();
}

QtConnector::~QtConnector()
{
	delete d;
}

ByteStream* QtConnector::stream() const
{
	return d->byteStream;
}

void QtConnector::connectToServer(const QString& server, quint16 port)
{
	d->server = server;
	d->port = port;

	if ( !d->byteStream->connect(server, port)  )
	{
		emit error();
	}
}

void QtConnector::init()
{
	connect( d->byteStream, SIGNAL(connected()), this, SIGNAL(connected()) );
}

void QtConnector::done()
{
	d->byteStream->close();
}

void QtConnector::slotConnected()
{
	emit connected();
}


}
