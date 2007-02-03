/*
    switchboardbridge.cpp - Peer To Peer Switchboard Bridge

    Copyright (c) 2006 by Gregg Edghill     <gregg.edghill@gmail.com>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; version 2 of the License.               *
    *                                                                       *
    *************************************************************************
*/

#include "switchboardbridge.h"
#include "msnchatsession.h"
#include "binarypacketformatter.h"
#include <qvaluelist.h>
#include <kdebug.h>

namespace PeerToPeer
{

class SwitchboardBridge::SwitchboardBridgePrivate
{
	public:
		SwitchboardBridgePrivate() : sbnetwork(0l), maxPendingPackets(5), maxSendBufferSize(1202), switchboardRequestNecessary(false) {}

		MSNChatSession*	sbnetwork;
		Q_UINT32 maxPendingPackets;
		Q_UINT32 maxSendBufferSize;
		QValueList<QByteArray> pendingPackets;
		bool switchboardRequestNecessary;
};

SwitchboardBridge::SwitchboardBridge(MSNChatSession* sbnetwork, QObject *parent) : TransportBridge(parent), d(new SwitchboardBridgePrivate())
{
	d->sbnetwork = sbnetwork;
	// Connect the signal/slot
	QObject::connect(d->sbnetwork, SIGNAL(dataReceived(const QByteArray&)), this,
	SLOT(onDataReceived(const QByteArray&)));
	// Connect the signal/slot
	QObject::connect(d->sbnetwork, SIGNAL(onSend()), this,
	SLOT(onSend()));
}

SwitchboardBridge::~SwitchboardBridge()
{
	delete d;
	d = 0l;
}

const Q_UINT32 SwitchboardBridge::identifier() const
{
	return 0;
}

bool SwitchboardBridge::isReadyToSend() const
{
	return ((state() == TransportBridge::Connected)&&(d->pendingPackets.size() < d->maxPendingPackets));
}

const Q_UINT32 SwitchboardBridge::maxSendBufferSize()
{
	return d->maxSendBufferSize;
}

void SwitchboardBridge::onConnect()
{
	setState(TransportBridge::Connected);
	kdDebug() << k_funcinfo << endl;
	emit connected();

	QValueList<QByteArray>::Iterator i = d->pendingPackets.begin();
	while (i != d->pendingPackets.end())
	{
		QByteArray bytes = *i;
		sendViaNetwork(bytes);
		i++;
	}

	emit readyToSend();

	d->switchboardRequestNecessary = false;
}

void SwitchboardBridge::onDisconnect()
{
	setState(TransportBridge::Disconnected);
	kdDebug() << k_funcinfo << endl;
	d->switchboardRequestNecessary = true;
	emit disconnected();
}

void SwitchboardBridge::onDataReceived(const QByteArray& data)
{
	if (data.size() > 0)
	{
		if (data.size() > (sizeof(Packet::Header) + maxSendBufferSize() + 4))
		{
			// If the tunnelled message length exceeds the max send buffer
			// size, stop any further processing of the received message.
			kdDebug() << k_funcinfo << "Got data whose size exceeds max send buffer size="
			<< maxSendBufferSize() << " bytes -- ignoring it" << endl;
			return;
		}

		// Retrieve the tunnelled peer to peer transport data
		// and fire the data received event.
		//
		QDataStream stream(data, IO_ReadOnly);
		Packet packet = BinaryPacketFormatter::deserialize(&stream);
		Q_INT32 appId = 0;
		stream.setByteOrder(QDataStream::BigEndian);
		stream >> appId;

		emit packetReceived(packet);
	}
}

void SwitchboardBridge::onSend()
{
	QValueList<QByteArray>::Iterator i = d->pendingPackets.begin();
	d->pendingPackets.remove(i);
	if (d->pendingPackets.size() == 0)
	{
		emit readyToSend();
	}
}

void SwitchboardBridge::send(const Packet& packet, const Q_UINT32 appId)
{
	QByteArray bytes(sizeof(Packet::Header) + packet.header().payloadSize + 4);
	QDataStream stream(bytes, IO_WriteOnly);
	// Serialize the packet into the memory stream.
	BinaryPacketFormatter::serialize(packet, &stream);
	stream.setByteOrder(QDataStream::BigEndian);
	stream << appId;

	kdDebug() << k_funcinfo << "About to send datachunk of size "
		<< packet.size() << " bytes" << endl;

	QByteArray datachunk;
	datachunk.duplicate(bytes.data(), bytes.size());
	d->pendingPackets.append(datachunk);
	// Send the serialized bytes via the switchboard network.
	sendViaNetwork(datachunk);
}

bool SwitchboardBridge::requestSwitchboardIfNecessary()
{
	return false;
}

void SwitchboardBridge::sendViaNetwork(const QByteArray& bytes)
{
	if (state() == TransportBridge::Connected)
	{
		d->sbnetwork->send(bytes);
	}
	else
	{
		if (d->switchboardRequestNecessary && state() == TransportBridge::Disconnected)
		{
			kdDebug() << k_funcinfo << "Requesting switchboard -- bridge is disconnected" << endl;
			emit requestSwitchboard();
			d->switchboardRequestNecessary = false;
		}
	}
}

}

#include "switchboardbridge.moc"

