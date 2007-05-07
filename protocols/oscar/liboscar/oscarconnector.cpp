
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

#include "oscarconnector.h"
#include "oscarbytestream.h"

#include <kdebug.h>

#include <QtNetwork/QTcpSocket>

KNetworkConnector::KNetworkConnector( QObject *parent )
		: Connector( parent )
{
	kDebug( 14151 ) << k_funcinfo << "New KNetwork connector." << endl;

	mErrorCode = QAbstractSocket::UnknownSocketError;   // no 'NoError' here :(

	mByteStream = new KNetworkByteStream( this );

	connect( mByteStream, SIGNAL ( connected () ), this, SLOT ( slotConnected () ) );
	connect( mByteStream, SIGNAL ( error ( QAbstractSocket::SocketError ) ), this, SLOT ( slotError ( QAbstractSocket::SocketError ) ) );
	mPort = 0;
}

KNetworkConnector::~KNetworkConnector()
{
	delete mByteStream;
}

void KNetworkConnector::connectToServer( const QString &server )
{
	kDebug( 14151 ) << k_funcinfo << "Initiating connection to " << mHost << endl;
	Q_ASSERT( !mHost.isEmpty() );
	Q_ASSERT( mPort );

	mErrorCode = QAbstractSocket::UnknownSocketError;

	if ( !mByteStream->connect ( mHost, mPort ) )
	{
		// Houston, we have a problem
		mErrorCode = mByteStream->socket()->error();
		emit error();
	}
}

void KNetworkConnector::slotConnected()
{
	kDebug( 14151 ) << k_funcinfo << "We are connected." << endl;

	// FIXME: setPeerAddress() is something different, find out correct usage later
	//KInetSocketAddress inetAddress = mStreamSocket->address().asInet().makeIPv6 ();
	//setPeerAddress ( QHostAddress ( inetAddress.ipAddress().addr () ), inetAddress.port () );

	emit connected ();
}

void KNetworkConnector::slotError( QAbstractSocket::SocketError code )
{
	kDebug( 14151 ) << k_funcinfo << "Error detected: " << code << endl;

	mErrorCode = code;
	emit error ();
}

QAbstractSocket::SocketError KNetworkConnector::errorCode()
{
	return mErrorCode;
}

ByteStream *KNetworkConnector::stream() const
{
	return mByteStream;
}

void KNetworkConnector::done()
{
	kDebug ( 14151 ) << k_funcinfo << endl;
	mByteStream->close ();
}

void KNetworkConnector::setOptHostPort( const QString &host, quint16 port )
{
	kDebug ( 14151 ) << k_funcinfo << "Manually specifying host " << host << " and port " << port << endl;

	mHost = host;
	mPort = port;

}

#include "oscarconnector.moc"

// kate: indent-width 4; replace-tabs off; tab-width 4; space-indent off;
