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
#include "packetscheduler.h"
#include "binarypacketformatter.h"
#include "sessionnotifier.h"
#include "switchboardbridge.h"
#include "tcptransportbridge.h"
#include <qbuffer.h>
#include <qfile.h>
#include <qregexp.h>
#include <qtimer.h>
#include <kdebug.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <ctype.h>

namespace PeerToPeer
{

class Transport::TransportPrivate
{
	public:
		TransportPrivate() : currentBridgeId(0), defaultBridgeId(2), nextBridgeId(64) {}

		QMap<Q_UINT32, TransportBridge*> bridges;
		QMap<Q_UINT32, QBuffer*> buffers;
		Q_UINT32 currentBridgeId;
		Q_UINT32 defaultBridgeId;
		Q_UINT32 nextBridgeId;
		Q_UINT32 nextPacketNumber;
		QMap<Q_UINT32, QUuid> nonces;
		QMap<Q_UINT32, SessionNotifier*> notifiers;
		QMap<Q_UINT32, PacketList*> packetLists;
		QPtrList<Packet> receivedPackets;
		PacketScheduler *scheduler;
		QPtrList<Packet> sentPackets;
		QPtrList<Packet> unacknowledgedPackets;
};

Transport::Transport(QObject *parent) : QObject(parent), d(new TransportPrivate())
{
	d->nextPacketNumber = (Q_INT32)(((double)(rand() * 1.0/0x7FFFFFFF))*(0x186A0 - 0x3104)) + 0x3104;
	// Create the transport packet scheduler.
	d->scheduler = new PacketScheduler(this);
	// Automatically delete the packets in the received packets list.
	d->receivedPackets.setAutoDelete(true);
	// Automatically delete the packets in the sent packets list.
	d->sentPackets.setAutoDelete(true);
	// Automatically delete the packets in the unacknowledged packets list.
	d->unacknowledgedPackets.setAutoDelete(true);

	d->currentBridgeId = d->defaultBridgeId;
}

Transport::~Transport()
{
	QMap<Q_UINT32, PacketList*>::ConstIterator it;
	for (it = d->packetLists.begin(); it != d->packetLists.end(); ++it)
	{
		PacketList *list = it.data();
		list->setAutoDelete(true);

		delete list;
	}

	d->packetLists.clear();

	delete d;
}


//BEGIN Transport Internal Functions

const QMap<Q_UINT32, TransportBridge*> & Transport::getBridges()
{
	return d->bridges;
}

const QMap<Q_UINT32, PacketList*> & Transport::getPacketLists()
{
	return d->packetLists;
}

const QPtrList<Packet> & Transport::getSentPackets()
{
	return d->sentPackets;
}

//END

Q_UINT32 Transport::nextPacketSequenceNumber() const
{
	return (++d->nextPacketNumber);
}

Q_UINT32 Transport::send(const QByteArray& message, const Q_UINT32 destination, const Q_UINT32 correlationId)
{
	Packet *packet = new Packet();

	// Get the header field of the packet.
	Packet::Header & h = packet->header();
	// Set the destination of the packet.
	h.destination = destination;
	// Set the sequence number of the packet.
	h.identifier = nextPacketSequenceNumber();
	// Set the data window size offset of the packet.
	h.offset = 0;
	// Set the data window size of the packet.
	h.window = message.size();
	// Write the bytes to the packet payload
	packet->payload()->writeBlock(message);
	// Set the payload size of the packet.
	h.payloadSize = h.window;
	// Set the type of the packet.
	h.type = (Q_UINT32)Packet::MessageDataType;
	// Set the correlation info of the packet.
	h.lprcvd = correlationId;
	h.lpsent = 0;
	h.lpsize = 0;

	// Queue the packet.
	queuePacket(packet, d->currentBridgeId);

	return h.identifier;
}

void Transport::sendBytes(const QByteArray& bytes, const Q_UINT32 destination, const Q_UINT32 correlationId)
{
	Packet *packet = new Packet();

	// Get the header field of the packet.
	Packet::Header & h = packet->header();
	// Set the destination of the packet.
	h.destination = destination;
	// Set the sequence number of the packet.
	h.identifier = nextPacketSequenceNumber();
	// Set the data window size offset of the packet.
	h.offset = 0;
	// Set the data window size of the packet.
	h.window = bytes.size();
	// Write the bytes to the packet payload
	packet->payload()->writeBlock(bytes);
	// Set the payload size of the packet.
	h.payloadSize = h.window;
	// Set the type of the packet.
	h.type = (Q_UINT32)Packet::ObjectDataType;
	// Set the correlation info of the packet.
	h.lprcvd = correlationId;
	h.lpsent = 0;
	h.lpsize = 0;


	// Queue the packet.
	queuePacket(packet, d->currentBridgeId);
}

void Transport::sendFile(QFile *file, const Q_UINT32 destination)
{
	Packet *packet = new Packet();

	// Get the header field of the packet.
	Packet::Header & h = packet->header();
	// Set the destination of the packet.
	h.destination = destination;
	// Set the sequence number of the packet.
	h.identifier = nextPacketSequenceNumber();
	// Set the data window size offset of the packet.
	h.offset = 0;
	// Set the data window size of the packet.
	h.window = file->size();
	// Write the bytes to the packet payload
	packet->setPayload(file);
	// Set the payload size of the packet.
	h.payloadSize = h.window;
	// Set the type of the packet.
	h.type = (Q_UINT32)Packet::FileDataType;
	// Set the correlation info of the packet.
	h.lprcvd = rand() % 0xCF56;
	h.lpsent = 0;
	h.lpsize = 0;


	// Queue the packet.
	queuePacket(packet, d->currentBridgeId);
}

//BEGIN Transport Bridge Functions

DirectTransportBridge* Transport::createDirectBridge(const QString& type, const QValueList<QString>& addresses, const Q_UINT16 port, const QUuid& nonce)
{
	DirectTransportBridge *bridge = 0l;
	if (type == QString::fromLatin1("TCPv1"))
	{
		bridge = new TcpTransportBridge(addresses, port, nextBridgeId(), this);
	}

	if (bridge != 0l)
	{
		d->nonces.insert(bridge->id(), nonce);
		// Add the direct transport bridge to the list.
		addBridge(bridge);
	}

	bridge->connect();

	return bridge;
}

SwitchboardBridge* Transport::createIndirectBridge()
{
	SwitchboardBridge *bridge = new SwitchboardBridge(d->defaultBridgeId, this);
	// Add the switchboard bridge to the list.
	addBridge(bridge);

	return bridge;
}

bool Transport::listen(const QString& address, const Q_UINT16 port)
{
	QValueList<QString> addresses;
	addresses.append(address);

	TcpTransportBridge *bridge = new TcpTransportBridge(addresses, port, nextBridgeId(), this);
	bool listening = bridge->listen();
	if (!listening)
	{
		bridge->deleteLater();
		bridge = 0l;
	}

	return listening;
}

void Transport::addBridge(TransportBridge *bridge)
{
	if (bridge == 0l)
	{
		kdDebug() << k_funcinfo << "Parameter \'bridge\' cannot be null."  << endl;
		return;
	}

	if (!d->bridges.contains(bridge->id()))
	{
		// Add the bridge to the list of transport bridges.
		d->bridges.insert(bridge->id(), bridge);

		// Connect the signal/slot.
		QObject::connect(bridge, SIGNAL(connected()), this,
		SLOT(onBridgeConnect()));
		QObject::connect(bridge, SIGNAL(dataReceived(const QByteArray&)), this,
		SLOT(onReceive(const QByteArray&)));
		QObject::connect(bridge, SIGNAL(dataSent(const Q_UINT32)), this,
		SLOT(onSent(const Q_UINT32)));
		QObject::connect(bridge, SIGNAL(disconnected()), this,
		SLOT(onBridgeDisconnect()));
		QObject::connect(bridge, SIGNAL(error()), this,
		SLOT(onBridgeError()));

		// Assign a packet list to the bridge.
		d->packetLists.insert(bridge->id(), new PacketList());
	}
}

Q_UINT32 Transport::findBestBridge() const
{
	Q_UINT32 bridgeId = 0;
	QMap<Q_UINT32, TransportBridge*>::ConstIterator it;
	for(it = d->bridges.begin(); it != d->bridges.end(); ++it)
	{
		if ((*it)->inherits("PeerToPeer::DirectTransportBridge")
		 && (*it)->state() == TransportBridge::Connected)
		{
			bridgeId = it.key();
			break;
		}
	}

	if (bridgeId == 0)
	{
		bridgeId = d->defaultBridgeId;
	}

	return bridgeId;
}

void Transport::movePacketsBetweenBridges(const Q_UINT32 oldBridgeId, const Q_UINT32 newBridgeId)
{
	kdDebug() << k_funcinfo << "enter" << endl;

	PacketList *fromList = d->packetLists[oldBridgeId];
	PacketList *toList   = d->packetLists[newBridgeId];

	if (fromList->count() > 0)
	{
		kdDebug() << k_funcinfo << "moving " << fromList->count() << " packet(s) from bridge "
		<< oldBridgeId << "'s list to bridge " << newBridgeId << "'s list" << endl;

		QPtrListIterator<Packet> it(*fromList);

		Packet *packet = 0l;
		while((packet = it.current()) != 0l)
		{
			toList->append(packet);
			fromList->removeRef(packet);
			++it;
		}
	}
	else
	{
		kdDebug() << k_funcinfo << "bridge " << oldBridgeId << "'s list is empty -- returning" << endl;
	}

	d->currentBridgeId = newBridgeId;

	kdDebug() << k_funcinfo << "leave" << endl;
}

Q_UINT32 Transport::nextBridgeId() const
{
	return (++d->nextBridgeId);
}

void Transport::removeBridge(const Q_UINT32 id)
{
	// Check whether the bridge associated with the specified
	// id is registered and it is not the indirect bridge's id.
	if (id != d->defaultBridgeId && d->bridges.contains(id))
	{
		// If the id of the bridge does not match that of
		// the indirect bridge, remove the bridge.
		d->bridges.remove(id);
		// Delete the packet list.
		delete d->packetLists[id];
		// Remove the packet list assigned to the bridge.
		d->packetLists.remove(id);
		// Remove the handshake nonce assigned
		// to the bridge.
		d->nonces.remove(id);
	}
}

//END


void Transport::sendBridgeHandshake(const QUuid& nonce, const Q_UINT32 bridgeId)
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

