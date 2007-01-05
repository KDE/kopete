/*
    packetscheduler.h - Peer to Peer Transport Packet Scheduler class

    Copyright (c) 2006 by Gregg Edghill     <gregg.edghill@gmail.com>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; version 2 of the License.               *
    *                                                                       *
    *************************************************************************
*/

#ifndef CLASS_P2P__PACKETSCHEDULER_H
#define CLASS_P2P__PACKETSCHEDULER_H

#include <qobject.h>
#include <qthread.h>
#include "packet.h"
#include "packetqueue.h"

namespace PeerToPeer
{

/** @brief Manages packet scheduling for the transport layer.
 *
 * @author Gregg Edghill <gregg.edghill@gmail.com>
 */
class PacketScheduler : public QObject, public QThread
{
	Q_OBJECT

	public:
		/** @brief Creates a new instance of the PacketScheduler class. */
		PacketScheduler(const Q_UINT32 identifier, const Q_UINT16 chunkSize, PacketQueue *queue, QObject *parent);
		virtual ~PacketScheduler();

	protected:
		virtual void run();

	signals:
		void schedulePacket(const Packet& packet, const Q_UINT32 bridgeId);

	private slots:
		void onPacketQueueAccessible();

	private:
		class PacketSchedulerPrivate;
		PacketSchedulerPrivate *d;

}; // PacketScheduler
}

#endif
