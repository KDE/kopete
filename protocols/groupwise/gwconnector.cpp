
/***************************************************************************
                   gwconnector.cpp  -  Socket Connector for KNetwork
                             -------------------
    begin                : Wed Jul 7 2004
    copyright            : (C) 2004 by Till Gerken <till@tantalo.net>

			   Kopete (C) 2004-2007 Kopete developers <kopete-devel@kde.org>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License as        *
 *   published by the Free Software Foundation; either version 2.1 of the  *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 ***************************************************************************/

#include "gwconnector.h"
#include <k3bufferedsocket.h>
#include <kdebug.h>
#include <k3resolver.h>

#include "gwerror.h"
#include "gwbytestream.h"

KNetworkConnector::KNetworkConnector ( QObject *parent )
 : Connector ( parent )
{
	kDebug () << "New KNetwork connector.";

	mErrorCode = 0;

	mByteStream = new KNetworkByteStream ( this );

	connect ( mByteStream, SIGNAL (connected()), this, SLOT (slotConnected()) );
	connect ( mByteStream, SIGNAL (error(int)), this, SLOT (slotError(int)) );
	mPort = 0;
}

KNetworkConnector::~KNetworkConnector ()
{

	delete mByteStream;

}

void KNetworkConnector::connectToServer ( const QString & /*server*/ )
{
	kDebug () << "Initiating connection to " << mHost;
	Q_ASSERT( !mHost.isNull() );
	Q_ASSERT( mPort );
	/*
	 * FIXME: we should use a SRV lookup to determine the
	 * actual server to connect to. As this is currently
	 * not supported yet, we're using setOptHostPort().
	 * For XMPP 1.0, we need to enable this!
	 */

	mErrorCode = 0;

	mByteStream->connect ( mHost, QString::number ( mPort ) );
}

void KNetworkConnector::slotConnected ()
{
	kDebug() << "We are connected.";

	// FIXME: setPeerAddress() is something different, find out correct usage later
	//KInetSocketAddress inetAddress = mStreamSocket->address().asInet().makeIPv6 ();
	//setPeerAddress ( QHostAddress ( inetAddress.ipAddress().addr () ), inetAddress.port () );

	emit connected ();

}

void KNetworkConnector::slotError ( int code )
{
	kDebug() << "Error detected: " << code;

	mErrorCode = code;
	emit error ();
}

int KNetworkConnector::errorCode ()
{

	return mErrorCode;

}

ByteStream *KNetworkConnector::stream () const
{

	return mByteStream;

}

void KNetworkConnector::done ()
{
	kDebug () ;
	mByteStream->close ();
}

void KNetworkConnector::setOptHostPort ( const QString &host, quint16 port )
{
	kDebug () << "Manually specifying host " << host << " and port " << port;

	mHost = host;
	mPort = port;

}

void KNetworkConnector::setOptSSL ( bool ssl )
{
	kDebug () << "Setting SSL to " << ssl;

	setUseSSL ( ssl );

}

#include "gwconnector.moc"
