
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
#include <kbufferedsocket.h>


/**
@author Kopete Developers
*/
class JabberByteStream : public ByteStream
{

Q_OBJECT

public:
	JabberByteStream ( QObject *parent = 0, const char *name = 0 );

	~JabberByteStream ();

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
	void slotBytesWritten ( int );
	void slotError ( int );

private:
	KNetwork::KBufferedSocket *mSocket;
	bool mClosing;

};

#endif
