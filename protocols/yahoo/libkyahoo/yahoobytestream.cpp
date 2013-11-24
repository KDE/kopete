/*
    YMSG - Yahoo Protocol Knetwork Bytestream

    Copyright (C) 2004 by Till Gerken <till@tantalo.net>

    Kopete (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU General Public                   *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "yahoobytestream.h"

#include <qobject.h>
#include <k3bufferedsocket.h>
#include <kdebug.h>
#include <k3resolver.h>

KNetworkByteStream::KNetworkByteStream( QObject *parent )
 : ByteStream ( parent )
{
	kDebug( 14181 ) << "Instantiating new KNetwork byte stream.";

	// reset close tracking flag
	mClosing = false;

	mSocket = new KNetwork::KBufferedSocket;

	// make sure we get a signal whenever there's data to be read
	mSocket->enableRead( true );

	// connect signals and slots
	QObject::connect( mSocket, SIGNAL (gotError(int)), this, SLOT (slotError(int)) );
	QObject::connect( mSocket, SIGNAL (connected(KNetwork::KResolverEntry)), this, SLOT (slotConnected()) );
	QObject::connect( mSocket, SIGNAL (closed()), this, SLOT (slotConnectionClosed()) );
	QObject::connect( mSocket, SIGNAL (readyRead()), this, SLOT (slotReadyRead()) );
	QObject::connect( mSocket, SIGNAL (bytesWritten(qint64)), this, SLOT (slotBytesWritten(qint64)) );
}

bool KNetworkByteStream::connect( QString host, QString service )
{
	kDebug( 14181 ) << "Connecting to " << host << ", service " << service;

	return socket()->connect( host, service );
}

bool KNetworkByteStream::isOpen() const
{
	// determine if socket is open
	return socket()->isOpen();
}

void KNetworkByteStream::close ()
{
	kDebug ( 14181 ) << "Closing stream.";

	// close the socket and set flag that we are closing it ourselves
	mClosing = true;
	socket()->close();
}

int KNetworkByteStream::tryWrite ()
{
	// send all data from the buffers to the socket
	QByteArray writeData = takeWrite();
	kDebug( 14181 ) << "[writeData.size() = " << writeData.size() << "]";
	
	socket()->write( writeData.data(), writeData.size () );

	return writeData.size();
}

KNetwork::KBufferedSocket *KNetworkByteStream::socket() const
{
	return mSocket;
}

KNetworkByteStream::~KNetworkByteStream()
{
	delete mSocket;
}

void KNetworkByteStream::slotConnected()
{
	emit connected();
}

void KNetworkByteStream::slotConnectionClosed()
{
	kDebug( 14181 ) << "Socket has been closed.";

	// depending on who closed the socket, emit different signals
	if ( mClosing )
	{
		kDebug( 14181 ) << "..by ourselves!";
		kDebug( 14181 ) << "socket error is " << socket()->errorString();
		emit connectionClosed ();
	}
	else
	{
		kDebug( 14181 ) << "..by the other end";
		emit delayedCloseFinished ();
	}
}

void KNetworkByteStream::slotReadyRead()
{
	kDebug( 14181 );
	// stuff all available data into our buffers
	QByteArray readBuffer;
	readBuffer.resize( socket()->bytesAvailable () );

	socket()->read( readBuffer.data (), readBuffer.size () );

	appendRead( readBuffer );

	emit readyRead();
}

void KNetworkByteStream::slotBytesWritten( qint64 bytes )
{
	kDebug( 14181 ) << "[int bytes]: " << bytes;
	emit bytesWritten(bytes);
}

void KNetworkByteStream::slotError( int code )
{
	kDebug( 14181 ) << "Socket error " << code;

	emit error( code );
}

#include "yahoobytestream.moc"

// kate: indent-width 4; replace-tabs off; tab-width 4; space-indent off;
