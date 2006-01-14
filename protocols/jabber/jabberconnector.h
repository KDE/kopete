
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

#ifndef JABBERCONNECTOR_H
#define JABBERCONNECTOR_H

#include <xmpp.h>
#include "jabberbytestream.h"

class ByteStream;
class KResolverEntry;

/**
@author Till Gerken
*/
class JabberConnector : public XMPP::Connector
{

Q_OBJECT

public:
	JabberConnector ( QObject *parent = 0, const char *name = 0 );

	~JabberConnector ();

	void connectToServer ( const QString &server );
	ByteStream *stream () const;
	void done ();

	void setOptHostPort ( const QString &host, Q_UINT16 port );
	void setOptSSL ( bool );
	void setOptProbe ( bool );

	int errorCode ();

private slots:
	void slotConnected ();
	void slotError ( int );

private:
	QString mHost;
	Q_UINT16 mPort;
	int mErrorCode;

	JabberByteStream *mByteStream;

};

#endif
