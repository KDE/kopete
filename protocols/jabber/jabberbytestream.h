
/***************************************************************************
                   jabberbytestream.h  -  Byte Stream for Jabber
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

#ifndef JABBERBYTESTREAM_H
#define JABBERBYTESTREAM_H

#include <bytestream.h>
#include <QTcpSocket>
#include <kopete_export.h>


/**
@author Kopete Developers
*/
class JABBER_EXPORT JabberByteStream : public ByteStream
{

Q_OBJECT

public:
	JabberByteStream ( QObject *parent = 0 );

	virtual ~JabberByteStream ();

	void connect ( QString host, int port );
	virtual bool isOpen () const;
	virtual void close ();

	QTcpSocket *socket () const;

signals:
	void connected ();

protected:
	virtual int tryWrite ();

private slots:
	void slotConnected ();
	void slotConnectionClosed ();
	void slotReadyRead ();
	void slotBytesWritten ( qint64 );
	void slotError ( QAbstractSocket::SocketError );

private:
	QTcpSocket *mSocket;
	bool mClosing;

};

#endif
