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

Packet* BinaryPacketFormatter::deserialize(QDataStream* stream)
{
	Packet *packet = 0l;

	if (stream == 0l)
	{
		kdDebug() << "Parameter \'stream\' cannot be null." << endl;
		return packet;
	}

	packet = new Packet();
	// Ensure that the byte order of the stream is little endian.
	stream->setByteOrder(QDataStream::LittleEndian);

	Packet::Header & h = packet->header();
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
		QByteArray buffer(h.payloadSize);
		// Read the packet payload bytes from the stream.
		stream->readRawBytes(buffer.data(), buffer.size());
		// Get the packet payload.
		QIODevice *payload = packet->payload();
		// Write the bytes to the packet payload.
		payload->writeBlock(buffer.data(), buffer.size());
		// Seek to the beginning of the payload.
		payload->at(0);
	}

	return packet;
}

void BinaryPacketFormatter::serialize(QDataStream* stream, Packet* packet)
{
	if (stream == 0l)
	{
		kdDebug() << "Parameter \'stream\' cannot be null." << endl;
		return;
	}

	if (packet == 0l)
	{
		kdDebug() << "Parameter \'packet\' cannot be null." << endl;
		return;
	}

	// Ensure that the byte order of the stream is little endian.
	stream->setByteOrder(QDataStream::LittleEndian);

	const Packet::Header & h = packet->header();
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
		// Get the packet payload.
		QIODevice *payload = packet->payload();
		if (payload != 0l)
		{
			// Seek to the beginning of the payload bytes.
			payload->at(0);
			QByteArray buffer(h.payloadSize);
			// Read the packet payload bytes into the buffer.
			payload->readBlock(buffer.data(), buffer.size());
			// Write the packet payload bytes to the stream.
			stream->writeRawBytes(buffer.data(), buffer.size());
		}
	}
}

}
