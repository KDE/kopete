/*
    transport.h - Peer to peer transport

    Copyright (c) 2005 by Gregg Edghill     <gregg.edghill@gmail.com>

    Kopete    (c) 2002-2005 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; version 2 of the License.               *
    *                                                                       *
    *************************************************************************
*/

#ifndef PEERTOPEERTRANSPORT_H
#define PEERTOPEERTRANSPORT_H

//BEGIN QT Includes
#include <qobject.h>
#include <qguardedptr.h>
#include <qvaluelist.h>
//END

//BEGIN KDE Includes
#include <ksocketaddress.h>
//END

namespace KNetwork {
class KClientSocketBase;
}

class MSNSwitchBoardSocket;

namespace PeerToPeer {

class MessageFormatter;
class TransportBridge;

enum TransportBridgeType
{
	Tcp = 1,
	Udp = 2
};
	
/**
 @author Gregg Edghill <gregg.edghill@gmail.com> */
/** @brief Represents the protocol used to send and receive message between peers. */
class Transport : public QObject
{
	Q_OBJECT
public:
	/** @brief Creates a new instance of the class Transport. */
    Transport(QObject* parent, const char* name = 0l);
    ~Transport();
	/** @brief Get a transport bridge with the specified address, port, type and identifier. */
    TransportBridge* getBridge(const QString& address, Q_UINT16 port, TransportBridgeType type, const QString& identifier);
    /** @brief Sets the default transport bridge. */
	void setDefaultBridge(MSNSwitchBoardSocket* mss);
	
private slots:
	/** @brief Invokes when a message is received on a transport bridge. */
// 	void slotOnReceive(Message& message);
	/** @brief Invokes when a message is received on the default transport bridge (relay). */
	void slotOnReceive(const QString& contact, const QByteArray& bytes);

private:
	/** @brief Known SocketAddresses of peers. */
	QMap<QString, KNetwork::KInetSocketAddress> mAddresses;
	/** @brief The list the connected transport bridges. */
	QValueList<TransportBridge*> mBridges;
	/** @brief The default transport bridge (relay). */
	QGuardedPtr<MSNSwitchBoardSocket> mDefaultBridge;
	/** @brief Message formatter used to ser/deser message. */
	MessageFormatter *mFormatter;
};

/** @brief Represents the channel connecting two peers. */
class TransportBridge : public QObject
{
	Q_OBJECT
public:
	virtual ~TransportBridge();

protected:
	/** @brief Creates a new instance of the class TransportBridge with the specified address and formatter. */
	TransportBridge(const KNetwork::KInetSocketAddress& to, MessageFormatter* formatter, QObject* parent, const char* name = 0l);
	/** @brief Creates a new instance of the class TransportBridge with the specified socket and formatter. */
	TransportBridge(KNetwork::KClientSocketBase* socket, MessageFormatter* formatter, QObject* parent, const char* name = 0l);

public:
	/** @brief Creates a connection between two peers. */
	void connect();
	/** @brief Disconnects the connection between two peers. */
	void disconnect();

protected slots:
	virtual void slotOnConnect();
	virtual void slotOnDisconnect();
	virtual void slotOnError(int);
	virtual void slotOnSocketClose();
	virtual void slotOnSocketConnect();
	virtual void slotOnSocketReceive();

signals:
	void bridgeConnect();
	void bridgeDisconnect();
	void bridgeError(const QString& e);
	void bytesReceived(const QByteArray&);
	
protected:
	
	KNetwork::KInetSocketAddress mAddress;
	bool mConnected;
	MessageFormatter *mFormatter;
	Q_UINT32 mLength;
	KNetwork::KClientSocketBase *mSocket;
	bool mVerified;
};

class TcpTransportBridge : public TransportBridge
{
	Q_OBJECT
	friend class Transport;
	
public:
	virtual ~TcpTransportBridge();

private:
	TcpTransportBridge(const KNetwork::KInetSocketAddress& to, MessageFormatter* formatter, QObject* parent, const char* name = 0l);
	TcpTransportBridge(KNetwork::KClientSocketBase* socket, MessageFormatter* formatter, QObject* parent, const char* name = 0l);
	
protected slots:
	virtual void slotOnConnect();
	virtual void slotOnDisconnect();
	virtual void slotOnError(int);
	virtual void slotOnSocketClose();
	virtual void slotOnSocketConnect();
	virtual void slotOnSocketReceive();
	
private slots:
	void slotOnSocketConnectTimeout();

signals:
	void bridgeConnectTimeout();
	
private:
	class Buffer : public QByteArray
	{
	public:
		Buffer(Q_UINT32 length = 0);
		~Buffer();
		
	public:
		void write(const QByteArray& bytes);
		QByteArray read(Q_UINT32 length);
	};
	
	Buffer mBuffer;
	Q_UINT32 mLength;
};


}

#endif
