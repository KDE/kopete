/*
    switchboardbridge.h - Peer to Peer Switchboard Bridge

    Copyright (c) 2006 by Gregg Edghill     <gregg.edghill@gmail.com>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; version 2 of the License.               *
    *                                                                       *
    *************************************************************************
*/

#ifndef CLASS_P2P__SWITCHBROADBRIDGE_H
#define CLASS_P2P__SWITCHBROADBRIDGE_H

#include "transportbridge.h"

class MSNChatSession;

namespace PeerToPeer
{

/**
 * @brief Represents a transport layer bridge used to send and received data.
 * The switchboard bridge connects the transport layers via the switchboard network.
 *
 * @author Gregg Edghill <gregg.edghill@gmail.com>
 */
class SwitchboardBridge : public TransportBridge
{
	Q_OBJECT

	public :
		/** @brief Creates a new instance of the SwitchboardBridge class. */
		SwitchboardBridge(MSNChatSession* via, QObject *parent);
		virtual ~SwitchboardBridge();
		/** @brief Sends a packet to the remote peer. */
		void send(const Packet& packet, const Q_UINT32 appId);
		bool isReadyToSend() const;
		virtual const Q_UINT32 identifier() const;
		/** @brief Gets the maximum send buffer size. */
		virtual const Q_UINT32 maxSendBufferSize();

	signals:
		void readyToSend();
		void requestSwitchboard();

	protected:
		virtual void onConnect();
		virtual void onDisconnect();

	private slots:
		void onDataReceived(const QByteArray& data);
		void onSend();

	private:
		bool requestSwitchboardIfNecessary();
		void sendViaNetwork(const QByteArray& bytes);

	private:
		class SwitchboardBridgePrivate;
		SwitchboardBridgePrivate *d;

}; // SwitchboardBridge
}

#endif