	Packet *packet = new Packet();

	// Get the header field of the packet.
	Packet::Header & h = packet->header();
	// Set the destination of the packet.
	h.destination = 0;
	// Set the sequence number of the packet.
	h.identifier = nextPacketSequenceNumber();
	// Set the data window size offset of the packet.
	h.offset = 0;
	// Set the data window size of the packet.
	h.window = 0;
	// Set the payload size of the packet.
	h.payloadSize = h.window;
	// Set the type of the packet.
	h.type = (Q_UINT32)Packet::HandshakeNonceType;
	// Set the correlation info of the packet.
	h.lprcvd = lprcvd;
	h.lpsent = lpsent;
	h.lpsize = lpsize;

	// Queue the handshake nonce packet.
	queuePacket(packet, bridgeId, true);
}

//END

//BEGIN Transport Bridge Event Handling Functions

void Transport::onBridgeConnect()
{
	const TransportBridge *bridge = dynamic_cast<const TransportBridge*>(sender());
	if (bridge != 0l)
	{
		kdDebug() << k_funcinfo << "Bridge " << bridge->id() << " is now connected." << endl;

		// Determine using type reflection whether the bridge is
		// a switchboard bridge or a direct transport bridge.
		if (bridge->inherits("PeerToPeer::DirectTransportBridge"))
		{
			// If the transport bridge is a direct bridge, we need to send
			// the handshake nonce to authenticate the use of the bridge.
			const QUuid nonce = d->nonces[bridge->id()];
			// Send the bridge handshake nonce.
			sendBridgeHandshake(nonce, bridge->id());
		}
	}
}

