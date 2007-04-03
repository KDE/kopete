
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
#include "jabberbytestream.h"
#include <k3bufferedsocket.h>
#include <k3resolver.h>
#include "jabberprotocol.h"

JabberByteStream::JabberByteStream ( QObject *parent )
 : ByteStream ( parent )
{
	kDebug ( JABBER_DEBUG_GLOBAL ) << k_funcinfo << "Instantiating new Jabber byte stream." << endl;

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

bool JabberByteStream::connect ( QString host, QString service )
{
	kDebug ( JABBER_DEBUG_GLOBAL ) << k_funcinfo << "Connecting to " << host << ", service " << service << endl;

	mClosing = false;

	return socket()->connect ( host, service );

}

bool JabberByteStream::isOpen () const
{

	// determine if socket is open
	return socket()->isOpen ();

}

void JabberByteStream::close ()
{
	kDebug ( JABBER_DEBUG_GLOBAL ) << k_funcinfo << "Closing stream." << endl;

	// close the socket and set flag that we are closing it ourselves
	mClosing = true;
	socket()->close();

}

int JabberByteStream::tryWrite ()
{

	// send all data from the buffers to the socket
	QByteArray writeData = takeWrite();
	socket()->write ( writeData.data (), writeData.size () );

	return writeData.size ();

}

KNetwork::KBufferedSocket *JabberByteStream::socket () const
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
	kDebug ( JABBER_DEBUG_GLOBAL ) << k_funcinfo << "Socket has been closed." << endl;

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
	kDebug ( JABBER_DEBUG_GLOBAL ) << k_funcinfo << "called:  available: " << socket()->bytesAvailable () << endl;
	appendRead ( socket()->readAll() );

	emit readyRead ();

}

void JabberByteStream::slotBytesWritten ( qint64 bytes )
{

	emit bytesWritten ( bytes );

}

void JabberByteStream::slotError ( int code )
{
	kDebug ( JABBER_DEBUG_GLOBAL ) << k_funcinfo << "Socket error '" <<  mSocket->errorString() <<  "' - Code : " << code <<  endl;
	if(KNetwork::KSocketBase::isFatalError( code ))
		emit error ( code );
}

#include "jabberbytestream.moc"
