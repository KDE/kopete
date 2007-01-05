/*
   packetqueue.h - PeerToPeer Transport Layer Packet Queue class.

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

#ifndef CLASS_P2P__PACKETQUEUE_H
#define CLASS_P2P__PACKETQUEUE_H

#include <qobject.h>
#include <qvaluelist.h>
#include "packet.h"

namespace PeerToPeer
{

/**
 * @brief Represents a transport layer packet queue.
 *
 * @author Gregg Edghill <gregg.edghill@gmail.com>
 */
class PacketQueue : public QObject
{
	Q_OBJECT

	public:
		PacketQueue(const Q_UINT32 chunkSize, QObject *parent);
		~PacketQueue();

		const Packet dequeue(const Q_UINT32 chunkSize) const;
		const bool dequeue(Packet & outPacket);
		void enqueue(const Packet & packet) const;
		const bool isEmpty() const;

	signals:
		void readyRead();

	private:
		class PacketQueuePrivate;
		PacketQueuePrivate *d;

		friend class Transport;

}; // PacketQueue
}

#endif