void Transport::onBridgeDisconnect()
{
	TransportBridge *bridge = dynamic_cast<TransportBridge*>(const_cast<QObject*>(sender()));
	if (bridge != 0l)
	{
		kdDebug() << k_funcinfo << "Bridge " << bridge->id() << " is now disconnected." << endl;

		// Determine using type reflection whether the bridge is
		// a switchboard bridge or a direct transport bridge.
		if (bridge->inherits("PeerToPeer::DirectTransportBridge"))
		{
			// Disconnect the signal/slot.
			QObject::disconnect(bridge, 0, this, 0);

			movePacketsBetweenBridges(bridge->id(), findBestBridge());
			// Remove the direct bridge from the list.
			removeBridge(bridge->id());
			// Delete the direct transport bridge.
			bridge->deleteLater();
			bridge = 0l;
		}
	}
}

void Transport::onBridgeError()
{
	TransportBridge *bridge = dynamic_cast<TransportBridge*>(const_cast<QObject*>(sender()));
	if (bridge != 0)
	{
		kdDebug() << k_funcinfo << "disconnecting bridge " << bridge->id() << endl;
		bridge->disconnect();
	}
}

void Transport::onBridgeHandshake(const QUuid& cnonce, const Q_UINT32 bridgeId)
{
	kdDebug() << k_funcinfo << "enter" << endl;

	if (bridgeId == d->defaultBridgeId)
	{
		kdDebug() << k_funcinfo << "Got nonce " << cnonce.toString().upper() << " via indirect bridge. "
			<<  "Must have not been sent via direct bridge -- ignoring." << endl;
	}
	else
	{
		// Otherwise, get the handshake nonce associated with the direct bridge.
		const QUuid nonce = d->nonces[bridgeId];

		if (cnonce == nonce)
		{
			movePacketsBetweenBridges(d->currentBridgeId, bridgeId);
		}
		else
		{
			kdDebug() << k_funcinfo << "Got nonce " << cnonce.toString().upper() << " excepted "
			<< nonce.toString().upper() <<  ".  Disconnecting bridge " << bridgeId << endl;

			// Disconnect the transport bridge.
			TransportBridge *bridge = d->bridges[bridgeId];
			bridge->disconnect();
		}
	}

	kdDebug() << k_funcinfo << "leave" << endl;
}

