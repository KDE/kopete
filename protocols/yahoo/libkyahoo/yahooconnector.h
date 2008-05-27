
/***************************************************************************
                   oscarconnector.h  -  Socket Connector for KNetwork
                             -------------------
    begin                : Wed Jul 7 2004
    copyright            : (C) 2004 by Till Gerken <till@tantalo.net>
                           (C) 2004 by Matt Rogers <matt.rogers@kdemail.net>

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

#ifndef YAHOOCONNECTOR_H
#define YAHOOCONNECTOR_H

#include "connector.h"
#include "libkyahoo_export.h"

class ByteStream;
class KNetworkByteStream;

/**
@author Till Gerken
@author Matt Rogers
*/
class LIBKYAHOO_EXPORT KNetworkConnector : public Connector
{

Q_OBJECT

public:
	KNetworkConnector( QObject *parent = 0 );

	virtual ~KNetworkConnector();

	virtual void connectToServer( const QString &server );
	virtual ByteStream *stream() const;
	virtual void done();

	void setOptHostPort( const QString &host, quint16 port );

	int errorCode();

private slots:
	void slotConnected();
	void slotError( int );

private:
	QString mHost;
	quint16 mPort;
	int mErrorCode;

	KNetworkByteStream *mByteStream;

};

#endif

// kate: indent-width 4; replace-tabs off; tab-width 4; space-indent off;
