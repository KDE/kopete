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
#include <qptrlist.h>
#include <quuid.h>
#include <qvaluelist.h>
#include "packet.h"

class QBuffer;
class QFile;

namespace PeerToPeer
{

class SwitchboardBridge;
class TransportBridge;
class DirectTransportBridge;
typedef QPtrList<Packet> PacketList;
class SessionNotifier;

/**
 * @brief Represents a transport layer implementation that manages and controls
 * end to end packet transmission.
 *
 * @author Gregg Edghill <gregg.edghill@gmail.com>
 */
class Transport : public QObject
{
	Q_OBJECT

	public :
		/** @brief Creates a new instance of the Transport class. */
		Transport(QObject *parent);
		~Transport();

		DirectTransportBridge* createDirectBridge(const QString& type, const QValueList<QString>& addresses, const Q_UINT16 port, const QUuid& nonce);
		SwitchboardBridge* createIndirectBridge();
		bool listen(const QString& address, const Q_UINT16 port);
		bool isConnected() const;
		void registerPort(const Q_UINT32 port, SessionNotifier* notifier);
		Q_UINT32 send(const QByteArray& message, const Q_UINT32 destination, const Q_UINT32 correlationId);
		void sendBytes(const QByteArray& bytes, const Q_UINT32 destination, const Q_UINT32 correlationId);
		void sendFile(QFile *file, const Q_UINT32 destination);
		void unregisterPort(const Q_UINT32 port);

	private slots:
		void onBridgeConnect();
		void onBridgeDisconnect();
		void onBridgeError();
		void onBridgeHandshake(const QUuid& cnonce, const Q_UINT32 bridgeId);
		void onReceive(const QByteArray& bytes);
		void onSendFailed(Packet* packet);
		void onSent(const Q_UINT32 id);

	private:
		void addBridge(TransportBridge *bridge);
		void addPacketToUnacknowledgedList(Packet *packet);
		DirectTransportBridge* createDirectBridge(const QString& type, const QValueList<QString>& addresses, const Q_UINT16 port);
		Q_UINT32 findBestBridge() const;
		void movePacketsBetweenBridges(const Q_UINT32 oldBridgeId, const Q_UINT32 newBridgeId);
		Q_UINT32 nextBridgeId() const;
		Q_UINT32 nextPacketSequenceNumber() const;
		void queuePacket(Packet *packet, const Q_UINT32 bridgeId, bool prepend=false);
		void onNonAcknowledgeControlPacketReceive(Packet *packet);
		void reassembleData(Packet *packet, QByteArray& data);
		void removePacketFromUnacknowledgedList(const Q_UINT32 packetId);
		void removeBridge(const Q_UINT32 id);
		void sendAcknowledge(const Q_UINT32 destination, const Q_UINT32 lprcvd, const Q_UINT32 lpsent, const Q_UINT64 lpsize);
		void sendBridgeHandshake(const QUuid& nonce, const Q_UINT32 bridgeId);
		void sendCancelData(const Q_UINT32 destination, const Q_UINT32 lprcvd, const Q_UINT32 lpsent, const Q_UINT64 lpsize);
		void sendControlPacket(const Packet::Type type, const Q_UINT32 destination, const Q_UINT32 lprcvd, const Q_UINT32 lpsent, const Q_UINT64 lpsize);
		void sendFault(const Q_UINT32 destination, const Q_UINT32 lprcvd, const Q_UINT32 lpsent, const Q_UINT64 lpsize);
		void sendInternal(Packet* packet, const Q_UINT32 bridgeId);
		void sendReset(const Q_UINT32 destination, const Q_UINT32 lprcvd, const Q_UINT32 lpsent, const Q_UINT64 lpsize);
		void sendTimeout(const Q_UINT32 destination, const Q_UINT32 lprcvd, const Q_UINT32 lpsent, const Q_UINT64 lpsize);
		void stopAllSends(const Q_UINT32 session);

	signals:
		void connected();

	private: // internal
		const QMap<Q_UINT32, TransportBridge*> & getBridges();
		const QMap<Q_UINT32, PacketList*> & getPacketLists();
		const QPtrList<Packet> & getSentPackets();

	private:
		class TransportPrivate;
		TransportPrivate *d;

	friend class PacketScheduler;

}; // Transport
}

#endif