void Transport::onReceive(const QByteArray& datachunk)
{
	const TransportBridge *bridge = dynamic_cast<const TransportBridge*>(sender());
	if (bridge == 0l)
	{
		kdDebug() << k_funcinfo << "Received data from non-transport bridge -- ignoring." << endl;
		return;
	}

	Q_UINT32 appId = 0;

	QDataStream stream(datachunk, IO_ReadOnly);
	// Deserialize the packet from the supplied stream.
	Packet* packet = BinaryPacketFormatter::deserialize(&stream);
	// Determine using type reflection whether the packet
	// was received via the switchboard bridge.
	if (bridge->inherits("PeerToPeer::SwitchboardBridge"))
	{
		// If the packet was received via the switchboard bridge,
		// deserialize the application id that is appended after
		// a serialized packet.
		stream.setByteOrder(QDataStream::BigEndian);
		stream >> appId;
	}

	// Add the packet to the list of received packets.
	d->receivedPackets.append(packet);

	// Get the header field of the received packet.
	Packet::Header & h = packet->header();

	kdDebug() << k_funcinfo << "Received packet " << h.identifier << " via bridge " << bridge->id() << endl
		<< packet->toString() << endl;

	// Get the packet type from the packet header.
	const Packet::Type packetType = (Packet::Type)h.type;

	if (Packet::TimeoutType == packetType || Packet::FaultType == packetType ||
		Packet::ResetType == packetType || Packet::CancelType == packetType)
	{
		// We received a non ACK control packet.
		onNonAcknowledgeControlPacketReceive(packet);

		// Remove and delete the received packet from the list.
		d->receivedPackets.removeRef(packet);
	}
	else
	if (Packet::AcknowledgeType == packetType)
	{
		// We received an ACK control packet.
		kdDebug() << k_funcinfo << "Packet " << h.identifier << " is ACK to sent packet " << h.lprcvd
		<<  ".  Notifying session" << endl;

		// Remove the now acknowledged packet from the
		// unacknowledged packet list.
		removePacketFromUnacknowledgedList(h.lprcvd);

		// Determine if there is a registered notifier.
		if (d->notifiers.contains(h.destination))
		{
			d->notifiers[h.destination]->fireMessageAcknowledged(h.lprcvd);
		}

		// Remove and delete the received packet from the list.
		d->receivedPackets.removeRef(packet);
	}
	else
	if (Packet::MessageDataType == packetType)
	{
		// We received a message data packet.

		// A message can be chunked depending on the MTU of the transport bridge the
		// message was sent on; therefore, we try to reassemble the message if necessary
		// using the data from the packet received.

		QByteArray data;
		if (h.payloadSize == h.window)
		{
			// If the data is not chunked, there
			// is no need to try to reassemble it.
			data = static_cast<QBuffer*>(packet->payload())->buffer();
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
			// If the byte array is valid, all data has been received.
			kdDebug() << k_funcinfo << "Received all data from packet " << h.identifier
				<< ". Sending ACK." << endl;
			// Send an acknowledge to the other side.
			sendAcknowledge(h.destination, h.identifier, h.lprcvd, h.lpsize);

			// Determine if there is a registered notifier.
			if (d->notifiers.contains(h.destination))
			{
				d->notifiers[h.destination]->fireMessageReceived(data, h.identifier, h.lprcvd);
			}

			d->receivedPackets.removeRef(packet);
		}
	}
	else
	if (Packet::HandshakeNonceType == packetType)
	{
		// We received a handshake nonce packet.

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
		const QUuid nonce = QUuid(l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8);

		onBridgeHandshake(nonce, bridge->id());

		// Remove and delete the received packet from the list.
		d->receivedPackets.removeRef(packet);
	}
	else
	if (Packet::ObjectDataType == packetType || Packet::FileDataType == packetType)
	{
		bool isLastChunk = (h.payloadSize + h.offset == h.window);
		// We received a raw data packet.
		// Determine if there is a registered notifier.
		if (d->notifiers.contains(h.destination))
		{
			d->notifiers[h.destination]->fireDataReceived(packet->payload()->readAll(), isLastChunk);
		}

		if (isLastChunk)
		{
			// Send an acknowledge to the peer endpoint.
			sendAcknowledge(h.destination, h.identifier, h.lprcvd, h.lpsize);
		}

		// Remove and delete the received packet from the list.
		d->receivedPackets.removeRef(packet);
	}
	else
	{
		// Otherwise, we have received an unknown packet.
		kdDebug() << k_funcinfo << "Received unknown packet " << h.identifier
			<< ". Sending fault." << endl;
		// Send a transport error control packet.
		sendFault(h.destination, h.identifier, h.lprcvd, h.lpsize);

		// Remove and delete the received packet from the list.
		d->receivedPackets.removeRef(packet);
	}
}

