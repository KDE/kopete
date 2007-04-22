/*
    transportbridge.cpp - Peer to Peer Transport Bride

    Copyright (c) 2006 by Gregg Edghill     <gregg.edghill@gmail.com>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; version 2 of the License.               *
    *                                                                       *
    *************************************************************************
*/

#include "transportbridge.h"

namespace PeerToPeer
{
class TransportBridge::TransportBridgePrivate
{
	public:
		TransportBridgePrivate() : state(TransportBridge::Created) {}

		TransportBridgeState state;
};

TransportBridge::TransportBridge(QObject *parent) : QObject(parent), d(new TransportBridgePrivate())
{
}

TransportBridge::~TransportBridge()
{
	delete d;
}

void TransportBridge::connect()
{
	onConnect();
}

void TransportBridge::disconnect()
{
	onDisconnect();
}

const TransportBridge::TransportBridgeState & TransportBridge::state() const
{
	return d->state;
}

void TransportBridge::setState(const TransportBridgeState& state)
{
	d->state = state;
}

}

#include "transportbridge.moc"
