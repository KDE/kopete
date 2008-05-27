/*
    YMSG - Yahoo Protocol Knetwork Bytestream

    Copyright (C) 2004 by Till Gerken <till@tantalo.net>

    Kopete (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU General Public                   *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef YAHOOBYTESTREAM_H
#define YAHOOBYTESTREAM_H

#include <k3bufferedsocket.h>

#include "bytestream.h"


/**
 * Low level socket class, using KDE's KNetwork socket classes
 * @author Till Gerken
 */
 
class KNetworkByteStream : public ByteStream
{

Q_OBJECT

public:
	KNetworkByteStream ( QObject *parent = 0 );

	~KNetworkByteStream ();

	bool connect ( QString host, QString service );
	virtual bool isOpen () const;
	virtual void close ();

	KNetwork::KBufferedSocket *socket () const;

signals:
	void connected ();

protected:
	virtual int tryWrite ();

private slots:
	void slotConnected ();
	void slotConnectionClosed ();
	void slotReadyRead ();
	void slotBytesWritten ( qint64 );
	void slotError ( int );

private:
	KNetwork::KBufferedSocket *mSocket;
	bool mClosing;

};

#endif // YAHOOBYTESTREAM_H

// kate: indent-width 4; replace-tabs off; tab-width 4; space-indent off;
