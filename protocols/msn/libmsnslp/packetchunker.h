/*
   packetchunker.h - PeerToPeer Transport Layer Packet Chunker class.

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

#ifndef CLASS_P2P__PACKETCHUNKER_H
#define CLASS_P2P__PACKETCHUNKER_H

#include <qglobal.h>

namespace PeerToPeer
{
class Packet;

/**
 * @brief Represents a transport layer packet chunker.
 *
 * @author Gregg Edghill <gregg.edghill@gmail.com>
 */
class PacketChunker
{
	public:
		static Packet * getNextChunk(Packet *packet, const Q_UINT32 chunkSize);

	private:
		PacketChunker();

}; //PacketChunker
}

#endif
