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
#include <qmap.h>
#include <qvariant.h>

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

	public:
		/** @brief Creates a new instance of the class Transport Bridge. */
		TransportBridge(QObject *parent);
		virtual ~TransportBridge();
		/** @brief When overriden in a derived class, gets the properties of the transport bridge. */
		virtual const QMap<QString, QVariant> & getProperties() const = 0;
		/** @brief When overriden in a derived class, returns a value that uniquely identifies the transport bridge. */
		virtual Q_UINT32 id() const = 0;
		/** @brief Gets the state of the transport bridge. */
		const TransportBridgeState & state() const;
		/** @brief Connects a transport bridge. */
		void connect();
		/** @brief Disconnects a transport bridge. */
		void disconnect();
		/** @brief When overriden in a derived class, sends the specified byte array. */
		virtual void send(const QByteArray& bytes, const Q_UINT32 id) = 0;

	protected:
		virtual void onConnect() = 0;
		virtual void onDisconnect() = 0;
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
		void dataReceived(const QByteArray& bytes);
		/** @brief Indicates that a packet has been sent on the transport bridge. */
		void dataSent(const Q_UINT32 packetId);
		/** @brief Indicates that a data exchange time out has occurred on the transport bridge. */
		void timeout();

	private:
		class TransportBridgePrivate;
		TransportBridgePrivate *d;

}; // TransportBridge
}

#endif
