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
#include <qmap.h>
#include <qpair.h>
#include <qtimer.h>
#include <qvaluelist.h>
#include <kdebug.h>

namespace PeerToPeer
{

class SwitchboardBridge::SwitchboardBridgePrivate
{
	public:
		SwitchboardBridgePrivate() : identifier(0), sbnetwork(0l), maxPendingPackets(5), maxSendBufferSize(1202), sending(false),sent(0), switchboardRequestNecessary(false) {}

		Q_UINT32 identifier;
		MSNChatSession*	sbnetwork;
		Q_UINT32 maxPendingPackets;
		Q_UINT32 maxSendBufferSize;
		QValueList< QPair<Packet, Q_UINT32> > pendingPackets;
		bool sending;
		Q_UINT32 sent;
		QMap<Q_INT32, Packet> sentPackets;
		bool switchboardRequestNecessary;
};

SwitchboardBridge::SwitchboardBridge(MSNChatSession* sbnetwork, QObject *parent) : TransportBridge(parent), d(new SwitchboardBridgePrivate())
{
	d->sbnetwork = sbnetwork;
	// Connect the signal/slot
	QObject::connect(d->sbnetwork, SIGNAL(dataReceived(const QByteArray&)), this,
	SLOT(onDataReceived(const QByteArray&)));
	// Connect the signal/slot
	QObject::connect(d->sbnetwork, SIGNAL(onSend(const Q_INT32)), this,
	SLOT(onSend(const Q_INT32)));
}

SwitchboardBridge::~SwitchboardBridge()
{
	delete d;
	d = 0l;
}

const Q_UINT32 SwitchboardBridge::id() const
{
	return d->identifier;
}

bool SwitchboardBridge::isReadyToSend() const
{
	return ((state() == TransportBridge::Connected) && (d->sent < d->maxPendingPackets));
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

	if (d->pendingPackets.size() > 0 && !d->sending)
	{
		QTimer::singleShot(0, this, SLOT(sendPendingPackets()));
	}
	else
	{
		emit readyToSend();
	}

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

void SwitchboardBridge::onSend(const Q_INT32 id)
{
	if (d->sentPackets.contains(id))
	{
		Packet packet = d->sentPackets[id];
		emit sentPacket(packet);

		d->sentPackets.remove(id);
		d->sent -= 1;

		if (d->pendingPackets.size() == 0)
		{
			emit readyToSend();
		}
	}
}

void SwitchboardBridge::send(const Packet& packet, const Q_UINT32 appId)
{
	kdDebug() << k_funcinfo << "About to send datachunk of size " << packet.size() << " bytes" << endl;

	if (d->pendingPackets.size() > 0 || state() != TransportBridge::Connected)
	{
		d->pendingPackets.append(qMakePair(packet, appId));
	}
	else
	{
		QByteArray bytes(packet.size() + 4);
		QDataStream stream(bytes, IO_WriteOnly);
		// Serialize the packet into the memory stream.
		BinaryPacketFormatter::serialize(packet, &stream);
		stream.setByteOrder(QDataStream::BigEndian);
		stream << appId;

		QByteArray datachunk;
		datachunk.duplicate(bytes.data(), bytes.size());
		// Send the serialized bytes via the switchboard network.
		const Q_INT32 id = sendViaNetwork(datachunk);
		if (id == -1)
		{
			d->pendingPackets.append(qMakePair(packet, appId));
		}
		else
		{
			d->sentPackets.insert(id, packet);
			d->sent += 1;
		}
	}
}

bool SwitchboardBridge::requestSwitchboardIfNecessary()
{
	return false;
}

const Q_INT32 SwitchboardBridge::sendViaNetwork(const QByteArray& bytes)
{
	Q_INT32 tId = -1;
	if (state() == TransportBridge::Connected)
	{
		tId = d->sbnetwork->send(bytes);
	}
	else
	{
		if (d->switchboardRequestNecessary || state() == TransportBridge::Disconnected)
		{
			kdDebug() << k_funcinfo << "Requesting switchboard -- bridge is disconnected" << endl;
			emit requestSwitchboard();
			d->switchboardRequestNecessary = false;
		}
	}

	return tId;
}

void SwitchboardBridge::sendPendingPackets()
{
	if (d->pendingPackets.size() == 0)
	{
		return;
	}

	d->sending = true;

	QValueList< QPair<Packet, Q_UINT32> >::Iterator i = d->pendingPackets.begin();
	while (i != d->pendingPackets.end())
	{
		Packet packet = (*i).first;
		Q_INT32 appId = (*i).second;

		QByteArray bytes(packet.size() + 4);
		QDataStream stream(bytes, IO_WriteOnly);
		// Serialize the packet into the memory stream.
		BinaryPacketFormatter::serialize(packet, &stream);
		stream.setByteOrder(QDataStream::BigEndian);
		stream << appId;

		QByteArray datachunk;
		datachunk.duplicate(bytes.data(), bytes.size());
		// Send the serialized bytes via the switchboard network.
		const Q_INT32 id = sendViaNetwork(datachunk);
		if (id == -1)
		{
			break;
		}
		else
		{
			d->sentPackets.insert(id, packet);
			d->pendingPackets.remove(i);
			d->sent += 1;
		}

		i++;
	}

	if (d->pendingPackets.size() == 0)
	{
		d->sending = false;
		emit readyToSend();
	}
}

}

#include "switchboardbridge.moc"