void Transport::onSendFailed(Packet* packet)
{
	if (packet == 0l)
	{
		kdDebug() << k_funcinfo << "Parameter \'packet\' cannot be null."  << endl;
		return;
	}

	// Get the header field of the packet.
	const Packet::Header & h = packet->header();

	kdDebug() << k_funcinfo << "Packet " << h.identifier << " could not be sent." << endl;
}

void Transport::onSent(const Q_UINT32 packetId)
{
	const TransportBridge *bridge = dynamic_cast<const TransportBridge*>(sender());
	if (bridge == 0l)
	{
		kdDebug() << k_funcinfo << "Received event from non-transport bridge.  This should not happen." << endl;
		return;
	}

	Packet *packet = 0l;
	// Try to retrieve the sent packet from the list.
	for(Q_UINT32 i=0; i < d->sentPackets.count(); ++i)
	{
		if (d->sentPackets.at(i) && d->sentPackets.at(i)->header().identifier == packetId)
		{
			packet = d->sentPackets.at(i);
			break;
		}
	}

	if (packet == 0l)
	{
		kdDebug() << k_funcinfo << "Could not find packet " << packetId << " -- returning" << endl;
		return;
	}

	// Get the header field of the packet.
	const Packet::Header & h = packet->header();
	// Get the packet type from the packet header.
	const Packet::Type packetType = (Packet::Type)h.type;

	if (Packet::AcknowledgeType == packetType)
	{
		kdDebug() << k_funcinfo << "Removing sent packet " << h.identifier
		<< " ACK to received packet " << h.lprcvd << endl;
		// Delete the sent packet.
		d->sentPackets.removeRef(packet);
	}
	else
	if (Packet::MessageDataType == packetType)
	{
		kdDebug() << k_funcinfo << "Adding sent packet " << h.identifier
		<< " to the list of unacknowledged packets" << endl;

		// Add the packet to the unacknowledged packet list.
		addPacketToUnacknowledgedList(packet);
		// Remove the packet from the sent packets list.
		d->sentPackets.take();
	}
	else
	if (Packet::TimeoutType == packetType || Packet::FaultType == packetType ||
		Packet::ResetType == packetType || Packet::CancelType == packetType)
	{
		kdDebug() << k_funcinfo << "Removing sent packet " << h.identifier
		<< " non ACK control packet" << endl;
		// Delete the sent packet.
		d->sentPackets.removeRef(packet);
	}
	else
	{
		kdDebug() << k_funcinfo << "Removing sent packet " << h.identifier
		<< endl;
		// Delete the sent packet.
		d->sentPackets.removeRef(packet);
	}


	bool canStopScheduling = true;
	QMap<Q_UINT32, PacketList*>::ConstIterator it;
	// Determine whether there are more packets to schedule.
	for(it = d->packetLists.begin(); it != d->packetLists.end(); ++it)
	{
		canStopScheduling &= it.data()->isEmpty();
	}

	if (canStopScheduling)
	{
		// If there are no more packets to schedule,
		// stop the packet scheduler.
		d->scheduler->stop();
	}
}

