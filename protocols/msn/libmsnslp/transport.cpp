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
#include <qregexp.h>
#include <qtimer.h>
#include <qwaitcondition.h>
#include <kdebug.h>

#include <arpa/inet.h>
#include <stdlib.h>

namespace PeerToPeer
{

class Transport::TransportPrivate
{
	public:
		TransportPrivate() : defaultBridgeId(0), sequenceNumber((Q_INT32)(((double)(rand() * 1.0/0x7FFFFFFF))*(0x186A0 - 0x3104)) + 0x3104),
		sbridge(0l), stopScheduling(false), sending(false) {}

		QWaitCondition condition;
		QMap<Q_UINT32, QBuffer*> buffers;
		QMap<Q_UINT32, TransportBridge*> bridges;
		Q_UINT32 defaultBridgeId;
		QMap<Q_UINT32, SessionNotifier*> handlers;
		QMutex lock;
		QMap<Q_UINT32, QByteArray> pendingDatagrams;
		QMap<Q_UINT32, PacketQueue*> queues;
		QMap<Q_UINT32, Packet> recentlyReceivedPackets;
		Q_UINT32 sequenceNumber; // Sequence number for ordering packets.
		SwitchboardBridge *sbridge;
		bool stopScheduling;
		QMutex waitLock;
		QWaitCondition waitEvent;

		QMap<Q_INT32, QUuid> nonces;
		QValueList<Packet> packets;
		QValueList<Packet> queue;
		bool sending;
};

Transport::Transport(QObject *parent) : QObject(parent), QThread(), d(new TransportPrivate())
{
	d->queues.insert(0, new PacketQueue(this));

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

Q_UINT32 Transport::nextPacketSequenceNumber() const
{
	return (++d->sequenceNumber);
}

void Transport::setSwitchboardBridge(SwitchboardBridge* bridge)
{
	d->sbridge = bridge;
	// Connect the signal/slot
	QObject::connect(d->sbridge, SIGNAL(packetReceived(const Packet&)), this,
	SLOT(onReceive(const Packet&)));
	QObject::connect(d->sbridge, SIGNAL(readyToSend()), this,
	SLOT(onSwitchboardReadyToSend()));

	d->bridges.insert(0, bridge);
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
			appId = d->handlers[packet.header().destination]->type();
		}

		d->sbridge->send(packet, appId);
	}
}

