/*
    packet.h - Peer to Peer Transport Layer Packet class

    Copyright (c) 2006 by Gregg Edghill     <gregg.edghill@gmail.com>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; version 2 of the License.               *
    *                                                                       *
    *************************************************************************
*/

#ifndef CLASS_P2P__PACKET_H
#define CLASS_P2P__PACKET_H

#include <qbuffer.h>

namespace PeerToPeer
{

/**
 * @brief Represents a transport layer packet.
 *
 * @author Gregg Edghill <gregg.edghill@gmail.com>
 */
class Packet
{
	public:
		/** @brief Represents the types of packets that can be sent via a transport. */
		enum Type
		{
			// Indicates a message.  Data should be buffered
			// until all data is received before delivering to the
			// application.
			MessageType = 0x00,
			// Indicates that the last message was received.
			AcknowledgeType = 0x02,
			// Indicates that a timeout has occurred at the receiver
			// due to the absence of a response from the sender.
			TimeoutType = 0x04,
			// Indicates that an error has occurred.
			FaultType = 0x08,
			// Indicates PUSHed object data.  Data should be delivered
			// to the application upon arrival and not buffered until
			// all data is received.
			ObjectDataType = 0x20,
			// Indicates that the sender has no more data to send.
			EndOfDataType = 0x40,
			// Used to ungracefully close communication.
			ResetType = 0x80,
			// Indicates a direct connection nonce handshake.
			HandshakeType = 0x100,
			// Indicates PUSHed file data.  Data should be delivered
			// to the application upon arrival and not buffered until
			// all data is received.
			FileDataType = 0x1000030
		};

		/** @brief Represents the content of a packet header. */
		struct Header
		{
			// Indicates the destination port.
			Q_UINT32 destination;
			// Identifies a packet or a series of packet fragments
			Q_UINT32 identifier;
			// Indicates the data offset of a packet fragment.
			Q_UINT64 offset;
			// Indicates the packet window size.
			Q_UINT64 window;
			// Indicates the size of the payload.
			Q_UINT32 payloadSize;
			// Indicates the packet type.
			Q_UINT32 type;
			// Indicates the identifier of the last
			// packet received by the sender.
			Q_UINT32 lprcvd;
			// Indicates the identifier of the last
			// packet sent by by the sender.
			Q_UINT32 lpsent;
			// Indicates the data size of the last
			// packet received by the sender.
			Q_UINT64 lpsize;
		};

	public:
		Packet();
		Packet(const Packet& other);
		Packet & operator=(const Packet& other);
		~Packet();

		/** @brief Gets the packer header. */
		Header & header() const;
		/** @brief Gets the packet payload data. */
		const QBuffer & payload() const;
		QBuffer & payload();
		const Q_UINT32 priority() const;
		void setPriority(const Q_UINT32 priority) const;
		/** @brief Gets a value that indicates the size of the packet. */
		const Q_UINT32 size() const;
		bool operator==(const Packet& other);
		const QString toString() const;

	private:
		class PacketPrivate;
		PacketPrivate *d;

		friend class Transport;

}; // Packet
}

#endif
