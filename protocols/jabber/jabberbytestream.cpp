
/***************************************************************************
                   jabberbytestream.cpp  -  Byte Stream for Jabber
                             -------------------
    begin                : Wed Jul 7 2004
    copyright            : (C) 2004 by Till Gerken <till@tantalo.net>

			   Kopete (C) 2004 Kopete developers <kopete-devel@kde.org>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License as        *
 *   published by the Free Software Foundation; either version 2.1 of the  *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 ***************************************************************************/

#include <qobject.h>
#include <kdebug.h>
#include <ksocketfactory.h>
#include "jabberbytestream.h"
#include "jabberprotocol.h"

JabberByteStream::JabberByteStream ( QObject *parent )
 : ByteStream ( parent )
{
	kDebug ( JABBER_DEBUG_GLOBAL ) << "Instantiating new Jabber byte stream.";

	// reset close tracking flag
	mClosing = false;

	mSocket = NULL;
}

void JabberByteStream::connect ( QString host, int port )
{
	kDebug ( JABBER_DEBUG_GLOBAL ) << k_funcinfo << "Connecting to " << host << ", port " << port << endl;

	mClosing = false;

	mSocket = KSocketFactory::connectToHost("xmpp", host, port);

	QObject::connect ( mSocket, SIGNAL ( error(QAbstractSocket::SocketError) ),
		this, SLOT ( slotError ( QAbstractSocket::SocketError ) ) );
	QObject::connect ( mSocket, SIGNAL ( connected () ), this, SLOT ( slotConnected () ) );
	QObject::connect ( mSocket, SIGNAL ( disconnected () ), this, SLOT ( slotConnectionClosed () ) );
	QObject::connect ( mSocket, SIGNAL ( readyRead () ), this, SLOT ( slotReadyRead () ) );
	QObject::connect ( mSocket, SIGNAL ( bytesWritten ( qint64 ) ), this, SLOT ( slotBytesWritten ( qint64 ) ) );
}

bool JabberByteStream::isOpen () const
{

	// determine if socket is open
	return socket()->isOpen ();

}

void JabberByteStream::close ()
{
	kDebug ( JABBER_DEBUG_GLOBAL ) << "Closing stream.";

	// close the socket and set flag that we are closing it ourselves
	mClosing = true;
        if (mSocket) {
             kDebug ( JABBER_DEBUG_GLOBAL ) << k_funcinfo << "socket is not null" << endl;
	     mSocket->close();
             kDebug ( JABBER_DEBUG_GLOBAL ) << k_funcinfo << "socket closed" << endl;
             mSocket->deleteLater();
             mSocket=NULL;
        }
}

int JabberByteStream::tryWrite ()
{

	// send all data from the buffers to the socket
	QByteArray writeData = takeWrite();
	socket()->write ( writeData.data (), writeData.size () );

	return writeData.size ();

}

QTcpSocket *JabberByteStream::socket () const
{

	return mSocket;

}

JabberByteStream::~JabberByteStream ()
{

	delete mSocket;

}

void JabberByteStream::slotConnected ()
{

	emit connected ();

}

void JabberByteStream::slotConnectionClosed ()
{
	kDebug ( JABBER_DEBUG_GLOBAL ) << "Socket has been closed.";

	// depending on who closed the socket, emit different signals
	if ( !mClosing )
	{
		emit connectionClosed ();
	}
	else
	{
		emit delayedCloseFinished ();
	}

	mClosing = false;

}

void JabberByteStream::slotReadyRead ()
{
	kDebug ( JABBER_DEBUG_GLOBAL ) << "called:  available: " << socket()->bytesAvailable ();
	appendRead ( socket()->readAll() );

	emit readyRead ();

}

void JabberByteStream::slotBytesWritten ( qint64 bytes )
{

	emit bytesWritten ( bytes );

}

void JabberByteStream::slotError ( QAbstractSocket::SocketError code )
{
	kDebug ( JABBER_DEBUG_GLOBAL ) << "Socket error '" <<  mSocket->errorString() <<  "' - Code : " << code;
	emit error ( code );
}

#include "jabberbytestream.moc"