//END

//BEGIN Transport Packet Scheduling Functions

void Transport::queuePacket(Packet *packet, const Q_UINT32 bridgeId, bool prepend)
{
	if (packet == 0l)
	{
		kdDebug() << k_funcinfo << "Parameter \'packet\' cannot be null."  << endl;
		return;
	}

	// Get the header field of the packet.
	const Packet::Header & h = packet->header();

	// Determine if there is a registered transport
	// bridge to send the supplied packet.
	if (d->bridges.contains(bridgeId))
	{
		// Add the packet to the bridge's assigned send queue.
		PacketList *list = d->packetLists[bridgeId];
		prepend ? list->prepend(packet) : list->append(packet);

		kdDebug() << k_funcinfo << "Queued packet " << h.identifier << " for session "
		<< h.destination << " on bridge " << bridgeId << " (size " << (Q_UINT32)h.window << ")" << endl;

		if (!d->scheduler->isRunning())
		{
			// If the packet scheduler is not running,
			// start the scheduler.
			d->scheduler->start();
		}
	}
	else
	{
		// Otherwise, delete the packet.
		kdDebug() << k_funcinfo << "Could not queue packet " << h.identifier << " for session " << h.destination
		<< " on bridge " << bridgeId << " -- bridge could not be found." << endl;

		delete packet;
		packet = 0l;
	}
}

