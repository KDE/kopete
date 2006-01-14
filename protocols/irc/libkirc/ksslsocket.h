
#ifndef _K_SSL_SOCKET_H_
#define _K_SSL_SOCKET_H_

/*
    ksslsocket.h - KDE SSL Socket

    Copyright (c) 2005      by Tommi Rantala <tommi.rantala@cs.helsinki.fi>
    Copyright (c) 2004      by Jason Keirstead <jason@keirstead.org>

    Kopete    (c) 2002-2005 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include <qvariant.h>
#include <kextsock.h>
#include <kio/slavebase.h>

class KSSLSocketPrivate;

class KSSLSocket : public KExtendedSocket
{
	Q_OBJECT

	public:
		KSSLSocket();
		~KSSLSocket();

		Q_LONG readBlock( char* data, Q_ULONG maxLen );
		Q_LONG writeBlock( const char* data, Q_ULONG len );
		int peekBlock( char *data, uint maxLen );
		int bytesAvailable() const;

		void showInfoDialog();

	signals:
		void sslFailure();
		void certificateAccepted();
		void certificateRejected();

	private slots:
		void slotConnected();
		void slotDisconnected();
		void slotReadData();

	private:
		int verifyCertificate();
		int messageBox( KIO::SlaveBase::MessageBoxType type, const QString &text,
			const QString &caption,	const QString &buttonYes, const QString &buttonNo );


		//Copied frm tcpslavebase to simply integrating their dialog function
		void setMetaData( const QString &, const QVariant & );
		bool hasMetaData( const QString & );
		QString metaData( const QString & );

		KSSLSocketPrivate *d;
};

#endif
