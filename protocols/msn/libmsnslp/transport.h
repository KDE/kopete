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
		/** @brief Represents the type of connection between peer-to-peer transport layers.
		 * None - There is no connection between the peer-to-peer transport layers
		 * Indirect - There is a connection facilitated by the switchboard network between the peer-to-peer transport layers.
		 * Direct - There is a direct connection between the peer-to-peer transport layers
		 */
		enum Type { None=0, Indirect=1, Direct=2 };

	public :
		/** @brief Creates a new instance of the Transport class. */
		Transport(QObject *parent);
		~Transport();

		DirectTransportBridge* createDirectBridge(const QMap<QString, QVariant> & transportInfo);
		SwitchboardBridge* createIndirectBridge();
		Q_INT16 listen(const QMap<QString, QVariant> & transportInfo);
		bool isConnected() const;
		bool isDirectlyConnected() const;
		void registerPort(const Q_UINT32 port, SessionNotifier* notifier);
		Q_UINT32 send(const QByteArray& message, const Q_UINT32 destination, const Q_UINT32 correlationId);
		void sendBytes(const QByteArray& bytes, const Q_UINT32 destination, const Q_UINT32 correlationId);
		void sendFile(QFile *file, const Q_UINT32 destination);
		void unregisterPort(const Q_UINT32 port);

	private slots:
		void onBridgeConnect();
		void onBridgeDisconnect();
		void onBridgeError();
		void onBridgeAuthenticationKeyReceive(const QUuid& nonce, const Q_UINT32 bridgeId);
		void onReceive(const QByteArray& bytes);
		void onSendFailed(Packet* packet);
		void onSent(const Q_UINT32 id);

	private:
		void addBridge(TransportBridge *bridge);
		void addPacketToUnacknowledgedList(Packet *packet);
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
		void sendBridgeAuthenticationKey(const QUuid& key, const Q_UINT32 bridgeId);
		void sendCancelData(const Q_UINT32 destination, const Q_UINT32 lprcvd, const Q_UINT32 lpsent, const Q_UINT64 lpsize);
		void sendControlPacket(const Packet::Type type, const Q_UINT32 destination, const Q_UINT32 lprcvd, const Q_UINT32 lpsent, const Q_UINT64 lpsize);
		void sendFault(const Q_UINT32 destination, const Q_UINT32 lprcvd, const Q_UINT32 lpsent, const Q_UINT64 lpsize);
		void sendPacket(Packet* packet, const Q_UINT32 bridgeId);
		void sendNonAcknowledge(const Q_UINT32 destination, const Q_UINT32 lprcvd, const Q_UINT32 lpsent, const Q_UINT64 lpsize);
		void sendReset(const Q_UINT32 destination, const Q_UINT32 lprcvd, const Q_UINT32 lpsent, const Q_UINT64 lpsize);
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
