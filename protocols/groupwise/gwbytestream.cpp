
/***************************************************************************
                   gwbytestream.cpp  -  Byte Stream using KNetwork sockets
                             -------------------
    begin                : Wed Jul 7 2004
    copyright            : (C) 2004 by Till Gerken <till@tantalo.net>
    Copyright 			 : (c) 2006      Novell, Inc	 	 	 http://www.opensuse.org

			   Kopete (C) 2004-2007 Kopete developers <kopete-devel@kde.org>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qobject.h>
#include <kdebug.h>
#include <k3resolver.h>

#include "gwbytestream.h"
#include "gwerror.h"

KNetworkByteStream::KNetworkByteStream ( QObject *parent )
 : ByteStream ( parent )
{
	kDebug ( GROUPWISE_DEBUG_GLOBAL ) << "Instantiating new KNetwork byte stream.";

	// reset close tracking flag
	mClosing = false;

	mSocket = new KNetwork::KBufferedSocket;

	// make sure we get a signal whenever there's data to be read
	mSocket->enableRead ( true );

	// connect signals and slots
	QObject::connect ( mSocket, SIGNAL ( gotError ( int ) ), this, SLOT ( slotError ( int ) ) );
	QObject::connect ( mSocket, SIGNAL ( connected ( const KNetwork::KResolverEntry& ) ), this, SLOT ( slotConnected () ) );
	QObject::connect ( mSocket, SIGNAL ( closed () ), this, SLOT ( slotConnectionClosed () ) );
	QObject::connect ( mSocket, SIGNAL ( readyRead () ), this, SLOT ( slotReadyRead () ) );
	QObject::connect ( mSocket, SIGNAL ( bytesWritten ( qint64 ) ), this, SLOT ( slotBytesWritten ( qint64 ) ) );

}

bool KNetworkByteStream::connect ( QString host, QString service )
{
	kDebug ( GROUPWISE_DEBUG_GLOBAL ) << "Connecting to " << host << ", service " << service;

	return socket()->connect ( host, service );

}

bool KNetworkByteStream::isOpen () const
{

	// determine if socket is open
	return socket()->isOpen ();

}

void KNetworkByteStream::close ()
{
	kDebug ( GROUPWISE_DEBUG_GLOBAL ) << "Closing stream.";

	// close the socket and set flag that we are closing it ourselves
	mClosing = true;
	socket()->close();

}

int KNetworkByteStream::tryWrite ()
{

	// send all data from the buffers to the socket
	QByteArray writeData = takeWrite();
	socket()->write ( writeData.data (), writeData.size () );

	return writeData.size ();

}

KNetwork::KBufferedSocket *KNetworkByteStream::socket () const
{

	return mSocket;

}

KNetworkByteStream::~KNetworkByteStream ()
{

	delete mSocket;

}

void KNetworkByteStream::slotConnected ()
{

	emit connected ();

}

void KNetworkByteStream::slotConnectionClosed ()
{
	kDebug ( GROUPWISE_DEBUG_GLOBAL ) << "Socket has been closed.";

	// depending on who closed the socket, emit different signals
	if ( mClosing )
	{
		kDebug ( GROUPWISE_DEBUG_GLOBAL ) << "..by ourselves!";
		kDebug( GROUPWISE_DEBUG_GLOBAL ) << "socket error is \"" << socket()->errorString() << "\"";
		emit connectionClosed ();
	}
	else
	{
		kDebug ( GROUPWISE_DEBUG_GLOBAL ) << "..by the other end";
		emit delayedCloseFinished ();
	}

}

void KNetworkByteStream::slotReadyRead ()
{

	// stuff all available data into our buffers
	QByteArray readBuffer ( socket()->bytesAvailable (), 0 );

	socket()->read ( readBuffer.data (), readBuffer.size () );

	appendRead ( readBuffer );

	emit readyRead ();

}

void KNetworkByteStream::slotBytesWritten ( qint64 bytes )
{

	emit bytesWritten ( bytes );

}

void KNetworkByteStream::slotError ( int code )
{
	kDebug ( GROUPWISE_DEBUG_GLOBAL ) << "Socket error " << code;

	emit error ( code );

}

#include "gwbytestream.moc"
