/*
    udptransportbridge.cpp - UDP Peer to peer transport

    Copyright (c) 2007 by Michel Saliba     <msalibaba@gmail.com>

    Kopete    (c) 2002-2007 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "udptransportbridge.h"
#include "messageformatter.h"

//BEGIN QT Includes
//END

//BEGIN KDE Includes
#include <kclientsocketbase.h>
#include <kdebug.h>
#include <kdatagramsocket.h>
//END

//BEGIN Using Directives
using namespace KNetwork;
//END

#include "msnswitchboardsocket.h"
namespace PeerToPeer {

//BEGIN UDP Transport public methods
UdpTransportBridge::~UdpTransportBridge()
{
}
//END

//BEGIN UDP Transport private methods
UdpTransportBridge::UdpTransportBridge(const KNetwork::KInetSocketAddress& to, MessageFormatter* formatter, QObject* parent) : TransportBridge(to, formatter, parent)
{
	mSocket = new KDatagramSocket(this);
	mSocket->setBlocking(false);
	QObject::connect(mSocket, SIGNAL(connected(const KResolverEntry&)), SLOT(slotOnSocketConnect()));
	QObject::connect(mSocket, SIGNAL(gotError(int)), SLOT(slotOnError(int)));
	mConnected = false;
}
UdpTransportBridge::UdpTransportBridge(KNetwork::KClientSocketBase* socket, MessageFormatter* formatter, QObject* parent) : TransportBridge(socket, formatter, parent)
{
	mConnected = (mSocket->state() == KDatagramSocket::Open) ? true : false;
	mSocket->setBlocking(false);
}
//END

//BEGIN UDP Transport protected slot methods
void UdpTransportBridge::slotOnConnect()
{
	if (mConnected)
	{
		kDebug() << "Bridge (" << name() << ") ALREADY CONNECTED " << mSocket->peerAddress().toString() << " <-> " << mSocket->localAddress().toString();
		return;
	}

	KDatagramSocket *socket = static_cast<KDatagramSocket*>(mSocket);
	socket->setTimeout(5000);
	QObject::connect(socket, SIGNAL(timeOut()), SLOT(slotOnSocketConnectTimeout()));
	mSocket->connect();
}

void UdpTransportBridge::slotOnDisconnect()
{
	if (mConnected)
	{
		mSocket->close();
	}
}

void UdpTransportBridge::slotOnError(int errorCode)
{
	kDebug() << "Bridge (" << name() << ") ERROR occurred on {" << mSocket->localAddress().toString() << " <-> " << mSocket->peerAddress().toString() << "} - " << mSocket->errorString();
	emit bridgeError(QString("Bridge ERROR %1: %2").arg(errorCode).arg(mSocket->errorString()));
	if (mConnected){
		mSocket->disconnect();
		mConnected = false;
	}
	mSocket->deleteLater();
	mSocket = 0l;
}

void UdpTransportBridge::slotOnSocketClose()
{
	mSocket->disconnect();
	kDebug(14140) << "Bridge (" << name() << ") DISCONNECTED {" << mSocket->peerAddress().toString() << " <-> " << mSocket->localAddress().toString() << "}";
	mConnected = false;
	mSocket->deleteLater();
	mSocket = 0l;

	emit bridgeDisconnect();
}

void UdpTransportBridge::slotOnSocketConnect()
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

void UdpTransportBridge::slotOnSocketReceive()
{
	kDebug () << "Bridge (" << name() << ") RECEIVED " << mSocket->bytesAvailable() << " bytes.";
	
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
			kDebug () << "Bridge (" << name() << ") read " << mLength << " bytes.";
			bytes = mBuffer.read(mLength);
			mLength = 0;
// 			Message message = mFormatter->readMessage(bytes, true);
// 			emit messageReceived(message);
		}
		else
		{
			kDebug () << "Bridge (" << name() << ") waiting for " << mLength << " bytes.";
			break;
		}
	}
}
//END

//BEGIN UDP Transport private slot methods
void UdpTransportBridge::slotOnSocketConnectTimeout()
{
	kDebug () << "Bridge (" << name() << ") CONNECT timeout.";
	emit bridgeConnectTimeout();
	mSocket->deleteLater();
	mSocket = 0l;
}
//END

UdpTransportBridge::Buffer::Buffer(quint32 length)
: QByteArray(length)
{
}

UdpTransportBridge::Buffer::~Buffer()
{
}

//BEGIN Public Methods

void UdpTransportBridge::Buffer::write(const QByteArray& bytes)
{
	resize(size() + bytes.size());
	for (uint i=0; i < bytes.size(); i++){
		(*this)[size() + i] = bytes[i];
	}
}

QByteArray UdpTransportBridge::Buffer::read(quint32 length)
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
}
//END


