/*
   binarypacketformatter.cpp - PeerToPeer Transport Layer Binary Packet Formatter class.

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

#include "binarypacketformatter.h"
#include <kdebug.h>

namespace PeerToPeer
{

BinaryPacketFormatter::BinaryPacketFormatter()
{
}

Packet BinaryPacketFormatter::deserialize(QDataStream* stream)
{
	if (stream == 0l)
	{
		kdDebug() << "Parameter \'stream\' cannot be null." << endl;
		return Packet();
	}

	// Ensure that the byte order of the stream is little endian.
	stream->setByteOrder(QDataStream::LittleEndian);

	Packet packet;
	Packet::Header & h = packet.header();
	// Read the packet header fields from the stream.
	*stream >> h.destination;
	*stream >> h.identifier;
	*stream >> h.offset;
	*stream >> h.window;
	*stream >> h.payloadSize;
	*stream >> h.type;
	*stream >> h.lprcvd;
	*stream >> h.lpsent;
	*stream >> h.lpsize;

	if (h.payloadSize > 0)
	{
		// Read the packet payload data from the stream.
		QByteArray bytes(h.payloadSize);
		stream->device()->readBlock(bytes.data(), h.payloadSize);
		// Write the data to the packet payload.
		packet.payload().writeBlock(bytes);
	}

	kdDebug() << packet.toString() << endl;

	return packet;
}

void BinaryPacketFormatter::serialize(const Packet& packet, QDataStream* stream)
{
	if (stream == 0l)
	{
		kdDebug() << "Parameter \'stream\' cannot be null." << endl;
		return;
	}

	// Ensure that the byte order of the stream is little endian.
	stream->setByteOrder(QDataStream::LittleEndian);

	const Packet::Header & h = packet.header();
	// Write the packet header fields to the stream.
	*stream << h.destination;
	*stream << h.identifier;
	*stream << h.offset;
	*stream << h.window;
	*stream << h.payloadSize;
	*stream << h.type;
	*stream << h.lprcvd;
	*stream << h.lpsent;
	*stream << h.lpsize;

	if (h.payloadSize > 0)
	{
		// Write the packet payload to the stream.
		stream->device()->writeBlock(packet.payload().buffer());
	}

	kdDebug() << packet.toString() << endl;
}

}
