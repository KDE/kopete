/*
    tcptransportbridge.h - Peer to Peer Tcp Transport Bridge

    Copyright (c) 2006 by Gregg Edghill     <gregg.edghill@gmail.com>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; version 2 of the License.               *
    *                                                                       *
    *************************************************************************
*/

#ifndef CLASS_P2P__TCPTRANSPORTBRIDGE_H
#define CLASS_P2P__TCPTRANSPORTBRIDGE_H

#include "transportbridge.h"
#include <qvaluelist.h>

namespace PeerToPeer
{

/**
 * @brief Represents a transport layer bridge used to send and received data.
 * The tcp transport bridge connects the transport layers using TCP as the communication protocol.
 *
 * @author Gregg Edghill <gregg.edghill@gmail.com>
 */
class TcpTransportBridge : public TransportBridge
{
	Q_OBJECT

	public:
		/** @brief Creates a new instance of the TcpBridge class. */
		TcpTransportBridge(const QValueList<QString>& addresses, const Q_UINT16 port, QObject *parent);
		virtual ~TcpTransportBridge();

		virtual const Q_UINT32 identifier() const;
		/** @brief Gets the maximum size of a message that can be sent or received by the bridge. */
		virtual const Q_UINT32 maxSendBufferSize();
		void doUpnpPortMappingIfNecessary();
		bool listen();
		/** @brief Sends a packet to the remote peer. */
		void send(const Packet& packet);

	protected:
		virtual void onConnect();
		virtual void onDisconnect();

	private slots:
		void onSocketAccept();
		void onListenEndpointError(int errorCode);
		void onSocketClosed();
		void onSocketConnected();
		void onSocketConnectTimeout();
		void onSocketError(int errorCode);
		void onSocketRead();

	private:
		class TcpTransportBridgePrivate;
		TcpTransportBridgePrivate *d;

}; // TcpTransportBridge
}

#endif
