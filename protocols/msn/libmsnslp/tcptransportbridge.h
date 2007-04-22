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

#include "directtransportbridge.h"
#include <qvaluelist.h>

namespace PeerToPeer
{

/**
 * @brief Represents a transport layer bridge used to send and received data.
 * The tcp transport bridge connects the transport layers using TCP as the communication protocol.
 *
 * @author Gregg Edghill <gregg.edghill@gmail.com>
 */
class TcpTransportBridge : public DirectTransportBridge
{
	Q_OBJECT

	public:
		/** @brief Creates a new instance of the TcpBridge class. */
		TcpTransportBridge(const QValueList<QString>& addresses, const Q_UINT16 port, const Q_UINT32 bridgeId, QObject *parent);
		virtual ~TcpTransportBridge();

		/** @brief Gets a value that uniquely identifies the transport bridge. */
		virtual Q_UINT32 id() const;
		/** @brief Gets the MTU for the transport bridge. */
		virtual Q_UINT32 maxSendBufferSize() const;

		void doUpnpPortMappingIfNecessary();
		bool listen();
		/** @brief Sends the specified byte array. */
		virtual void send(const QByteArray& bytes, const Q_UINT32 id);

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
