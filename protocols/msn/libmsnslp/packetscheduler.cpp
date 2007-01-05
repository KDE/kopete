/*
    packetscheduler.cpp - Peer to Peer Transport Packet Scheduler class

    Copyright (c) 2006 by Gregg Edghill     <gregg.edghill@gmail.com>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; version 2 of the License.               *
    *                                                                       *
    *************************************************************************
*/

#include "packetscheduler.h"
#include "qmutex.h"
#include "qwaitcondition.h"
#include <kdebug.h>

namespace PeerToPeer
{

class PacketScheduler::PacketSchedulerPrivate
{
	public:
		Q_UINT16 chunkSize;
		Q_UINT32 identifier;
		PacketQueue *queue;
		QWaitCondition queueAccessible;
		bool stopScheduling;
};

PacketScheduler::PacketScheduler(const Q_UINT32 identifier, const Q_UINT16 chunkSize, PacketQueue *queue, QObject *parent) : QObject(parent), QThread(), d(new PacketSchedulerPrivate())
{
	d->identifier = identifier;
	d->chunkSize = chunkSize;
	d->queue = queue;
	// Connect the signal/slot
	QObject::connect(d->queue, SIGNAL(readyRead()), this,
	SLOT(onPacketQueueAccessible()));
	d->stopScheduling = false;
}

PacketScheduler::~PacketScheduler()
{
	d->stopScheduling = true;
	wait();

	delete d;
	d = 0l;
}

void PacketScheduler::run()
{
	while (!d->stopScheduling)
	{
		if (!d->queue->isEmpty())
		{
			Packet packet;
			bool inaccessible = d->queue->dequeue(packet);
			if (inaccessible)
			{
				// If the queue is inaccessible, wait for it to be accessible.
				d->queueAccessible.wait();
				continue;
			}

			// Schedule the packet on the assigned transport.
			emit schedulePacket(packet, d->identifier);
		}
	}
}

void PacketScheduler::onPacketQueueAccessible()
{
	d->queueAccessible.wakeOne();
}

}

#include "packetscheduler.moc"
