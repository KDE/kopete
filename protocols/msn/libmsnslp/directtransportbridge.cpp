/*
    directtransportbridge.cpp - Peer To Peer Direct Transport Bridge

    Copyright (c) 2006 by Gregg Edghill     <gregg.edghill@gmail.com>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; version 2 of the License.               *
    *                                                                       *
    *************************************************************************
*/

#include "directtransportbridge.h"

namespace PeerToPeer
{

class DirectTransportBridge::DirectTransportBridgePrivate
{
	public :
		QValueList<QString> addresses;
		Q_UINT16 port;
};

DirectTransportBridge::DirectTransportBridge(const QValueList<QString>& addresses, const Q_UINT16 port, QObject *parent) : TransportBridge(parent), d(new DirectTransportBridgePrivate())
{
	d->addresses = addresses;
	d->port = port;
}

DirectTransportBridge::~DirectTransportBridge()
{
	delete d;
}

QValueList<QString> & DirectTransportBridge::addresses() const
{
	return d->addresses;
}

Q_UINT16 DirectTransportBridge::port() const
{
	return d->port;
}

}

#include "directtransportbridge.moc"
