
/***************************************************************************
                   gwbytestream.cpp  -  Byte Stream using KNetwork sockets
                             -------------------
    begin                : Wed Jul 7 2004
    copyright            : (C) 2004 by Till Gerken <till@tantalo.net>

			   Kopete (C) 2004 Kopete developers <kopete-devel@kde.org>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "oscarbytestream.h"

#include <kdebug.h>
#include <ksocketfactory.h>

#include <QtCore/QObject>
#include <QtNetwork/QTcpSocket>

KNetworkByteStream::KNetworkByteStream( QObject *parent )
 : ByteStream ( parent )
{
	kDebug( 14151 ) << k_funcinfo << "Instantiating new KNetwork byte stream." << endl;

	// reset close tracking flag
	mClosing = false;

	mSocket = 0;
}

bool KNetworkByteStream::connect( QString host, unsigned short port )
{
	kDebug( 14151 ) << k_funcinfo << "Connecting to " << host << ", port " << port << endl;

    if( mSocket )
        delete mSocket;

    mSocket = KSocketFactory::connectToHost( QLatin1String( "tcp" ), host, port, this );

	// connect signals and slots
	QObject::connect( mSocket, SIGNAL ( error ( QAbstractSocket::SocketError ) ), this, SLOT ( slotError ( QAbstractSocket::SocketError ) ) );
	QObject::connect( mSocket, SIGNAL ( connected () ), this, SLOT ( slotConnected () ) );
	QObject::connect( mSocket, SIGNAL ( disconnected ()  ), this, SLOT ( slotDisconnected () ) );
	QObject::connect( mSocket, SIGNAL ( readyRead () ), this, SLOT ( slotReadyRead () ) );
	QObject::connect( mSocket, SIGNAL ( bytesWritten ( qint64 ) ), this, SLOT ( slotBytesWritten ( qint64 ) ) );

    return true;
}

bool KNetworkByteStream::isOpen() const
{
	// determine if socket is open
	return mSocket && mSocket->isOpen();
}

void KNetworkByteStream::close ()
{
#ifdef OSCAR_EXCEContactVE_DEBUG
	kDebug ( 14151 ) << k_funcinfo << "Closing stream." << endl;
#endif
	// close the socket and set flag that we are closing it ourselves
	mClosing = true;
    if( mSocket )
        mSocket->close();
}

int KNetworkByteStream::tryWrite ()
{
	// send all data from the buffers to the socket
	QByteArray writeData = takeWrite();
#ifdef OSCAR_EXCEContactVE_DEBUG
	kDebug(14151) << k_funcinfo << "writing " << writeData.size() << " bytes." << endl;
#endif
    if( !mSocket )
        return false;

    mSocket->write( writeData.data (), writeData.size () );
	return writeData.size();
}

QTcpSocket *KNetworkByteStream::socket() const
{
	return mSocket;
}

KNetworkByteStream::~KNetworkByteStream()
{
	// If socket is open than it has data in buffer, so delete socket later
	if ( mSocket->isOpen() )
		QObject::connect( mSocket, SIGNAL(closed()), mSocket, SLOT(deleteLater()) );
	else
		delete mSocket;
}

void KNetworkByteStream::slotConnected()
{
	emit connected();
}

void KNetworkByteStream::slotDisconnected()
{
	kDebug( 14151 ) << k_funcinfo << "Socket has been closed." << endl;

	// depending on who closed the socket, emit different signals
	if ( mClosing )
	{
		kDebug( 14151 ) << "..by ourselves!" << endl;
		kDebug( 14151 ) << "socket error is " << mSocket->errorString() << endl;
		emit connectionClosed ();
	}
	else
	{
		kDebug( 14151 ) << "..by the other end" << endl;
		emit delayedCloseFinished ();
	}
}

void KNetworkByteStream::slotReadyRead()
{
	// stuff all available data into our buffers
	QByteArray readBuffer = mSocket->readAll();

	appendRead( readBuffer );

	emit readyRead();
}

void KNetworkByteStream::slotBytesWritten( qint64 bytes )
{
	emit bytesWritten( bytes );
}
void KNetworkByteStream::slotError( QAbstractSocket::SocketError code )
{
	kDebug( 14151 ) << k_funcinfo << "Socket error " << code << endl;

    emit error ( code );
}

#include "oscarbytestream.moc"

// kate: indent-width 4; replace-tabs off; tab-width 4; space-indent off;
