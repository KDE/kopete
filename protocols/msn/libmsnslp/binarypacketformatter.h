/*
   binarypacketformatter.h - PeerToPeer Transport Layer Binary Packet Formatter class.

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

#ifndef CLASS_P2P__BINARYPACKETFORMATTER_H
#define CLASS_P2P__BINARYPACKETFORMATTER_H

#include <qdatastream.h>
#include "packet.h"

namespace PeerToPeer
{

/**
 * @brief Represents a transport layer binary packet formatter.
 *
 * @author Gregg Edghill <gregg.edghill@gmail.com>
 */
class BinaryPacketFormatter
{
	public:
		static Packet deserialize(QDataStream* stream);
		static void serialize(const Packet& packet, QDataStream* stream);

	private:
		BinaryPacketFormatter();

}; // BinaryPacketFormatter
}

#endif
