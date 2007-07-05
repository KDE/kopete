/*
   packetscheduler.cpp - PeerToPeer Transport Layer Packet Scheduler class.

   Copyright (c) 2006 by Gregg Edghill <gregg.edghill@gmail.com>

   *************************************************************************
   *                                                                       *
   * This library is free software; you can redistribute it and/or         *
   * modify it under the terms of the GNU Lesser General Public            *
   * License as published by the Free Software Foundation; either          *
   * version 2 of the License, or (at your option) any later version.      *
   *                                                                       *
   *************************************************************************
*/

#include "packetscheduler.h"
#include "packet.h"
#include "packetchunker.h"
#include "transport.h"
#include "transportbridge.h"
#include <qmap.h>
#include <qtimer.h>
#include <kdebug.h>
#include <stdlib.h>

namespace PeerToPeer
{

class PacketScheduler::PacketSchedulerPrivate
{
	public:
		PacketSchedulerPrivate() : isRunning(false), maxPendingPackets(5) {}

		bool isRunning;
		Q_UINT32 maxPendingPackets;
		QMap<Q_UINT32, QValueList<Q_UINT32> > packetClasses;
		QTimer *timer;
		Transport *transport;
};

PacketScheduler::PacketScheduler(Transport *transport) : QObject(transport), d(new PacketSchedulerPrivate())
{
	d->transport = transport;

	d->timer = new QTimer(this);
	// Connect the signal/slot.
	QObject::connect(d->timer, SIGNAL(timeout()), this, SLOT(onSchedulePacketsToSend()));

	// Define the packet classes which are used to
	// determine the type of packet to send first.

	// Control
	QValueList<Q_UINT32> controlPacketClassTypes;
	controlPacketClassTypes.append(Packet::NonAcknowledgeType);
	controlPacketClassTypes.append(Packet::FaultType);
	controlPacketClassTypes.append(Packet::ResetType);
	controlPacketClassTypes.append(Packet::CancelType);
	controlPacketClassTypes.append(Packet::AcknowledgeType);
	controlPacketClassTypes.append(Packet::HandshakeNonceType);

	d->packetClasses[0] = controlPacketClassTypes;

	// Message Data
	QValueList<Q_UINT32> messageDataPacketClassTypes;
	messageDataPacketClassTypes.append(Packet::MessageDataType);

	d->packetClasses[1] = messageDataPacketClassTypes;

	// Raw Data
	QValueList<Q_UINT32> rawDataPacketClassTypes;
	rawDataPacketClassTypes.append(Packet::ObjectDataType);
	rawDataPacketClassTypes.append(Packet::FileDataType);

	d->packetClasses[2] = rawDataPacketClassTypes;
}

PacketScheduler::~PacketScheduler()
{
	delete d;
}

bool PacketScheduler::isRunning() const
{
	return d->isRunning;
}

void PacketScheduler::start()
{
	// Start the timer with a 1 sec interval.
	d->timer->start(1000);
	d->isRunning = true;
}

void PacketScheduler::stop()
{
	// Stop the timer.
	d->timer->stop();
	d->isRunning = false;
}

//BEGIN PacketScheduler Event Handling Functions

///////////////////////////////////////////////////////////////////////////////////
// The packet scheduler uses a priority scheduling model whereby, it gives priority
// to packets of a certain class and continues scheduling the packets in a FIFO
// manner until the list becomes empty.
///////////////////////////////////////////////////////////////////////////////////
void PacketScheduler::onSchedulePacketsToSend()
{
	kdDebug() << k_funcinfo << "enter" << endl;

	QMap<Q_UINT32, TransportBridge*>::ConstIterator bridge;
	// Get the transport layer bridges.
	const QMap<Q_UINT32, TransportBridge*> & bridges = d->transport->getBridges();
	// Get the transport layer packet lists.
	const QMap<Q_UINT32, PacketList*> & lists = d->transport->getPacketLists();

	for (bridge = bridges.begin(); bridge != bridges.end(); ++bridge)
	{
		// Get the packet list assigned to the bridge.
		PacketList *list = lists[bridge.key()];
		if (list->isEmpty())
		{
			kdDebug() << k_funcinfo << "No packets to send on bridge " << bridge.key() << endl;
			continue;
		}

		// Get the mtu of the current transport bridge.
		const Q_UINT32 maxDataChunkSize = (*bridge)->getProperties()["mtu"].toUInt();

		// If the packet list is not empty, try to send the packets.
		while(!list->isEmpty() && d->transport->getSentPackets().count() < d->maxPendingPackets)
		{
			// Get the next packet that is ready to be sent.
			Packet *packet = getNextPacket(list);

			// If the packet payload exceeds the mtu of the transport
			// bridge we have to fragment the payload into chunks.
			if (packet->header().window > maxDataChunkSize)
			{
				Packet *chunkedPacket = packet;
				// Get the next chunk of the packet to send.
				packet = PacketChunker::getNextChunk(chunkedPacket, maxDataChunkSize);
				// Check whether this is the last chunk of the chunked packet.
				if (packet->header().offset + packet->header().payloadSize == packet->header().window)
				{
					// If so, remove the packet from the list.
					const Q_INT32 pos = list->findRef(chunkedPacket);
					list->remove(pos);
					delete chunkedPacket;
				}
			}
			else
			{
				// Otherwise, we can send the single packet
				// which contains the entire payload; so,
				// remove the packet from the list.
				const Q_INT32 pos = list->findRef(packet);
				list->remove(pos);
			}

			// Send the packet via the specified transport bridge.
			d->transport->sendPacket(packet, bridge.key());
		}
	}

	kdDebug() << k_funcinfo << "leave" << endl;
}

//END

Packet * PacketScheduler::getNextPacket(PacketList *list)
{
	kdDebug() << k_funcinfo << "enter" << endl;

	Packet *packet = 0l;
	Q_UINT32 selectedPacketClass = 0;
	PacketList packets;

	const Q_UINT32 controlPacketClass = 0;
	const Q_UINT32 messageDataPacketClass = 1;

	// Try to select packets to schedule based on their classification.
	// Packet classification scheduling is as follows:
	// 1. control packets
	// 2. message data packets
	// 3. raw data packets
	for(Q_UINT32 i=0; i < 3; ++selectedPacketClass, ++i)
	{
		packets = selectPacketsByClass(d->packetClasses[i], list);
		if (packets.count() > 0)
		{
			break;
		}
	}

	kdDebug() << k_funcinfo << "scheduling packets of class: " << selectedPacketClass << endl;
	QPtrListIterator<Packet> it2(packets);
	while(it2.current() != 0l)
	{
		kdDebug() << k_funcinfo << "packet " << it2.current()->header().identifier
			<< " is ready to be sent" << endl;
		++it2;
	}

	// Get the number of packets that are ready to be scheduled.
	const Q_INT32 count = packets.count();
	if (count > 0)
	{
		if (selectedPacketClass == controlPacketClass ||
			selectedPacketClass == messageDataPacketClass)
		{
			// If the current packet class is control
			// or message data, get a packet from the
			// list.  Priority is given to session 0
			// packets.

			bool hasSession0Packets = false;
			QPtrListIterator<Packet> it(packets);
			while(it.current() != 0l)
			{
				if (it.current()->header().destination == 0)
				{
					// If we have found a session 0 packet,
					// select the packet.
					packet = it.current();
					hasSession0Packets = true;
					break;
				}

				++it;
			}

			if (hasSession0Packets == false)
			{
				// If no session 0 packets were found,
				// just get the first packet from the
				// list.
				packet = packets.first();
			}
		}
		else
		{
			// Otherwise, randomly select a raw data
			// packet from the list.
			Q_INT32 random = (Q_INT32)(((double)(rand() * 1.0/0x7FFFFFFF))*count);
			if (random == count)
			{
				random -= 1;
			}

			packet = packets.at(random);
		}
	}

	kdDebug() << k_funcinfo << "leave" << endl;
	return packet;
}

PacketList PacketScheduler::selectPacketsByClass(const QValueList<Q_UINT32> & packetClass, PacketList* list)
{
	PacketList packets;
	QPtrListIterator<Packet> it(*list);

	Packet *packet = 0l;
	while((packet = it.current()) != 0l)
	{
		if (packetClass.contains(packet->header().type))
		{
			// If the packet type matches the specified
			// packet class we are looking for, add the
			// packet to the list.
			packets.append(packet);
		}

		++it;
	}

	return packets;
}

}

#include "packetscheduler.moc"
