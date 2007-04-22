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

#include <qiodevice.h>
#include <qstring.h>

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
			// Indicates message data.  Data should be buffered
			// until all data is received before delivering to the
			// session client.
			MessageDataType = 0x00,
			// Indicates that the last message was received.
			AcknowledgeType = 0x02,
			// Indicates that a timeout has occurred at the receiver
			// due to the absence of a response ACK from the sender.
			TimeoutType = 0x04,
			// Indicates that an error has occurred.
			FaultType = 0x08,
			// Indicates PUSHed object data.  Data should be delivered
			// to the application upon arrival and not buffered until
			// all data is received.
			ObjectDataType = 0x20,
			// Indicates that the sender cancelled the data exchange.
			CancelType = 0x40,
			// Used to ungracefully end a data exchange.
			ResetType = 0x80,
			// Indicates a direct connection handshake nonce.
			HandshakeNonceType = 0x100,
			// Indicates PUSHed file data.  Data should be delivered
			// to the application upon arrival and not buffered until
			// all data is received.
			FileDataType = 0x1000000 | 0x30
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
		/** @brief Creates a new instance of the class Packet. */
		Packet();
		/** @brief Finalizer. */
		~Packet();

		/** @brief Gets the packer header. */
		Header & header() const;
		/** @brief Gets the packet payload. */
		QIODevice* payload() const;
		/** @brief Sets the packet payload. */
		void setPayload(QIODevice* payload);
		/** @brief Gets a value that indicates the size of the packet. */
		const Q_UINT32 size() const;
		/** @brief Returns a string representation of the packet. */
		const QString toString() const;

	private:
		class PacketPrivate;
		PacketPrivate *d;

}; // Packet
}

#endif
