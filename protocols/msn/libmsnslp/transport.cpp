/*
    transport.cpp - Peer to Peer Transport class

    Copyright (c) 2006 by Gregg Edghill     <gregg.edghill@gmail.com>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; version 2 of the License.               *
    *                                                                       *
    *************************************************************************
*/

#include "transport.h"
#include "packetqueue.h"
#include "switchboardbridge.h"
#include "tcptransportbridge.h"
#include "sessionnotifier.h"
#include <qmutex.h>
#include <qtimer.h>
#include <qwaitcondition.h>
#include <kdebug.h>

namespace PeerToPeer
{

class Transport::TransportPrivate
{
	public:
		TransportPrivate() : sbridge(0l), stopScheduling(false), sending(false) {}

		QWaitCondition condition;
		QMap<Q_UINT32, TransportBridge*> downlinks;
		QMap<Q_UINT32, SessionNotifier*> handlers;
		QMutex lock;
		QMap<Q_UINT32, QByteArray> pendingDatagrams;
		QMap<Q_UINT32, PacketQueue*> queues;
		QMap<Q_UINT32, Packet> recentlyReceivedPackets;
		SwitchboardBridge *sbridge;
		bool stopScheduling;
		QMutex waitLock;
		QWaitCondition waitEvent;

		QValueList<Packet> packets;
		QValueList<Packet> queue;
		bool sending;
};

Transport::Transport(QObject *parent) : QObject(parent), QThread(), d(new TransportPrivate())
{
	d->queues.insert(0, new PacketQueue(1202, this));

	start();
}

Transport::~Transport()
{
	d->lock.lock();
	d->stopScheduling = true;
	d->condition.wakeOne();
	d->lock.unlock();

	d->waitLock.lock();
	d->waitEvent.wakeOne();
	d->waitLock.unlock();

	wait();
	delete d;
	d = 0l;
}

void Transport::setSwitchboardBridge(SwitchboardBridge* bridge)
{
	d->sbridge = bridge;
	// Connect the signal/slot
	QObject::connect(d->sbridge, SIGNAL(packetReceived(const Packet&)), this,
	SLOT(onPacketReceived(const Packet&)));
	QObject::connect(d->sbridge, SIGNAL(readyToSend()), this,
	SLOT(onSwitchboardReadyToSend()));
	d->downlinks.insert(0, bridge);
}

void Transport::run()
{
	while(!d->stopScheduling)
	{
		d->lock.lock();
		if (d->queues[0]->isEmpty())
		{
			// If the packet scheduler has no more
			// packets to schedule, sleep until a new
			// packet is queued.
			d->condition.wait(&d->lock);
		}
		d->lock.unlock();

		d->waitLock.lock();
		if (!d->sbridge->isReadyToSend() && !d->stopScheduling)
		{
			kdDebug() << k_funcinfo << "not ready to send ======== " << endl;
			// If the switchboard bridge is not ready to send more packets
			// wait for the bridge to be ready.
			d->waitEvent.wait(&d->waitLock);
		}
		d->waitLock.unlock();

		if (d->queues[0]->isEmpty()) continue;

		Q_UINT32 chunkSize = d->sbridge->maxSendBufferSize();
		const Packet packet = d->queues[0]->dequeue(chunkSize);

		Q_INT32 appId = 0;
		// Determine app id based on frame flag field.
		if (packet.header().destination != 0 || packet.header().destination != 64)
		{
			if (Packet::ObjectDataType == packet.header().type || Packet::FileDataType == packet.header().type)
			{
				appId = d->handlers[packet.header().destination]->type();
			}
		}

		d->sbridge->send(packet, appId);
	}
}

void Transport::queuePacket(const Packet& packet, const Q_UINT32 bridgeId)
{
	Packet::Header & h = packet.header();

	// Determine is there is a registered transport bridge
	// to send the supplied packet.
	if (d->downlinks.contains(bridgeId))
	{
		Q_UINT32 relatesTo = h.lprcvd;
		// If a previously received packet is associated with
		// the supplied packet to be sent, copy the correlating
		// information from the previous packet.
		if (d->recentlyReceivedPackets.contains(relatesTo))
		{
			// Retrieve the previous packet.
			const Packet & p = d->recentlyReceivedPackets[relatesTo];
			kdDebug() << k_funcinfo << "Packet=" << relatesTo << " retrieved from recently received list" << endl;

			// NOTE The last packet sent is the last packet
			// received by the peer and the last packet size
			// is the size of the last packet sent by the
			// peer.
			h.lpsent = p.header().lprcvd;
			h.lpsize = p.header().window;
			// Remove the previous packet as its lifetime
			// is only for the duration of a transaction.
			d->recentlyReceivedPackets.remove(relatesTo);
		}

		h.payloadSize = packet.payload().buffer().size();

		// Queue the packet on the scheduler associated
		// with the specified transport bridge.
		d->queues[bridgeId]->enqueue(packet);

		kdDebug() << k_funcinfo << "Queued packet=" << h.identifier << " (priority=" << packet.priority() << ") for session="
		<< h.destination << " on bridge=" << bridgeId << " (size " << (Q_UINT32)h.window << ")" << endl;
		// Wait the scheduler if it was waiting.
		d->condition.wakeOne();
	}
	else
	{
		kdDebug() << k_funcinfo << "Could not queue packet=" << h.identifier << " for session=" << h.destination
		<< " on bridge=" << bridgeId << " -- unknown bridge id" << endl;
	}
}

void Transport::sendAcknowledge(const Q_UINT32 destination, const Q_UINT32 identifier, const Q_UINT32 relatesTo, const Q_UINT32 priority)
{
	// Create the acknowledge packet to send.
	Packet packet;
	packet.setPriority(priority);

	Packet::Header & header = packet.header();
	header.destination = destination;
	header.identifier = identifier;
	header.window = 0;
	header.type = (Q_UINT32)Packet::AcknowledgeType;
	header.lprcvd = relatesTo;

	// Queue the packet on the specified transport bridge.
	queuePacket(packet, 0);
}

void Transport::sendBytes(const QByteArray& bytes, const Q_UINT32 destination, const Q_UINT32 identifier, const Q_UINT32 relatesTo, const Q_UINT32 priority)
{
	// Create the raw data packet to send.
	Packet packet;
	packet.setPriority(priority);

	Packet::Header & header = packet.header();
	header.destination = destination;
	header.identifier = identifier;
	header.window = bytes.size();
	header.type = (Q_UINT32)((d->handlers[destination]->type() == 1) ? Packet::ObjectDataType : Packet::FileDataType);
	header.lprcvd = relatesTo;
	// Write the datagram bytes in the packet payload
	packet.payload().writeBlock(bytes);

	// Queue the packet on the specified transport bridge.
	queuePacket(packet, 0);
}

void Transport::sendDatagram(const QByteArray& datagram, const Q_UINT32 destination, const Q_UINT32 identifier, const Q_UINT32 relatesTo, const Q_UINT32 priority)
{
	// Create the datagram packet to send.
	Packet packet;
	packet.setPriority(priority);

	Packet::Header & header = packet.header();
	header.destination = destination;
	header.identifier = identifier;
	header.window = datagram.size();
	header.type = (Q_UINT32)Packet::MessageType;
	header.lprcvd = relatesTo;
	// Write the datagram bytes in the packet payload
	packet.payload().writeBlock(datagram);

	// Queue the packet on the specified transport bridge.
	queuePacket(packet, 0);
}

void Transport::onSwitchboardReadyToSend()
{
	d->waitEvent.wakeOne();
}

void Transport::onPacketReceived(const Packet& packet)
{
	Packet::Header & h = packet.header();

	if (!d->handlers.contains(h.destination))
	{
		kdDebug() << k_funcinfo << "Packet=" << h.identifier <<" could not be processed -- no such destination="
			<< h.destination << endl;
		return;
	}

	SessionNotifier *handler = d->handlers[h.destination];
	// Log the final packet in a series of one
	// or more received packets for future lookup.
	if (h.payloadSize + h.offset == h.window)
	{
		kdDebug() << k_funcinfo << "Packet=" << h.identifier << " added to recently received list" << endl;
		d->recentlyReceivedPackets.insert(h.identifier, packet);
	}

	// Try to process the received packet based on the packet type.
	Packet::Type packetType = (Packet::Type)h.type;
	if (Packet::AcknowledgeType == packetType)
	{
		kdDebug() << k_funcinfo << "Packet=" << h.identifier << " is ACK to sent packet=" << h.lprcvd << endl;

		// Determine is there is a registered port to
		// received the specified datagram.
		if (handler != 0l) handler->fireMessageAcknowledged(h.lprcvd);
		// Remove the sent packet from the queue
	}
	else
	if (Packet::EndOfDataType == packetType)
	{
		kdDebug() << k_funcinfo << "End of data ACK excepted for sent data packet=" << h.lpsent << endl;
	}
	else
	if (Packet::FaultType == packetType)
	{
		kdDebug() << k_funcinfo << "Transport fault" << endl;
	}
	else
	if (Packet::HandshakeType == packetType)
	{
		kdDebug() << k_funcinfo << "Transport handshake" << endl;
	}
	else
	if (Packet::MessageType == packetType)
	{
		// NOTE Datagrams can be fragmented depending on the MTU of the transport bridge;
		// therefore, we try to reassemble the datagram is necessary using the data
		// from the packet received.

		QByteArray datagram;
		if (h.payloadSize != h.window)
		{
			// The datagram is chunked (fragmented) so reassemble the
			// datagram using the received packet payload fragment.
			reassembleDatagram(packet, datagram);
		}
		else
		{
			// Otherwise, the datagram is not chunked so
			// no need to try to reassemble it.
			datagram = packet.payload().buffer();
		}

		// Try to dispatch the received datagram to a waiting consumer
		// higher in the stack if the datagram is valid.
		if (!datagram.isNull() && !tryDispatchDatagram(datagram, h.identifier, h.lprcvd, h.destination))
		{
			// If we get here, we have an undeliverable datagram.
			kdDebug() << k_funcinfo << "Datagram=" << h.identifier <<" could not be dispatched -- no such destination="
			<< h.destination << endl;

			// Remove the logged packet associated with the undeliverable datagram;
			d->recentlyReceivedPackets.remove(h.identifier);
		}
	}
	else
	if (Packet::ResetType == packetType)
	{
		kdDebug() << k_funcinfo << "transport reset" << endl;
	}
	else
	if (Packet::ObjectDataType == packetType || Packet::FileDataType == packetType)
	{
		if (handler != 0l)
		{
			handler->fireDataReceived(packet.payload().buffer());

			if (h.payloadSize + h.offset == h.window)
			{
				handler->fireEndOfData(h.identifier);
			}
		}
	}
	else
	if (Packet::TimeoutType == packetType)
	{
		kdDebug() << k_funcinfo << "ACK expected for received packet=" << h.lpsent << endl;
		if (handler != 0l) handler->fireTransactionTimedout(h.identifier, h.lpsent);
	}
	else
	{
		kdDebug() << k_funcinfo << "Got unknown packet type" << endl;
	}
}

//////////////////////////////////////////////////////////////////////
// Handles an ACK message received from the switchboard for a
// sent messages
//////////////////////////////////////////////////////////////////////
void Transport::onPacketSent(const Q_UINT32 identifier, const bool acknowledged)
{
	Packet packet;
	if (acknowledged)
	{
		Packet::Type type = (Packet::Type)packet.header().type;
		if (type == Packet::AcknowledgeType)
		{
			kdDebug() << k_funcinfo << "Removing sent packet=" << packet.header().identifier
			<< " ACK to received packet=" << packet.header().lprcvd << endl;
		}
		else
		if (type == Packet::MessageType)
		{
			kdDebug() << k_funcinfo << "Adding sent packet=" << packet.header().identifier
			<< " to the list of unacknowledged packets" << endl;
		}
		else
		if (type == Packet::TimeoutType)
		{
			kdDebug() << k_funcinfo << "Removing sent packet=" << packet.header().identifier
			<< " Non-ACK control packet" << endl;
		}
	}
	else
	{
		kdDebug() << k_funcinfo << "Removing sent packet=" << packet.header().identifier << endl;
	}
}

bool Transport::findOrCreateDatagram(const Packet& packet, QByteArray& datagram)
{
	Packet::Header & h = packet.header();
	bool datagramCreated = false;
	if (!d->pendingDatagrams.contains(h.identifier))
	{
		// If the datagram does not already exist, create
		// a new datagram of size packet.window bytes.
		datagram = QByteArray(h.window);
		datagramCreated = true;
	}
	else
	{
		// Otherwise, retrieve the existing entry.
		datagram = d->pendingDatagrams[h.identifier];
	}

	return datagramCreated;
}

void Transport::reassembleDatagram(const Packet& packet, QByteArray& datagram)
{
	QByteArray data;
	Packet::Header & h = packet.header();

	// Determine if the received packet is the first in a series
	// of packets that contain fragmented datagram data.  If so,
	// create an entry in the pending datagram collection and
	// buffer the data received in the packet.
	if (findOrCreateDatagram(packet, data))
	{
		d->pendingDatagrams.insert(h.identifier, data);
	}

	QBuffer buffer(data);
	// Open the memory buffer for writing.
	buffer.open(IO_WriteOnly);
	// Seek to the offset in the buffer where the
	// received datagram fragment data will be written.
	buffer.at(h.offset);
	// Write the data into the buffer.
	buffer.writeBlock(packet.payload().buffer().data(), h.payloadSize);
	buffer.close();

	// Determine if the received packet is the last in a series
	// of packets that contain fragmented datagram data.  If so,
	// we assume that the datagram has been reassembled.
	if (h.payloadSize + h.offset == h.window)
	{
		// Remove the reassembled datagram from the pending datagram collection.
		d->pendingDatagrams.remove(h.identifier);
		datagram = data;
	}
}

bool Transport::tryDispatchDatagram(const QByteArray& datagram, const Q_UINT32 identifier, const Q_UINT32 relatesTo, const Q_UINT32 destination)
{
	bool dispatched = false;
	// Determine is there is a registered handler to dispatch the specified datagram.
	if (d->handlers.contains(destination))
	{
		// Queue the datagram on the specified port and dispatch
		// it to a waiting comsumer higher in the stack.
		d->handlers[destination]->fireMessageReceived(datagram, identifier, relatesTo);
		dispatched = true;
	}

	return dispatched;
}

//BEGIN Session Handler Functions

void Transport::registerReceiver(Q_UINT32 port, SessionNotifier* receiver)
{
	if (!d->handlers.contains(port))
	{
		d->handlers.insert(port, receiver);
// 		if (port != 0 && port != 64)
// 		{
// 			d->queues.insert(port, new PacketQueue(1202, this));
// 		}
	}
}

void Transport::unregisterReceiver(Q_UINT32 port)
{
	if (d->handlers.contains(port))
	{
		d->handlers.remove(port);
// 		if (port != 0 && port != 64)
// 		{
// 			PacketQueue *queue = d->queues[port];
// 			d->queues.remove(port);
// 			queue->deleteLater();
// 			queue = 0l;
// 		}
	}
}

//END

//BEGIN Endpoint Functions

bool Transport::listen(const QString& address, const Q_UINT16 port)
{
	TcpTransportBridge *bridge = new TcpTransportBridge(address, port, this);
	bool listening = bridge->listen();

	return listening;
}

//END

//BEGIN Bridge Functions

const Q_UINT32 Transport::createBridge(const QString& address, const Q_UINT16 port)
{
	TcpTransportBridge *bridge = new TcpTransportBridge(address, port, this);
	// Connect the signal/slot
	QObject::connect(bridge, SIGNAL(connected()), this,
	SLOT(onBridgeConnected()));
	QObject::connect(bridge, SIGNAL(disconnected()), this,
	SLOT(onBridgeDisconnected()));
	QObject::connect(bridge, SIGNAL(packetReceived(const Packet&)), this,
	SLOT(onPacketReceived(const Packet&)));
	bridge->connect();

	return bridge->identifier();
}

void Transport::onBridgeConnected()
{
	const TcpTransportBridge *bridge = dynamic_cast<const TcpTransportBridge*>(sender());
	if (bridge != 0)
	{
		emit bridgeConnected(bridge->identifier());
		kdDebug() << k_funcinfo << "TCP bridge=" << bridge->identifier() << " connected" << endl;
	}
}

void Transport::onBridgeDisconnected()
{
	const TcpTransportBridge *bridge = dynamic_cast<const TcpTransportBridge*>(sender());
	if (bridge != 0)
	{
		kdDebug() << k_funcinfo << "TCP bridge=" << bridge->identifier() << " disconnected" << endl;
	}
}

//END

}

#include "transport.moc"
