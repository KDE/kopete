/*
   packetscheduler.h - PeerToPeer Transport Layer Packet Scheduler class.

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

#ifndef CLASS_P2P__PACKETSCHEDULER_H
#define CLASS_P2P__PACKETSCHEDULER_H

#include <qobject.h>
#include <qptrlist.h>
#include <qvaluelist.h>

namespace PeerToPeer
{

class Packet;
typedef QPtrList<Packet> PacketList;
class Transport;

/**
 * @brief Represents a transport layer packet scheduler
 * which manages the scheduling of different packets based
 * based on packet type and priority.
 *
 * @author Gregg Edghill <gregg.edghill@gmail.com>
 */
class PacketScheduler: public QObject
{
	Q_OBJECT

	public:
		/** @brief Creates a new instance of the class PacketScheduler. */
		PacketScheduler(Transport *transport);
		/** @brief Finalizer. */
		~PacketScheduler();

		/** @brief Indicates whether the scheduler is running. */
		bool isRunning() const;
		/** @brief Starts the packet scheduler. */
		void start();
		/** @brief Stops the packet scheduler. */
		void stop();

	private slots:
		/** @brief Occurs when the scheduler's timer event is called. */
		void onSchedulePacketsToSend();

	private:
		/** @brief Gets the next packet that is ready to be sent. */
		Packet * getNextPacket(PacketList *list);
		/** @brief Gets a list of packets from the supplied packet list
		  * that match the specified packet class.
		  */
		PacketList selectPacketsByClass(const QValueList<Q_UINT32> & packetClass, PacketList* list);

	private:
		class PacketSchedulerPrivate;
		PacketSchedulerPrivate *d;

}; //PacketScheduler
}

#endif
