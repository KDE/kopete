/*
    transportbridge.h - Peer to Peer Transport Bridge class

    Copyright (c) 2006 by Gregg Edghill     <gregg.edghill@gmail.com>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; version 2 of the License.               *
    *                                                                       *
    *************************************************************************
*/

#ifndef CLASS_P2P__TRANSPORTBRIDGE_H
#define CLASS_P2P__TRANSPORTBRIDGE_H

#include <qobject.h>
#include "packet.h"

namespace PeerToPeer
{

/**
 * @brief Represents a transport layer bridge used to send and received data between endpoints.
 *
 * @author Gregg Edghill <gregg.edghill@gmail.com>
 */
class TransportBridge : public QObject
{
	Q_OBJECT

	public:
		/** @brief Represents the possible states during a transport bridge's life cycle. */
		enum TransportBridgeState { Created, Connecting, Connected, Disconnecting, Disconnected, Faulted };
		/** @brief Defines the types of transport bridges. */
		enum TransportBridgeType { Direct, Indirect };

	public:
		/** @brief Creates a new instance of the class Transport Bridge. */
		TransportBridge(QObject *parent);
		virtual ~TransportBridge();
		/** @brief When overriden in a derived class, returns a value that uniquely identifies the transport bridge. */
		virtual const Q_UINT32 id() const = 0;
		/** @brief When overriden in a derived class, returns the MTU for the transport bridge. */
		virtual const Q_UINT32 maxSendBufferSize() = 0;
		/** @brief Gets the state of the transport bridge. */
		const TransportBridgeState & state() const;
		/** @brief Connects a transport bridge. */
		void connect();
		/** @brief Disconnects a transport bridge. */
		void disconnect();

	protected:
		virtual void onConnect();
		virtual void onDisconnect();
		/** @brief Sets the state of the transport bridge. */
		void setState(const TransportBridgeState& state);

	signals:
		/** @brief Indicates that the transport bridge is connected. */
		void connected();
		/** @brief Indicates that the transport bridge is disconnected. */
		void disconnected();
		/** @brief Indicates that an error has occurred on the transport bridge. */
		void error();
		/** @brief Indicates that a packet has been received on the transport bridge. */
		void packetReceived(const Packet& packet);
		/** @brief Indicates that a data exchange time out has occurred on the transport bridge. */
		void timeout();

	private:
		class TransportBridgePrivate;
		TransportBridgePrivate *d;

}; // TransportBridge
}

#endif
