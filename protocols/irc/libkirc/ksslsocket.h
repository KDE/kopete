
#ifndef _K_SSL_SOCKET_H_
#define _K_SSL_SOCKET_H_

/*
    ksslsocket.h - KDE SSL Socket

    Copyright (c) 2004      by Jason Keirstead <jason@keirstead.org>

    Kopete    (c) 2002-2003 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include <kextsock.h>
class KSSLSocketPrivate;
class SSLPollThread;

class KSSLSocket : public KExtendedSocket
{
	friend class SSLPollThread;

	Q_OBJECT

	public:
		KSSLSocket();
		~KSSLSocket();

		Q_LONG readBlock( char* data, Q_ULONG maxLen );
		Q_LONG writeBlock( const char* data, Q_ULONG len );
		int peekBlock( char *data, uint maxLen );
		int bytesAvailable() const;

	signals:
		void sslFailure();

	private slots:
		void slotConnected();
		void slotDisconnected();
		void slotPoll();

	private:
		void readData( int bytes );
		KSSLSocketPrivate *d;
};

#endif
