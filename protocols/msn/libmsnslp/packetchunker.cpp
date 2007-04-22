/*
   packetchunker.cpp - PeerToPeer Transport Layer Packet Chunker class.

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

#include "packetchunker.h"
#include "packet.h"
#include <kdebug.h>

namespace PeerToPeer
{

PacketChunker:: PacketChunker()
{
}

Packet* PacketChunker::getNextChunk(Packet *packet, const Q_UINT32 chunkSize)
{
	Packet *chunk = 0l;

	if (packet == 0l)
	{
		kdDebug() << k_funcinfo << "Parameter \'packet\' cannot be null."  << endl;
		return chunk;
	}

	const Q_UINT32 count = packet->header().window - packet->header().offset;
	// Calculate the payload size of the packet chunk.
	const Q_UINT32 length = QMIN(chunkSize, count);

	chunk = new Packet();
	// Get the packet header.
	Packet::Header & h = chunk->header();
	// Copy the packet header field to the packet chunk.
	h.destination = packet->header().destination;
	h.identifier = packet->header().identifier;
	h.offset = packet->header().offset;
	h.window = packet->header().window;
	h.payloadSize = length;
	h.type = packet->header().type;
	h.lprcvd = packet->header().lprcvd;
	h.lpsent = packet->header().lpsent;
	h.lpsize = packet->header().lpsize;

	//
	// Copy 'length' bytes of the packet payload to the packet chunk.
	//

	// Seek to the specified offset in the payload.
	packet->payload()->at(h.offset);
	QByteArray bytes(length);
	// Read the packet payload bytes into the buffer.
	packet->payload()->readBlock(bytes.data(), bytes.size());
	// Write the payload bytes to the chunk's payload.
	chunk->payload()->writeBlock(bytes.data(), bytes.size());
	// Update the data offset.
	packet->header().offset += length;

	return chunk;
}

}
