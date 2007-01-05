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
		enum TransportBridgeState { Created, Connected, Disconnecting, Disconnected, Faulted };

	public:
		/** @brief Creates a new instance of the class Transport Bridge. */
		TransportBridge(QObject *parent);
		virtual ~TransportBridge();

		virtual const Q_UINT32 identifier() const = 0;
		/** @brief When overriden in a derived class, returns the MTU for the transport bridge. */
		virtual const Q_UINT32 maxSendBufferSize() = 0;
		const TransportBridgeState & state() const;

		void connect();
		void disconnect();

	protected:
		virtual void onConnect();
		virtual void onDisconnect();
		void setState(const TransportBridgeState& state);

	signals:
		void connected();
		void disconnected();
		void error();
		void packetReceived(const Packet& packet);
		void timeout();

	private:
		class TransportBridgePrivate;
		TransportBridgePrivate *d;

}; // Transport
}

#endif