void Transport::sendInternal(Packet* packet, const Q_UINT32 bridgeId)
{
	if (packet == 0l)
	{
		kdDebug() << k_funcinfo << "Parameter \'packet\' cannot be null."  << endl;
		return;
	}

	// Get the header field of the packet.
	const Packet::Header & h = packet->header();

	kdDebug() << k_funcinfo << "Sending packet " << h.identifier << " via bridge " << bridgeId << endl
		<< packet->toString() << endl;

	// Get the bridge the packet is going to be sent via.
	TransportBridge *bridge = d->bridges[bridgeId];

	QByteArray datachunk;

	QDataStream stream(datachunk, IO_WriteOnly);
	// Serialize the packet to the given stream.
	BinaryPacketFormatter::serialize(&stream, packet);

	// Determine using type reflection whether the
	// specified bridge is a switchboard bridge.
	if (bridge->inherits("PeerToPeer::SwitchboardBridge"))
	{
		Q_UINT32 appId = 0;
		// Determine app id based on the session notifier type.
		if (h.destination != 0 || h.destination != 64)
		{
			if (d->notifiers.contains(h.destination))
			{
				appId = d->notifiers[h.destination]->type();
			}
		}

		// Serialize and append the application
		// id to the datachunk.
		stream.setByteOrder(QDataStream::BigEndian);
		stream << appId;
	}

	// Send the datachunk via the selected transport bridge.
	bridge->send(datachunk, h.identifier);

	// Add the packet to the sent packets list.
	d->sentPackets.append(packet);
}

//END

//BEGIN Transport Transmission Control Functions

void Transport::sendAcknowledge(const Q_UINT32 destination, const Q_UINT32 lprcvd, const Q_UINT32 lpsent, const Q_UINT64 lpsize)
{
	kdDebug() << k_funcinfo << "enter" << endl;
	// Send a message/data acknowledge control packet.
	sendControlPacket(Packet::AcknowledgeType, destination, lprcvd, lpsent, lpsize);
	kdDebug() << k_funcinfo << "leave" << endl;
}

void Transport::sendCancelData(const Q_UINT32 destination, const Q_UINT32 lprcvd, const Q_UINT32 lpsent, const Q_UINT64 lpsize)
{
	kdDebug() << k_funcinfo << "enter" << endl;
	// Send a cancel data exchange control packet which informs
	// the other side that we have canceled the sending of data.
	sendControlPacket(Packet::CancelType, destination, lprcvd, lpsent, lpsize);
	kdDebug() << k_funcinfo << "leave" << endl;
}

void Transport::sendFault(const Q_UINT32 destination, const Q_UINT32 lprcvd, const Q_UINT32 lpsent, const Q_UINT64 lpsize)
{
	kdDebug() << k_funcinfo << "enter" << endl;
	// Send a transport fault control packet.
	sendControlPacket(Packet::FaultType, destination, lprcvd, lpsent, lpsize);
	kdDebug() << k_funcinfo << "leave" << endl;
}

void Transport::sendReset(const Q_UINT32 destination, const Q_UINT32 lprcvd, const Q_UINT32 lpsent, const Q_UINT64 lpsize)
{
	kdDebug() << k_funcinfo << "enter" << endl;
	// Send a session reset control packet.
	sendControlPacket(Packet::ResetType, destination, lprcvd, lpsent, lpsize);
	kdDebug() << k_funcinfo << "leave" << endl;
}

void Transport::sendTimeout(const Q_UINT32 destination, const Q_UINT32 lprcvd, const Q_UINT32 lpsent, const Q_UINT64 lpsize)
{
	kdDebug() << k_funcinfo << "enter" << endl;
	// Send a no acknowledge timeout control packet.
	sendControlPacket(Packet::TimeoutType, destination, lprcvd, lpsent, lpsize);
	kdDebug() << k_funcinfo << "leave" << endl;
}

