
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

#include "gwbytestream.h"
#include <ksocketfactory.h>

#include <kdebug.h>

#include "kopetesockettimeoutwatcher.h"

#include "gwerror.h"

KNetworkByteStream::KNetworkByteStream ( QObject *parent )
 : ByteStream ( parent )
{
	kDebug () << "Instantiating new KNetwork byte stream.";

	// reset close tracking flag
	mClosing = false;

	mSocket = 0;

}

bool KNetworkByteStream::connect ( QString host, QString service )
{
	kDebug () << "Connecting to " << host << ", service " << service;
	mSocket = KSocketFactory::connectToHost( "gwims", host, service.toUInt(), this );

	Kopete::SocketTimeoutWatcher* timeoutWatcher = Kopete::SocketTimeoutWatcher::watch( mSocket );
	if ( timeoutWatcher )
	{
		QObject::connect( timeoutWatcher, SIGNAL(error(QAbstractSocket::SocketError)),
		                  this, SLOT(slotError(QAbstractSocket::SocketError)) );
	}

	QObject::connect( mSocket, SIGNAL(error(QAbstractSocket::SocketError)),
			this, SLOT(slotError(QAbstractSocket::SocketError)) );
	QObject::connect( mSocket, SIGNAL(connected()), this, SLOT(slotConnected()) );
	QObject::connect( mSocket, SIGNAL(disconnected()), this, SLOT(slotConnectionClosed()) );
	QObject::connect( mSocket, SIGNAL(readyRead()), this, SLOT(slotReadyRead()) );
	QObject::connect( mSocket, SIGNAL(bytesWritten(qint64)), this, SLOT(slotBytesWritten(qint64)) );
	return true;
}

bool KNetworkByteStream::isOpen () const
{

	// determine if socket is open
	if ( socket() )
	{
		return socket()->isOpen ();
	}
	else
	{
		return false;
	}

}

void KNetworkByteStream::close ()
{
	kDebug () << "Closing stream.";

	// close the socket and set flag that we are closing it ourselves
	mClosing = true;
	if ( socket() )
		socket()->close();

}

int KNetworkByteStream::tryWrite ()
{

	// send all data from the buffers to the socket
	QByteArray writeData = takeWrite();
	socket()->write ( writeData.data (), writeData.size () );

	return writeData.size ();

}

QTcpSocket *KNetworkByteStream::socket () const
{

	return mSocket;

}

KNetworkByteStream::~KNetworkByteStream ()
{
}

void KNetworkByteStream::slotConnected ()
{

	emit connected ();

}

void KNetworkByteStream::slotConnectionClosed ()
{
	kDebug () << "Socket has been closed.";

	// depending on who closed the socket, emit different signals
	if ( mClosing )
	{
		kDebug () << "..by ourselves!";
		kDebug() << "socket error is \"" << socket()->errorString() << "\"";
		emit connectionClosed ();
	}
	else
	{
		kDebug () << "..by the other end";
		emit delayedCloseFinished ();
	}

}

void KNetworkByteStream::slotReadyRead ()
{
	appendRead ( socket()->readAll() );

	emit readyRead ();

}

void KNetworkByteStream::slotBytesWritten ( qint64 bytes )
{

	emit bytesWritten ( bytes );

}

void KNetworkByteStream::slotError ( QAbstractSocket::SocketError code )
{
	kDebug () << "Socket error " <<  mSocket->errorString() <<  "' - Code : " << code;
	emit error ( code );
}

#include "gwbytestream.moc"
