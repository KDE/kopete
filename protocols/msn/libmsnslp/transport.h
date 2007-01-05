/*
    transport.h - Peer to Peer Transport class

    Copyright (c) 2006 by Gregg Edghill     <gregg.edghill@gmail.com>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; version 2 of the License.               *
    *                                                                       *
    *************************************************************************
*/

#ifndef CLASS_P2P__TRANSPORT_H
#define CLASS_P2P__TRANSPORT_H

#include <qobject.h>
#include <qcstring.h>
#include <qthread.h>
#include "packet.h"

namespace PeerToPeer
{

class SessionNotifier;
class SwitchboardBridge;

/**
 * @brief Represents a transport layer implementation used to send and receive data.
 *
 * @author Gregg Edghill <gregg.edghill@gmail.com>
 */
class Transport : public QObject, public QThread
{
	Q_OBJECT

	public :
		/** @brief Creates a new instance of the Transport class. */
		Transport(QObject *parent);
		virtual ~Transport();

		bool listen(const QString& address, const Q_UINT16 port);
		void registerReceiver(Q_UINT32 port, SessionNotifier* receiver);
		const Q_UINT32 createBridge(const QString& address, const Q_UINT16 port);
		void setSwitchboardBridge(SwitchboardBridge* bridge);
		void unregisterReceiver(Q_UINT32 port);
		void queuePacket(const Packet& packet, const Q_UINT32 bridgeId);
		void sendAcknowledge(const Q_UINT32 destination, const Q_UINT32 identifier, const Q_UINT32 relatesTo, const Q_UINT32 priority=1);
		void sendBytes(const QByteArray& bytes, const Q_UINT32 destination, const Q_UINT32 identifier, const Q_UINT32 relatesTo, const Q_UINT32 priority=1);
		void sendDatagram(const QByteArray& datagram, const Q_UINT32 destination, const Q_UINT32 identifier, const Q_UINT32 relatesTo, const Q_UINT32 priority=1);

	protected:
		virtual void run();

	signals:
		void bridgeConnected(const Q_UINT32 identifier);

	public slots:
		void onPacketSent(const Q_UINT32 identifier, const bool acknowledged);

	private slots:
		void onBridgeConnected();
		void onBridgeDisconnected();
		void onSwitchboardReadyToSend();
		void onPacketReceived(const Packet& packet);

	private:
		bool findOrCreateDatagram(const Packet& packet, QByteArray& datagram);
		void reassembleDatagram(const Packet& packet, QByteArray& datagram);
		bool tryDispatchDatagram(const QByteArray& datagram, const Q_UINT32 identifier, const Q_UINT32 relatesTo, const Q_UINT32 destination);

	private:
		class TransportPrivate;
		TransportPrivate *d;

}; // Transport
}

#endif