void Transport::sendControlPacket(const Packet::Type type, const Q_UINT32 destination, const Q_UINT32 lprcvd, const Q_UINT32 lpsent, const Q_UINT64 lpsize)
{
	kdDebug() << k_funcinfo << "enter" << endl;

	Packet *packet = new Packet();

	// Get the header field of the packet.
	Packet::Header & h = packet->header();
	// Set the destination of the packet.
	h.destination = destination;
	// Set the sequence number of the packet.
	h.identifier = nextPacketSequenceNumber();
	// Set the data window size offset of the packet.
	h.offset = 0;
	// Set the data window size of the packet.
	h.window = 0;
	// Set the payload size of the packet.
	h.payloadSize = h.window;
	// Set the type of the packet.
	h.type = (Q_UINT32)type;
	// Set the correlation info of the packet.
	h.lprcvd = lprcvd;
	h.lpsent = lpsent;
	h.lpsize = lpsize;

	// Queue the control packet.
	queuePacket(packet, d->currentBridgeId/*, true*/);

	kdDebug() << k_funcinfo << "leave" << endl;
}

//END

//BEGIN Transport Transmission Control Event Handling Functions

void Transport::onNonAcknowledgeControlPacketReceive(Packet *packet)
{
	kdDebug() << k_funcinfo << "enter" << endl;

	if (packet == 0l)
	{
		kdDebug() << k_funcinfo << "Parameter \'packet\' cannot be null."  << endl;
		return;
	}

	// Get the header field of the packet.
	const Packet::Header & h = packet->header();

	switch(h.type)
	{
		case Packet::CancelType:
		{
			kdDebug() << k_funcinfo << "Cancel data receive for session " << h.identifier << endl;
			break;
		}

		case Packet::TimeoutType:
		{
			kdDebug() << k_funcinfo << "Received NAK for packet " << h.lpsent << endl;
			break;
		}

		case Packet::FaultType:
		{
			kdDebug() << k_funcinfo << "Transport layer error" << endl;
			break;
		}

		case Packet::ResetType:
		{
			kdDebug() << k_funcinfo << "Transport layer error, session = "
				<< h.destination << endl;
			break;
		}
	}

	kdDebug() << k_funcinfo << "leave" << endl;
}

//END

//BEGIN Session Notifier Functions

void Transport::registerPort(const Q_UINT32 port, SessionNotifier* notifier)
{
	if (!d->notifiers.contains(port))
	{
		d->notifiers.insert(port, notifier);
	}
}

void Transport::unregisterPort(const Q_UINT32 port)
{
	if (d->notifiers.contains(port))
	{
		d->notifiers.remove(port);
	}
}

//END

void Transport::reassembleData(Packet *packet, QByteArray& data)
{
	kdDebug() << k_funcinfo << "enter" << endl;

	QBuffer *buffer = 0l;
	// Get the header field of the pakcet.
	const Packet::Header & h = packet->header();

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
		QByteArray bytes = static_cast<QBuffer*>(packet->payload())->buffer();
		// Write the data into the buffer.
		buffer->writeBlock(bytes.data(), bytes.size());

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

void Transport::addPacketToUnacknowledgedList(Packet *packet)
{
	kdDebug() << k_funcinfo << "enter" << endl;

	if (packet == 0l)
	{
		kdDebug() << k_funcinfo << "Parameter \'packet\' cannot be null."  << endl;
		return;
	}

	kdDebug() << k_funcinfo << "Packet id "
		<< packet->header().identifier << endl;
	delete packet;

	kdDebug() << k_funcinfo << "leave" << endl;
}

void Transport::removePacketFromUnacknowledgedList(const Q_UINT32 packetId)
{
	kdDebug() << k_funcinfo << "enter" << endl;
	kdDebug() << k_funcinfo << "Packet id " << packetId << endl;
	kdDebug() << k_funcinfo << "leave" << endl;
}

//BEGIN Transport Timer Event Handling Functions
//END

}

#include "transport.moc"
