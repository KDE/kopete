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
		SwitchboardBridge(const Q_UINT32 bridgeId, QObject *parent);
		virtual ~SwitchboardBridge();

		/** @brief Gets a value that uniquely identifies the transport bridge. */
		virtual Q_UINT32 id() const;
		/** @brief Gets the MTU for the transport bridge. */
		virtual Q_UINT32 maxSendBufferSize() const;
		/** @brief Attachs the switchboard bridge to the switchboard network. */
		void connectTo(MSNChatSession *switchboard);
		/** @brief Sends the specified byte array. */
		virtual void send(const QByteArray& bytes, const Q_UINT32 id);

	public slots:
		void onDataReceived(const QByteArray& bytes);
		void onSend(const Q_INT32 id);

	private slots:
		void onSendPendingPackets();

	protected:
		virtual void onConnect();
		virtual void onDisconnect();

	private:
		bool trySendViaSwitchboard(const QByteArray& bytes, Q_UINT32& cookie);

	private:
		class SwitchboardBridgePrivate;
		SwitchboardBridgePrivate *d;

}; // SwitchboardBridge
}

#endif
