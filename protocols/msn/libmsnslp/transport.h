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
#include <quuid.h>
#include <qvaluelist.h>
#include "packet.h"

class QBuffer;

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
		void registerPort(Q_UINT32 port, SessionNotifier* notifier);
		const Q_UINT32 createBridge(const QValueList<QString>& addresses, const Q_UINT16 port, const QUuid& nonce);
		void setSwitchboardBridge(SwitchboardBridge* bridge);
		void unregisterPort(Q_UINT32 port);
		void queuePacket(const Packet& packet, const Q_UINT32 bridgeId, bool prepend=false);
		Q_UINT32 send(const QByteArray& message, const Q_UINT32 destination, const Q_UINT32 relatesTo, const Q_UINT32 priority=1);
		void sendBytes(const QByteArray& bytes, const Q_UINT32 destination, const Q_UINT32 relatesTo, const Q_UINT32 priority=1);

	protected:
		virtual void run();

	private slots:
		void onBridgeConnected();
		void onBridgeDisconnected();
		void onBridgeError();
		void onNonAcknowledgeControlPacketReceived(const Packet& packet);
		void onNonceReceived(const QUuid& cnonce, const Q_UINT32 bridgeId);
		void onReceive(const Packet& packet);
		void onSend(const Packet& packet, const Q_UINT32 bridgeId);
		void onSendFailed(const Packet& packet);
		void onSent(const Q_UINT32 identifier, const bool packetSent);
		void onSent(const Packet& packet);
		void onSwitchboardReadyToSend();
		void onUnacknowledgedPacketTimer();

	private:
		void dispatch(const QByteArray& message, const Q_UINT32 destination, const Q_UINT32 identifier, const Q_UINT32 relatesTo);
		Q_UINT32 nextPacketSequenceNumber() const;
		void reassembleData(const Packet& packet, QByteArray& data);
		void sendAcknowledge(const Q_UINT32 destination, const Q_UINT32 lprcvd);
		void sendControlPacket(const Packet::Type type, const Q_UINT32 destination, const Q_UINT32 lprcvd, const Q_UINT32 lpsent, const Q_UINT64 lpsize);
		void sendNonce(const QUuid& nonce, const Q_UINT32 bridgeId);
		void removeUnacknowledgedPacket(const Q_UINT32 identifier);

	private:
		class TransportPrivate;
		TransportPrivate *d;

}; // Transport
}

#endif
