/*
    transport.cpp - Peer to peer transport

    Copyright (c) 2005 by Gregg Edghill     <gregg.edghill@gmail.com>

    Kopete    (c) 2002-2005 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "transport.h"
#include "messageformatter.h"

//BEGIN QT Includes
//END

//BEGIN KDE Includes
#include <kclientsocketbase.h>
#include <kdebug.h>
#include <kstreamsocket.h>
//END

//BEGIN Using Directives
using namespace KNetwork;
//END

#include "msnswitchboardsocket.h"

namespace PeerToPeer {

Transport::Transport(QObject* parent)
 : QObject(parent)
{
	mFormatter = new PeerToPeer::MessageFormatter(this);
}


Transport::~Transport()
{
}

//BEGIN Public Methods

TransportBridge* Transport::getBridge (const QString& to, quint16 port, TransportBridgeType type, const QString& identifier)
{
	TransportBridge *bridge = 0l;
	KInetSocketAddress address;
	if (mAddresses.contains(to))
	{
		address = mAddresses[to];
	}
	else
	{
		address = KInetSocketAddress(KIpAddress(to), port);
		mAddresses[to] = address;
	}

	if (PeerToPeer::Tcp == type){
		bridge = new TcpTransportBridge(address, mFormatter, this, identifier.ascii());
	}

	if (PeerToPeer::Udp == type){
// TODO Add class UdpTransportBridge
// 		bridge = new UdpTransportBridge(address, this, mFormatter, identifier.ascii());
	}

	if (bridge != 0l)
	{
		QObject::connect(bridge, SIGNAL(readyRead(const QByteArray&)), SLOT(slotOnReceive(const QByteArray&)));
	}

	return 0l;
}

void Transport::setDefaultBridge(MSNSwitchBoardSocket* mss)
{
	mDefaultBridge = mss;
	QObject::connect((MSNSwitchBoardSocket*)mDefaultBridge, SIGNAL(messageReceived(const QString&, const QByteArray&)), SLOT(slotOnReceive(const QString&, const QByteArray&)));
}

//END

//BEGIN Private Slot Methods

// void Transport::slotOnReceive(Message& message)
// {
// }

void Transport::slotOnReceive(const QString& contact, const QByteArray& bytes)
{
	kDebug (14140) << " >> RECEIVED " << bytes.size() << " bytes.";
// 	Message message = mFormatter->readMessage(bytes);
}

//END




TransportBridge::TransportBridge(const KNetwork::KInetSocketAddress& to, MessageFormatter* formatter, QObject* parent)
: QObject(parent)
{
	mAddress = to;
	mFormatter = formatter;
}

TransportBridge::TransportBridge(KNetwork::KClientSocketBase* socket, MessageFormatter* formatter, QObject* parent)
: QObject(parent)
{
	mSocket = socket;
	mAddress = mSocket->peerAddress();
}

TransportBridge::~TransportBridge()
{
}

//BEGIN Public Methods

void TransportBridge::connect()
{
	slotOnConnect();
}

void TransportBridge::disconnect()
{
	slotOnDisconnect();
}

//END

//BEGIN Protected Slot Methods

void TransportBridge::slotOnConnect()
{
}

void TransportBridge::slotOnDisconnect()
{
}

void TransportBridge::slotOnError(int)
{
}

void TransportBridge::slotOnSocketClose()
{
}

void TransportBridge::slotOnSocketConnect()
{
}

void TransportBridge::slotOnSocketReceive()
{
}


//END



TcpTransportBridge::TcpTransportBridge(const KNetwork::KInetSocketAddress& to, MessageFormatter* formatter, QObject* parent)
: TransportBridge(to, formatter, parent)
{
	mSocket = new KStreamSocket(mAddress.ipAddress().toString(), QString::number(mAddress.port()), this);
	mSocket->setBlocking(false);
	QObject::connect(mSocket, SIGNAL(connected(const KResolverEntry&)), SLOT(slotOnSocketConnect()));
	QObject::connect(mSocket, SIGNAL(gotError(int)), SLOT(slotOnError(int)));
	mConnected = false;
}

TcpTransportBridge::TcpTransportBridge(KNetwork::KClientSocketBase* socket, MessageFormatter* formatter, QObject* parent)
: TransportBridge(socket, formatter, parent)
{
	mConnected = (mSocket->state() == KStreamSocket::Open) ? true : false;
	mSocket->setBlocking(false);
}

TcpTransportBridge::~TcpTransportBridge()
{
}

//BEGIN Protected Slot Methods

void TcpTransportBridge::slotOnConnect()
{
	if (mConnected)
	{
		kDebug(14140) << "Bridge (" << name() << ") ALREADY CONNECTED " << mSocket->peerAddress().toString() << " <-> " << mSocket->localAddress().toString();
		return;
	}

	KStreamSocket *socket = static_cast<KStreamSocket*>(mSocket);
	socket->setTimeout(5000);
	QObject::connect(socket, SIGNAL(timeOut()), SLOT(slotOnSocketConnectTimeout()));
	mSocket->connect();
}

void TcpTransportBridge::slotOnDisconnect()
{
	if (mConnected){
		mSocket->close();
	}
}

void TcpTransportBridge::slotOnError(int errorCode)
{
	kDebug(14140) << "Bridge (" << name() << ") ERROR occurred on {" << mSocket->localAddress().toString() << " <-> " << mSocket->peerAddress().toString() << "} - " << mSocket->errorString();
	emit bridgeError(QString("Bridge ERROR %1: %2").arg(errorCode).arg(mSocket->errorString()));
	if (mConnected){
		mSocket->disconnect();
		mConnected = false;
	}
	mSocket->deleteLater();
	mSocket = 0l;
}

void TcpTransportBridge::slotOnSocketClose()
{
	mSocket->disconnect();
	kDebug(14140) << "Bridge (" << name() << ") DISCONNECTED {" << mSocket->peerAddress().toString() << " <-> " << mSocket->localAddress().toString() << "}";
	mConnected = false;
	mSocket->deleteLater();
	mSocket = 0l;

	emit bridgeDisconnect();
}

void TcpTransportBridge::slotOnSocketConnect()
{
	kDebug(14140) << "Bridge (" << name() << ") CONNECTED to " << mSocket->peerAddress().toString() << " from "
		<< mSocket->localAddress().toString() << endl;
	mConnected = true;

	QObject::connect(mSocket, SIGNAL(readyRead()), SLOT(slotOnSocketReceive()));
	QObject::connect(mSocket, SIGNAL(closed()), SLOT(slotOnSocketClose()));

	mVerified = true;
	QString foo = "foo\0";
	mSocket->write(foo.toAscii(), foo.length());
	foo.clear();

	emit bridgeConnect();
}

void TcpTransportBridge::slotOnSocketReceive()
{
	kDebug (14140) << "Bridge (" << name() << ") RECEIVED " << mSocket->bytesAvailable() << " bytes.";

	QByteArray bytes(mSocket->bytesAvailable());
	mSocket->read(bytes.data(), bytes.size());
	// Write the data to the buffer.
	mBuffer.write(bytes);

	if (mVerified == false && mBuffer.size() >= 4)
	{
		QByteArray foo = mBuffer.read(4);
		if (QString(foo) == "foo"){
			kDebug (14140) << "Bridge (" << name() << ") CONNECTION verified.";
			mVerified = true;
		}
	}

	while(mBuffer.size() > 0)
	{
		if (mBuffer.size() >= 4 && mLength == 0)
		{
			QByteArray array = mBuffer.read(4);
			for (int i=0; i < 4; i++){
				((char*)mLength)[i] = array[i];
			}
		}

		if (mLength > 0 && mBuffer.size() >= mLength)
		{
			kDebug (14140) << "Bridge (" << name() << ") read " << mLength << " bytes.";
			bytes = mBuffer.read(mLength);
			mLength = 0;
// 			Message message = mFormatter->readMessage(bytes, true);
// 			emit messageReceived(message);
		}
		else
		{
			kDebug (14140) << "Bridge (" << name() << ") waiting for " << mLength << " bytes.";
			break;
		}
	}
}

//END

//BEGIN Private Slot Methods

void TcpTransportBridge::slotOnSocketConnectTimeout()
{
	kDebug (14140) << "Bridge (" << name() << ") CONNECT timeout.";
	emit bridgeConnectTimeout();
	mSocket->deleteLater();
	mSocket = 0l;
}

//END




TcpTransportBridge::Buffer::Buffer(quint32 length)
: QByteArray(length)
{
}

TcpTransportBridge::Buffer::~Buffer()
{
}

//BEGIN Public Methods

void TcpTransportBridge::Buffer::write(const QByteArray& bytes)
{
	resize(size() + bytes.size());
	for (uint i=0; i < bytes.size(); i++){
		(*this)[size() + i] = bytes[i];
	}
}

QByteArray TcpTransportBridge::Buffer::read(quint32 length)
{
	if (length >= size()) return QByteArray();

	QByteArray buffer;
	buffer.duplicate(data(), length);

	char *bytes = new char[size() - length];
	for(uint i=0; i < size() - length; i++){
		bytes[i] = data()[length + i];
	}

	duplicate(bytes, size() - length);
	delete[] bytes;

	return buffer;
}

//END
}

#include "transport.moc"

