/*
   packetqueue.cpp - PeerToPeer Transport Layer Packet Queue class.

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

#include "packetqueue.h"
#include <qmap.h>
#include <qmutex.h>
#include <stdlib.h>
#include <kdebug.h>

namespace PeerToPeer
{

class PacketQueue::PacketQueuePrivate
{
	public:
		Q_UINT32 chunkSize;
		QMutex queueLock;
		QMap<Q_UINT32, QValueList<Packet>* > queues;
};

PacketQueue::PacketQueue(const Q_UINT32 chunkSize, QObject *parent) : QObject(parent), d(new PacketQueuePrivate())
{
	d->chunkSize = chunkSize;
}

PacketQueue::~PacketQueue()
{
	delete d;
	d = 0l;
}

const Packet PacketQueue::dequeue(const Q_UINT32 chunkSize) const
{
	QMutexLocker locker(&d->queueLock);

	Q_UINT32 count = d->queues.size();
	QValueList<Packet>::Iterator iterators[count];
	for (Q_UINT32 i=0; i < count; ++i)
	{
		iterators[i] = d->queues[d->queues.keys()[i]]->begin();
	}

	Q_UINT32 j = 1;
	Q_UINT32 selected = 0;
	QValueList<Packet>::Iterator i = iterators[selected];
	while(j < count)
	{
		if ((*i).priority() == (*iterators[j]).priority() &&
			(*i).header().type < (*iterators[j]).header().type)
		{
			selected = j;
			i = iterators[j];
		}
		else
		if ((*i).priority() > (*iterators[j]).priority())
		{
			selected = j;
			i = iterators[j];
		}
		else
		if ((*i).header().identifier > (*iterators[j]).header().identifier)
		{
			selected = j;
			i = iterators[j];
		}

		++j;
	}

	const Q_UINT32 key = d->queues.keys()[selected];
	Packet & packet = *i;

	if (packet.header().window > chunkSize)
	{
		// If the payload size of the packet exceeds
		// the maximum transmission unit value set
		// for the queue, fragment the packet payload.
		Packet fragment;
		Packet::Header & h = fragment.header();
		// Copy the packet header field to the packet fragment.
		h.destination = packet.header().destination;
		h.identifier = packet.header().identifier;
		h.window = packet.header().window;
		h.type = packet.header().type;
		h.lprcvd = packet.header().lprcvd;
		h.lpsent = packet.header().lpsent;
		h.lpsize = packet.header().lpsize;

		const Q_UINT32 count = packet.header().window - packet.header().offset;
		// Calculate the payload size of the fragment.
		const Q_UINT32 length = QMIN(chunkSize, count);
		// Set the fragment's payload size
		h.payloadSize = length;
		h.offset = packet.header().offset;
		// Copy 'length' bytes of the packet payload
		// to the packet fragment.
		packet.payload().at(h.offset);
		QByteArray bytes(length);
		packet.payload().readBlock(bytes.data(), length);
		fragment.payload().writeBlock(bytes);
		// Update the data offset
		packet.header().offset += length;

		if (h.offset + length == h.window)
		{
			// If this is the last fragment,
			// remove it from the queue.
			d->queues[key]->remove(i);
			if (d->queues[key]->isEmpty())
			{
				// If the queue is empty, remove it.
				delete d->queues[key];
				d->queues.remove(key);
			}
		}

		return fragment;
	}
	else
	{
		// Otherwise, return the unfragmented packet.
		const Packet p = *i;
		// Remove the packet from the queue.
		d->queues[key]->remove(i);
		if (d->queues[key]->isEmpty())
		{
			// If the queue is empty, remove it.
			delete d->queues[key];
			d->queues.remove(key);
		}
		return p;
	}
}

const bool PacketQueue::dequeue(Packet & outPacket)
{
	return false;
}

void PacketQueue::enqueue(const Packet & packet) const
{
	QMutexLocker locker(&d->queueLock);
	if (!d->queues.contains(packet.header().destination))
	{
		d->queues.insert(packet.header().destination, new QValueList<Packet>());
	}

	d->queues[packet.header().destination]->append(packet);
}

const bool PacketQueue::isEmpty() const
{
	QMutexLocker locker(&d->queueLock);
	return d->queues.isEmpty();
}

}

#include "packetqueue.moc"
