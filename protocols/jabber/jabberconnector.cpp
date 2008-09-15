
/***************************************************************************
                   jabberconnector.cpp  -  Socket Connector for Jabber
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

#include <kdebug.h>
#include "jabberconnector.h"
#include "jabberbytestream.h"
#include "jabberprotocol.h"

JabberConnector::JabberConnector ( QObject *parent )
 : XMPP::Connector ( parent )
{
	kDebug ( JABBER_DEBUG_GLOBAL ) << "New Jabber connector.";

	mErrorCode = 0;

	mByteStream = new JabberByteStream ( this );

	connect ( mByteStream, SIGNAL ( connected () ), this, SLOT ( slotConnected () ) );
	connect ( mByteStream, SIGNAL ( error ( int ) ), this, SLOT ( slotError ( int ) ) );

}

JabberConnector::~JabberConnector ()
{

	delete mByteStream;

}

void JabberConnector::connectToServer ( const QString &server )
{
	kDebug ( JABBER_DEBUG_GLOBAL ) << "Initiating connection to " << server;

	/*
	 * FIXME: we should use a SRV lookup to determine the
	 * actual server to connect to. As this is currently
	 * not supported yet, we're using setOptHostPort().
	 * For XMPP 1.0, we need to enable this!
	 */

	mErrorCode = 0;

	mByteStream->connect ( mHost, mPort );

}

void JabberConnector::slotConnected ()
{
	kDebug ( JABBER_DEBUG_GLOBAL ) << "We are connected.";

	// FIXME: setPeerAddress() is something different, find out correct usage later
	//KInetSocketAddress inetAddress = mStreamSocket->address().asInet().makeIPv6 ();
	//setPeerAddress ( QHostAddress ( inetAddress.ipAddress().addr () ), inetAddress.port () );

	emit connected ();

}

void JabberConnector::slotError ( int code )
{
	kDebug ( JABBER_DEBUG_GLOBAL ) << "Error detected: " << code;

	mErrorCode = code;
	emit error ();

}

int JabberConnector::errorCode ()
{

	return mErrorCode;

}

ByteStream *JabberConnector::stream () const
{

	return mByteStream;

}

void JabberConnector::done ()
{

	mByteStream->close ();

}

void JabberConnector::setOptHostPort ( const QString &host, quint16 port )
{
	kDebug ( JABBER_DEBUG_GLOBAL ) << "Manually specifying host " << host << " and port " << port;

	mHost = host;
	mPort = port;

}

void JabberConnector::setOptSSL ( bool ssl )
{
	kDebug ( JABBER_DEBUG_GLOBAL ) << "Setting SSL to " << ssl;

	setUseSSL ( ssl );

}

void JabberConnector::setOptProbe ( bool )
{
	// FIXME: Implement this.
}

#include "jabberconnector.moc"
