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

#include <qobject.h>
#include <kbufferedsocket.h>
#include <kdebug.h>
#include <kresolver.h>

#include "yahoobytestream.h"

KNetworkByteStream::KNetworkByteStream( QObject *parent, const char */*name*/ )
 : ByteStream ( parent )
{
	kdDebug( 14181 ) << k_funcinfo << "Instantiating new KNetwork byte stream." << endl;

	// reset close tracking flag
	mClosing = false;

	mSocket = new KNetwork::KBufferedSocket;

	// make sure we get a signal whenever there's data to be read
	mSocket->enableRead( true );

	// connect signals and slots
	QObject::connect( mSocket, SIGNAL ( gotError ( int ) ), this, SLOT ( slotError ( int ) ) );
	QObject::connect( mSocket, SIGNAL ( connected ( const KResolverEntry& ) ), this, SLOT ( slotConnected () ) );
	QObject::connect( mSocket, SIGNAL ( closed () ), this, SLOT ( slotConnectionClosed () ) );
	QObject::connect( mSocket, SIGNAL ( readyRead () ), this, SLOT ( slotReadyRead () ) );
	QObject::connect( mSocket, SIGNAL ( bytesWritten ( int ) ), this, SLOT ( slotBytesWritten ( int ) ) );
}

bool KNetworkByteStream::connect( QString host, QString service )
{
	kdDebug( 14181 ) << k_funcinfo << "Connecting to " << host << ", service " << service << endl;

	return socket()->connect( host, service );
}

bool KNetworkByteStream::isOpen() const
{
	// determine if socket is open
	return socket()->isOpen();
}

void KNetworkByteStream::close ()
{
	kdDebug ( 14181 ) << k_funcinfo << "Closing stream." << endl;

	// close the socket and set flag that we are closing it ourselves
	mClosing = true;
	socket()->close();
}

int KNetworkByteStream::tryWrite ()
{
	// send all data from the buffers to the socket
	QByteArray writeData = takeWrite();
	kdDebug( 14181 ) << k_funcinfo << "[writeData.size() = " << writeData.size() << "]" <<   endl;
	
	socket()->writeBlock( writeData.data(), writeData.size () );

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
	kdDebug( 14181 ) << k_funcinfo << "Socket has been closed." << endl;

	// depending on who closed the socket, emit different signals
	if ( mClosing )
	{
		kdDebug( 14181 ) << "..by ourselves!" << endl;
		kdDebug( 14181 ) << "socket error is " << socket()->errorString( socket()->error() ) << endl;
		emit connectionClosed ();
	}
	else
	{
		kdDebug( 14181 ) << "..by the other end" << endl;
		emit delayedCloseFinished ();
	}
}

void KNetworkByteStream::slotReadyRead()
{
	kdDebug( 14181 ) <<  endl;
	// stuff all available data into our buffers
	QByteArray readBuffer( socket()->bytesAvailable () );

	socket()->readBlock( readBuffer.data (), readBuffer.size () );

	appendRead( readBuffer );

	emit readyRead();
}

void KNetworkByteStream::slotBytesWritten( int bytes )
{
	kdDebug( 14181 ) << "[int bytes]: " << bytes << endl;
	emit bytesWritten(bytes);
}

void KNetworkByteStream::slotError( int code )
{
	kdDebug( 14181 ) << k_funcinfo << "Socket error " << code << endl;

	emit error( code );
}

#include "yahoobytestream.moc"

// kate: indent-width 4; replace-tabs off; tab-width 4; space-indent off;
