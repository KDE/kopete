
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

JabberConnector::JabberConnector ( QObject *parent, const char */*name*/ )
 : XMPP::Connector ( parent )
{
	kdDebug ( JABBER_DEBUG_GLOBAL ) << k_funcinfo << "New Jabber connector." << endl;

	mErrorCode = KNetwork::KSocketBase::NoError;

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
	kdDebug ( JABBER_DEBUG_GLOBAL ) << k_funcinfo << "Initiating connection to " << server << endl;

	/*
	 * FIXME: we should use a SRV lookup to determine the
	 * actual server to connect to. As this is currently
	 * not supported yet, we're using setOptHostPort().
	 * For XMPP 1.0, we need to enable this!
	 */

	mErrorCode = KNetwork::KSocketBase::NoError;

	if ( !mByteStream->connect ( mHost, QString::number ( mPort ) ) )
	{
		// Houston, we have a problem
		mErrorCode = mByteStream->socket()->error ();
		emit error ();
	}

}

void JabberConnector::slotConnected ()
{
	kdDebug ( JABBER_DEBUG_GLOBAL ) << k_funcinfo << "We are connected." << endl;

	// FIXME: setPeerAddress() is something different, find out correct usage later
	//KInetSocketAddress inetAddress = mStreamSocket->address().asInet().makeIPv6 ();
	//setPeerAddress ( QHostAddress ( inetAddress.ipAddress().addr () ), inetAddress.port () );

	emit connected ();

}

void JabberConnector::slotError ( int code )
{
	kdDebug ( JABBER_DEBUG_GLOBAL ) << k_funcinfo << "Error detected: " << code << endl;

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

void JabberConnector::setOptHostPort ( const QString &host, Q_UINT16 port )
{
	kdDebug ( JABBER_DEBUG_GLOBAL ) << k_funcinfo << "Manually specifying host " << host << " and port " << port << endl;

	mHost = host;
	mPort = port;

}

void JabberConnector::setOptSSL ( bool ssl )
{
	kdDebug ( JABBER_DEBUG_GLOBAL ) << k_funcinfo << "Setting SSL to " << ssl << endl;

	setUseSSL ( ssl );

}

void JabberConnector::setOptProbe ( bool )
{
	// FIXME: Implement this.
}

#include "jabberconnector.moc"
