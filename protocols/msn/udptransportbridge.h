/*
    udptransportbridge.h - UDP Peer to peer transport

    Copyright (c) 2007 by Michel Saliba     <msalibaba@gmail.com>

    Kopete    (c) 2002-2007 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/
#ifndef UDPTRANSPORTBRIDGE_H
#define UDPTRANSPORTBRIDGE_H

//BEGIN QT Includes
#include <qobject.h>
#include <qguardedptr.h>
#include <qvaluelist.h>
//END

//BEGIN KDE Includes
#include <ksocketaddress.h>
#include <transport.h>
//END

class UdpTransportBridge : public TransportBridge
{
	Q_OBJECT
	friend class Transport;
	
public:
	virtual ~UdpTransportBridge();

private:
	UdpTransportBridge(const KNetwork::KInetSocketAddress& to, MessageFormatter* formatter, QObject* parent);
	UdpTransportBridge(KNetwork::KClientSocketBase* socket, MessageFormatter* formatter, QObject* parent);

protected slots:
	virtual void slotOnConnect();
	virtual void slotOnDisconnect();
	virtual void slotOnError(int);
	virtual void slotOnSocketClose();
	virtual void slotOnSocketConnect();
	virtual void slotOnSocketReceive();
	
private slots:
	void slotOnSocketConnectTimeout();

signals:
	void bridgeConnectTimeout();

private:
	class Buffer : public QByteArray
	{
	public:
		Buffer(quint32 length = 0);
		~Buffer();
		
	public:
		void write(const QByteArray& bytes);
		QByteArray read(quint32 length);
	};
	
	Buffer mBuffer;
	quint32 mLength;
};

#endif