void Transport::queuePacket(const Packet& packet, const Q_UINT32 bridgeId, bool prepend)
{
	Packet::Header & h = packet.header();

	// Determine if there is a registered transport bridge
	// to send the supplied packet.
	if (d->bridges.contains(bridgeId))
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
		d->queues[bridgeId]->enqueue(packet, prepend);

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

void Transport::sendAcknowledge(const Q_UINT32 destination, const Q_UINT32 lprcvd)
{
	sendControlPacket(Packet::AcknowledgeType, destination, lprcvd, 0, 0);
}

void Transport::sendControlPacket(const Packet::Type type, const Q_UINT32 destination, const Q_UINT32 lprcvd, const Q_UINT32 lpsent, const Q_UINT64 lpsize)
{
	Packet packet;
	packet.setPriority(0);

	// Get the header field of the packet.
	Packet::Header & h = packet.header();
	// Set the destination of the packet.
	h.destination = destination;
	// Set the sequence number of the packet.
	h.identifier = nextPacketSequenceNumber();
	// Set the data window size offset of the packet.
	h.offset = 0;
	// Set the data window size of the packet.
	h.window = 0;
	// Set the payload size of the packet.
	h.payloadSize = 0;
	// Set the type of the packet.
	h.type = (Q_UINT32)type;
	// Set the sequence number of a received packet
	// this packet correlates to.
	h.lprcvd = lprcvd;
	h.lpsent = lpsent;
	h.lpsize = lpsize;

	// Queue the control packet on the specified transport bridge.
	queuePacket(packet, d->defaultBridgeId, true);
}

void Transport::sendBytes(const QByteArray& bytes, const Q_UINT32 destination, const Q_UINT32 relatesTo, const Q_UINT32 priority)
{
	// Create the raw data packet to send.
	Packet packet;
	packet.setPriority(priority);

	Packet::Header & header = packet.header();
	header.destination = destination;
	header.identifier = nextPacketSequenceNumber();
	header.window = bytes.size();
	header.type = (Q_UINT32)((d->handlers[destination]->type() == 1) ? Packet::ObjectDataType : Packet::FileDataType);
	header.lprcvd = relatesTo;
	// Write the datagram bytes in the packet payload
	packet.payload().writeBlock(bytes);

	// Queue the packet on the specified transport bridge.
	queuePacket(packet, 0);
}

Q_UINT32 Transport::send(const QByteArray& message, const Q_UINT32 destination, const Q_UINT32 relatesTo, const Q_UINT32 priority)
{
	Packet packet;
	// Set the priority of the message to the specified priority.
	packet.setPriority(priority);

	// Get the header field of the packet.
	Packet::Header & h = packet.header();
	// Set the destination of the packet.
	h.destination = destination;
	// Set the sequence number of the packet.
	h.identifier = nextPacketSequenceNumber();
	// Set the data window size of the packet.
	h.window = message.size();
	// Set the type of the packet.
	h.type = (Q_UINT32)Packet::MessageType;
	// Set the sequence number of a received packet
	// this packet correlates to.
	h.lprcvd = relatesTo;

	// Write the message bytes to the packet payload
	packet.payload().writeBlock(message);

	// Queue the packet on the specified transport bridge.
	queuePacket(packet, 0);

	return h.identifier;
}

void Transport::sendNonce(const QUuid& nonce, const Q_UINT32 bridgeId)
{
	// Convert the nonce to byte parameters.
	const QString cnonce = nonce.toString().remove(QRegExp("[\\-\\{\\}]")).upper();

	const Q_UINT32 l  = cnonce.mid(0, 8).toUInt(0, 16);
	const Q_UINT16 w1 = cnonce.mid(12, 4).toUShort(0, 16);
	const Q_UINT16 w2 = cnonce.mid(8, 4).toUShort(0, 16);
	const Q_UINT32 b1_4 = cnonce.mid(24, 8).toUInt(0, 16);
	const Q_UINT32 b5_8 = cnonce.mid(16, 8).toUInt(0, 16);

	const Q_UINT32 lprcvd = l;
	const Q_UINT32 lpsent = (Q_UINT32(w1) << 16) + w2;
	const Q_UINT64 lpsize = (Q_UINT64(htonl(b1_4)) << 32) + htonl(b5_8);

	Packet packet;
	packet.setPriority(0);

	// Get the header field of the packet.
	Packet::Header & h = packet.header();
	// Set the destination of the packet.
	h.destination = 0;
	// Set the sequence number of the packet.
	h.identifier = nextPacketSequenceNumber();
	// Set the data window size offset of the packet.
	h.offset = 0;
	// Set the data window size of the packet.
	h.window = 0;
	// Set the payload size of the packet.
	h.payloadSize = 0;
	// Set the type of the packet.
	h.type = (Q_UINT32)Packet::HandshakeType;

	h.lprcvd = lprcvd;
	h.lpsent = lpsent;
	h.lpsize = lpsize;

	// Queue the handshake packet on the specified transport bridge.
	queuePacket(packet, bridgeId, true);
}

//BEGIN Event Handler Functions

void Transport::onNonAcknowledgeControlPacketReceived(const Packet& packet)
{
	// Get the header field of received packet.
	Packet::Header & h = packet.header();
	// Get the packet type from the packet header.
	Packet::Type packetType = (Packet::Type)h.type;

	const QString flags =  QString::fromLatin1("0x") + QString::number(h.type, 16).rightJustify(8, '0');
	kdDebug() << k_funcinfo << "Control packet (flags " << flags << ")" << endl;

	switch(packetType)
	{
		case Packet::CancelType:
		{
			kdDebug() << k_funcinfo << "Cancel data exchange for session " << h.identifier << endl;
			break;
		}

		case Packet::MessageType:
		{
			kdDebug() << k_funcinfo << "Acknowledge expected for packet " << h.lpsent << endl;
			break;
		}

		case Packet::FaultType:
		{
			kdDebug() << k_funcinfo << "Transport fault notification" << endl;
			break;
		}

		case Packet::ResetType:
		{
			kdDebug() << k_funcinfo << "Reset transport.  Fail all sends and receives" << endl;
			break;
		}

		default:
			break;
	}
}

void Transport::onNonceReceived(const QUuid& cnonce, const Q_UINT32 bridgeId)
{
	if (bridgeId == d->defaultBridgeId)
	{
		kdDebug() << k_funcinfo << "Got nonce " << cnonce.toString().upper() << " via switchboard bridge. "
			<<  "Must have not been sent via direct bridge -- ignoring." << endl;
	}
	else
	{
		// Otherwise, get the nonce associated with the direct bridge.
		const QUuid nonce = d->nonces[bridgeId];

		if (cnonce != nonce)
		{
			kdDebug() << k_funcinfo << "Got nonce " << cnonce.toString().upper() << " excepted "
			<< nonce.toString().upper() <<  ".  Disconnecting bridge " << bridgeId << endl;
		}
	}
}

void Transport::onReceive(const Packet& packet)
{
	TransportBridge *bridge = static_cast<TransportBridge*>(const_cast<QObject*>(sender()));
	if (bridge == 0l)
	{
		return;
	}

	// Get the header field of received packet.
	Packet::Header & h = packet.header();
	// Get the packet type from the packet header.
	Packet::Type packetType = (Packet::Type)h.type;

	kdDebug() << k_funcinfo << "Received packet " << h.identifier << " for session " << h.destination
		<< " via bridge " << bridge->identifier() << endl;


	SessionNotifier *handler = d->handlers[h.destination];

	if (h.payloadSize + h.offset == h.window)
	{
		kdDebug() << k_funcinfo << "Adding packet " << h.identifier << " to recently received list" << endl;
		// TODO Log the final packet in a series of one or more received packets for future lookup.
		d->recentlyReceivedPackets.insert(h.identifier, packet);
	}

	if (Packet::AcknowledgeType == packetType)
	{
		kdDebug() << k_funcinfo << "Packet " << h.identifier << " is ACK to sent packet " << h.lprcvd
		<<  ".  Notifying session." << endl;
		// TODO
		if (handler != 0l) handler->fireMessageAcknowledged(h.lprcvd);
	}
	else
	if (Packet::TimeoutType == packetType || Packet::FaultType == packetType || Packet::ResetType == packetType || Packet::CancelType == packetType)
	{
		// Handle the transport control packet.
		onNonAcknowledgeControlPacketReceived(packet);
	}
	else
	if (Packet::MessageType == packetType)
	{
		// A message can be chunked depending on the MTU of the transport bridge the
		// message was sent on; therefore, we try to reassemble the message if necessary
		// using the data from the packet received.

		QByteArray data;
		if (h.payloadSize == h.window)
		{
			// If the data is not chunked, there
			// is no need to try to reassemble it.
			data = packet.payload().buffer();
		}
		else
		{
			// Otherwise, if the payload size of the packet is not
			// equal to the data window size of the packet, the data
			// is chunked so reassemble the data.
			reassembleData(packet, data);
		}

		if (!data.isNull())
		{
			kdDebug() << k_funcinfo << "Received all data from packet " << h.identifier
			<< ". Sending ACK." << endl;

			// Send an acknowledge to the peer endpoint.
			sendAcknowledge(h.destination, h.identifier);

			// If the byte array is valid, all data has been received.
			// Try to dispatch the received data to a comsumer waiting higher in the stack.
			dispatch(data, h.destination, h.identifier, h.lprcvd);
		}
	}
	else
	if (Packet::HandshakeType == packetType)
	{
		const QUuid nonce;
		// Get the byte parameters from the last three fields of the packet header.
		const Q_UINT32 l = h.lprcvd;
		const Q_UINT16 w1 = h.lpsent & 0xFFFF;
		const Q_UINT16 w2 = (h.lpsent >> 16) & 0xFFFF;
		const Q_UINT8 b1 = h.lpsize & 0xFF;
		const Q_UINT8 b2 = (h.lpsize >> 8) & 0xFF;
		const Q_UINT8 b3 = (h.lpsize >> 16) & 0xFF;
		const Q_UINT8 b4 = (h.lpsize >> 24) & 0xFF;
		const Q_UINT8 b5 = (h.lpsize >> 32) & 0xFF;
		const Q_UINT8 b6 = (h.lpsize >> 40) & 0xFF;
		const Q_UINT8 b7 = (h.lpsize >> 48) & 0xFF;
		const Q_UINT8 b8 = (h.lpsize >> 56) & 0xFF;

		// Create the comparison nonce from the byte parameters.
		const QUuid cnonce = QUuid(l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8);

		// Handle received nonce.
		onNonceReceived(cnonce, bridge->identifier());
	}
	else
	if (Packet::ObjectDataType == packetType || Packet::FileDataType == packetType)
	{
		if (handler != 0l)
		{
			if (h.payloadSize + h.offset == h.window)
			{
				// Send an acknowledge to the peer endpoint.
				sendAcknowledge(h.destination, h.identifier);
			}

			handler->fireDataReceived(packet.payload().buffer(), h.identifier, (h.payloadSize + h.offset == h.window));
		}
	}
	else
	{
	}
}

void Transport::onSend(const Packet& packet, const Q_UINT32 bridgeId)
{
	// Get the header field of the packet.
	const Packet::Header & h = packet.header();

	kdDebug() << k_funcinfo << "Sending packet " << h.identifier
	<< " (bytes [" << h.offset << ".." << h.payloadSize << "] of " << h.window << ")"
	<< " for session " << h.destination << " (type= " << h.type << ") via bridge "
	<< bridgeId << endl;
}

void Transport::onSendFailed(const Packet& packet)
{
	// Get the header field of the packet.
	const Packet::Header & h = packet.header();

	kdDebug() << k_funcinfo << "Packet " << h.identifier << " failed to be sent." << endl;
}

///////////////////////////////////////////////////////////////////////
// Handles ACK/NAK notifications for messages sent via the switchboard
// network.
///////////////////////////////////////////////////////////////////////
void Transport::onSent(const Q_UINT32 identifier, const bool packetSent)
{
	// Get the packet associated with the supplied identifier.
	const Packet packet;

	if (packetSent)
	{
		// Indicate that the packet was sent.
		onSent(packet);
	}
	else
	{
		// Otherwise, indicate that the packet failed to be sent.
		onSendFailed(packet);
	}
}

void Transport::onSent(const Packet& packet)
{
	// Get the header field of the packet.
	const Packet::Header & h = packet.header();

	// If the packet was sent, determine what to do
	// with the packet based on its type.
	kdDebug() << k_funcinfo << "Sent packet " << h.identifier
	<< " (bytes [" << h.offset << ".." << h.payloadSize << "] of " << h.window << ")"
	<< " for session " << h.destination << " (type= " << h.type << ") via bridge " << endl;

	// Get the packet type.
	Packet::Type type = (Packet::Type)packet.header().type;
	if (type == Packet::AcknowledgeType)
	{
		kdDebug() << k_funcinfo << "Removing sent packet " << h.identifier
		<< " ACK to received packet " << h.lprcvd << endl;
	}
	else
	if (type == Packet::MessageType)
	{
		kdDebug() << k_funcinfo << "Adding sent packet " << h.identifier
		<< " to the list of unacknowledged packets" << endl;

		// Add the sent packet to the list of unacknowledged packets.
	}
	else
	if (type == Packet::TimeoutType || type == Packet::FaultType || type == Packet::ResetType)
	{
		kdDebug() << k_funcinfo << "Removing sent packet " << h.identifier
		<< " Non-ACK control packet" << endl;
	}
	else
	{
		kdDebug() << k_funcinfo << "Removing sent packet " << h.identifier << endl;
	}
}

//END

void Transport::dispatch(const QByteArray& message, const Q_UINT32 destination, const Q_UINT32 identifier, const Q_UINT32 relatesTo)
{
	// Determine if there is a registered handler to dispatch the specified datagram.
	if (d->handlers.contains(destination))
	{
		// Queue the datagram on the specified port and dispatch
		// it to a waiting comsumer higher in the stack.
		d->handlers[destination]->fireMessageReceived(message, identifier, relatesTo);
	}
}

void Transport::reassembleData(const Packet& packet, QByteArray& data)
{
	kdDebug() << k_funcinfo << "enter" << endl;

	QBuffer *buffer = 0l;
	// Get the header field of the pakcet.
	const Packet::Header & h = packet.header();

	// Determine if the received packet is the first in a series
	// of packets that contain fragmented datagram data.
	if (!d->buffers.contains(h.identifier))
	{
		// If the buffer does not already exist, create
		// a new buffer.
		buffer = new QBuffer();
		kdDebug() << k_funcinfo << "created buffer " << buffer << endl;
		// Open the memory buffer for writing.
		buffer->open(IO_WriteOnly);
		// Add the buffer to the buffer collection.
		d->buffers.insert(h.identifier, buffer);
	}
	else
	{
		// Otherwise, retrieve the existing buffer.
		buffer = d->buffers[h.identifier];
	}

	if (buffer != 0)
	{
		kdDebug() << k_funcinfo << "Received (bytes [" << h.offset << ".." << (h.offset + h.payloadSize) << "] of " << h.window << ")"
		<< " for packet " << h.identifier << endl;

		// Seek to the offset in the buffer where the
		// received data chunk will be written.
		buffer->at(h.offset);
		// Write the data into the buffer.
		buffer->writeBlock(packet.payload().buffer().data(), h.payloadSize);

		// Determine if the received packet is the last in a series
		// of packets that contain chunked data.
		if (h.payloadSize + h.offset == h.window)
		{
			// Close the buffer.
			buffer->close();
			// Remove the reassembled datagram from the pending datagram collection.
			d->buffers.remove(h.identifier);
			// We assume that the data has been reassembled.
			data = buffer->buffer();
			// Dispose of the data buffer.
			delete buffer;
			buffer = 0l;
		}
	}

	kdDebug() << k_funcinfo << "leave" << endl;
}

void Transport::onSwitchboardReadyToSend()
{
	d->waitEvent.wakeOne();
}

//BEGIN Session Handler Functions

void Transport::registerReceiver(Q_UINT32 port, SessionNotifier* receiver)
{
	if (!d->handlers.contains(port))
	{
		d->handlers.insert(port, receiver);
	}
}

void Transport::unregisterReceiver(Q_UINT32 port)
{
	if (d->handlers.contains(port))
	{
		d->handlers.remove(port);
	}
}

//END

//BEGIN Endpoint Functions

bool Transport::listen(const QString& address, const Q_UINT16 port)
{
	QValueList<QString> addresses;
	addresses.append(address);
	TcpTransportBridge *bridge = new TcpTransportBridge(addresses, port, this);
	bool listening = bridge->listen();

	return listening;
}

//END

//BEGIN Bridge Functions

const Q_UINT32 Transport::createBridge(const QUuid& nonce, const QValueList<QString>& addresses, const Q_UINT16 port)
{
	TcpTransportBridge *bridge = new TcpTransportBridge(addresses, port, this);
	// Connect the signal/slot
	QObject::connect(bridge, SIGNAL(connected()), this,
	SLOT(onBridgeConnected()));
	QObject::connect(bridge, SIGNAL(disconnected()), this,
	SLOT(onBridgeDisconnected()));
	QObject::connect(bridge, SIGNAL(error()), this,
	SLOT(onBridgeError()));
	QObject::connect(bridge, SIGNAL(packetReceived(const Packet&)), this,
	SLOT(onPacketReceived(const Packet&)));
	bridge->connect();

	const Q_INT32 identifier = bridge->identifier();
	d->nonces.insert(identifier, nonce);

	return identifier;
}

void Transport::onBridgeConnected()
{
	TcpTransportBridge *bridge = dynamic_cast<TcpTransportBridge*>(const_cast<QObject*>(sender()));
	if (bridge != 0)
	{
		kdDebug() << k_funcinfo << "Bridge " << bridge->identifier() << " is now connected" << endl;

		const QUuid nonce = d->nonces[bridge->identifier()];
		const QString cnonce = nonce.toString().upper().remove('-').remove('{').remove('}');

		// Create the handshake packet to send.
		Packet packet;

		Packet::Header & header = packet.header();
		header.destination = 0;
		header.identifier = nextPacketSequenceNumber();
		header.window = 0;
		header.type = (Q_UINT32)Packet::HandshakeType;
		header.lprcvd = cnonce.mid(0, 8).toUInt(0, 16);

		Q_UINT32 middle = (cnonce.mid(12, 4).toUInt(0, 16) << 16);
		middle |= cnonce.mid(8, 4).toUInt(0, 16);
		header.lpsent = middle;

		Q_UINT64 lower = htonl(cnonce.mid(24, 8).toUInt(0, 16));
		lower <<= 32;
		lower |= htonl(cnonce.mid(16, 8).toUInt(0, 16));
		header.lpsize = lower;

		kdDebug() << k_funcinfo << "Sending nonce, " << nonce.toString().upper() << endl;

		bridge->send(packet);
	}
}

void Transport::onBridgeDisconnected()
{
	TcpTransportBridge *bridge = dynamic_cast<TcpTransportBridge*>(const_cast<QObject*>(sender()));
	if (bridge != 0)
	{
		kdDebug() << k_funcinfo << "Bridge " << bridge->identifier() << " is now disconnected" << endl;
		QObject::disconnect(bridge, 0, this, 0);

		bridge->deleteLater();
		bridge = 0l;
	}
}

void Transport::onBridgeError()
{
	TcpTransportBridge *bridge = dynamic_cast<TcpTransportBridge*>(const_cast<QObject*>(sender()));
	if (bridge != 0)
	{
		kdDebug() << k_funcinfo << "Error on bridge " << bridge->identifier() << endl;
		bridge->disconnect();
	}
}

//END

}

#include "transport.moc"
