
/***************************************************************************
                   gwconnector.cpp  -  Socket Connector for KNetwork
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

#include "yahooconnector.h"

#include <k3bufferedsocket.h>
#include <kdebug.h>
#include <k3resolver.h>

#include "yahoobytestream.h"
#include "yahootypes.h"

KNetworkConnector::KNetworkConnector( QObject *parent )
		: Connector( parent )
{
	kDebug( YAHOO_RAW_DEBUG ) << "New KNetwork connector.";

	mErrorCode = KNetwork::KSocketBase::NoError;

	mByteStream = new KNetworkByteStream( this );

	connect( mByteStream, SIGNAL (connected()), this, SLOT (slotConnected()) );
	connect( mByteStream, SIGNAL (error(int)), this, SLOT (slotError(int)) );
	mPort = 5510;
}

KNetworkConnector::~KNetworkConnector()
{
	delete mByteStream;
}

void KNetworkConnector::connectToServer( const QString &server )
{
	Q_UNUSED( server );
	kDebug( YAHOO_RAW_DEBUG ) << "Initiating connection to " << mHost;
	Q_ASSERT( !mHost.isNull() );
	Q_ASSERT( mPort );

	mErrorCode = KNetwork::KSocketBase::NoError;

	if ( !mByteStream->connect( mHost, QString::number (mPort) ) )
	{
		// Houston, we have a problem
		mErrorCode = mByteStream->socket()->error();
		emit error();
	}
}

void KNetworkConnector::slotConnected()
{
	kDebug( YAHOO_RAW_DEBUG ) << "We are connected.";

	// FIXME: setPeerAddress() is something different, find out correct usage later
	//KInetSocketAddress inetAddress = mStreamSocket->address().asInet().makeIPv6 ();
	//setPeerAddress ( QHostAddress ( inetAddress.ipAddress().addr () ), inetAddress.port () );

	emit connected ();
}

void KNetworkConnector::slotError( int code )
{
	kDebug( YAHOO_RAW_DEBUG ) << "Error detected: " << code;

	mErrorCode = code;
	emit error ();
}

int KNetworkConnector::errorCode()
{
	return mErrorCode;
}

ByteStream *KNetworkConnector::stream() const
{
	kDebug(YAHOO_RAW_DEBUG) ;
	return mByteStream;
}

void KNetworkConnector::done()
{
	kDebug ( YAHOO_RAW_DEBUG ) ;
	mByteStream->close ();
}

void KNetworkConnector::setOptHostPort( const QString &host, quint16 port )
{
	kDebug ( YAHOO_RAW_DEBUG ) << "Manually specifying host " << host << " and port " << port;

	mHost = host;
	mPort = port;

}

#include "yahooconnector.moc"

// kate: indent-width 4; replace-tabs off; tab-width 4; space-indent off;
