/*
    tcptransportbridge.cpp - Peer To Peer Tcp Transport Bridge

    Copyright (c) 2006 by Gregg Edghill     <gregg.edghill@gmail.com>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; version 2 of the License.               *
    *                                                                       *
    *************************************************************************
*/

#include "tcptransportbridge.h"
#include "binarypacketformatter.h"
#include <dcopclient.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kserversocket.h>
#include <ksocketdevice.h>
#include <kstreamsocket.h>
#include <qdatastream.h>
#include <stdlib.h>

using namespace KNetwork;

namespace PeerToPeer
{

class TcpTransportBridge::TcpTransportBridgePrivate
{
	public:
		TcpTransportBridgePrivate() : bytesToRead(0), length(0), endpoint(0l), maxSendBufferSize(1352),
			port(0), socket(0l) {}

		QValueList<QString> addresses;
		QByteArray buffer;
		Q_UINT32 bytesToRead;
		Q_UINT32 length;
		KServerSocket *endpoint;
		Q_UINT32 identifier;
		Q_UINT32 maxSendBufferSize;
		Q_INT16 port;
		KStreamSocket *socket;
};

TcpTransportBridge::TcpTransportBridge(const QValueList<QString>& addresses, const Q_UINT16 port, QObject *parent) : TransportBridge(parent), d(new TcpTransportBridgePrivate())
{
	d->addresses = addresses;
	d->port = port;
	d->identifier = (rand() & 0xF1FC) + 5;
}

TcpTransportBridge::~TcpTransportBridge()
{
	delete d;
}

const Q_UINT32 TcpTransportBridge::identifier() const
{
	return d->identifier;
}

const Q_UINT32 TcpTransportBridge::maxSendBufferSize()
{
	return d->maxSendBufferSize;
}

void TcpTransportBridge::send(const Packet& packet)
{
	QByteArray bytes(4 + packet.size());
	QDataStream stream(bytes, IO_WriteOnly);
	stream.setByteOrder(QDataStream::LittleEndian);
	// Write the length preamble to the stream.
	stream << packet.size();
	// Serialize the packet into the memory stream.
	BinaryPacketFormatter::serialize(packet, &stream);

	kdDebug() << k_funcinfo << "About to send datachunk of size "
		<< packet.size() << " bytes" << endl;

	Q_UINT32 bytesWritten = d->socket->writeBlock(bytes.data(), bytes.size());
	kdDebug() << k_funcinfo << "Sent " << bytesWritten << " bytes on socket " << d->socket->socketDevice()->socket() << endl;
}

void TcpTransportBridge::onConnect()
{
	if (state() == TransportBridge::Connected)
	{
		kdDebug() << k_funcinfo << "Already connected on socket "
		<< d->socket->socketDevice()->socket() << endl;
		return;
	}

	if (d->endpoint == 0l)
	{
		// If the bridge is not connected, connect the bridge.
		d->socket = new KStreamSocket(d->addresses[0], QString::number(d->port), this);
		// Set the socket to non blocking.
		d->socket->setBlocking(false);
		// Enable asynchronous read opeartions.
		d->socket->enableRead(true);

		// Connect the signal/slot
		QObject::connect(d->socket, SIGNAL(readyRead()), this,
		SLOT(onSocketRead()));
		QObject::connect(d->socket, SIGNAL(connected(const KResolverEntry&)), this,
		SLOT(onSocketConnected()));
		QObject::connect(d->socket, SIGNAL(gotError(int)), this,
		SLOT(onSocketError(int)));
		QObject::connect(d->socket, SIGNAL(timedOut()), this,
		SLOT(onSocketConnectTimeout()));
		QObject::connect(d->socket, SIGNAL(closed()), this,
		SLOT(onSocketClosed()));

		// Try to connect the socket.
		d->socket->connect();
	}
}

void TcpTransportBridge::onDisconnect()
{
	if (state() == TransportBridge::Connected)
	{
		// If the bridge is connected, disconnect it.
		setState(TransportBridge::Disconnecting);
		if (d->socket != 0l)
		{
			// Close the socket.
			d->socket->close();
		}
	}
}

//////////////////////////////////////////////////////////////////////
// Performs UPnP port mapping if it is necessary to establish
// an underlying socket connection.
//////////////////////////////////////////////////////////////////////
void TcpTransportBridge::doUpnpPortMappingIfNecessary()
{
	QByteArray data, replyData;
	QDataStream request( data, IO_WriteOnly );
	request << d->addresses[0];
	request << (Q_INT32)d->port;
	request << QString::fromLatin1("TCP");
	request << (Q_INT32)d->port;
	request << true;
	request << QString("Transfer (%1:%2) %3 %4").arg(d->addresses[0]).arg(d->port).arg(d->port).arg("TCP");
	request << (Q_INT32)0;

	QCString method = "addPortMapping(QString,int,QString,int,bool,QString,int)";
	QCString replyType;
	if (kapp->dcopClient()->call( "kopete", "UPnP", method, data, replyType, replyData ) )
	{
	}
}

//BEGIN Endpoint Functions

bool TcpTransportBridge::listen()
{
	// Create a listening socket for direct file transfer.
	d->endpoint = new KServerSocket(d->addresses[0], QString::number(d->port), this);
	d->endpoint->setResolutionEnabled(false);
	d->endpoint->setAcceptBuffered(false);
	// Create the callback that will try to accept incoming connections.
	QObject::connect(d->endpoint, SIGNAL(readyAccept()), this, SLOT(onSocketAccept()));
	QObject::connect(d->endpoint, SIGNAL(gotError(int)), this, SLOT(onListenEndpointError(int)));

	d->endpoint->bind();
	// Listen for incoming connections.
	bool listening = d->endpoint->listen(10);
	if (listening)
	{
		kdDebug() << k_funcinfo << "Listening on socket " << d->endpoint->socketDevice()->socket()
		<< " (addr=" << d->addresses[0] << " port=" << d->port << ")" << endl;
	}

	return listening;
}

void TcpTransportBridge::onSocketAccept()
{
	d->socket = static_cast<KStreamSocket*>(d->endpoint->accept());

	kdDebug() << k_funcinfo << "Accepted socket " << d->socket->socketDevice()->socket()
	<< " (listening socket " << d->endpoint->socketDevice()->socket() << ")" << endl;

	setState(TransportBridge::Connected);
	emit connected();

	// Set the accepted socket to non blocking.
	d->socket->setBlocking(false);
	// Enable asynchronous read opeartions.
	d->socket->enableRead(true);

	// Connect the signal/slot
	QObject::connect(d->socket, SIGNAL(readyRead()), this,
	SLOT(onSocketRead()));
	QObject::connect(d->socket, SIGNAL(closed()), this,
	SLOT(onSocketClosed()));
	QObject::connect(d->socket, SIGNAL(gotError(int)), this,
	SLOT(onSocketError(int)));
}

void TcpTransportBridge::onListenEndpointError(int errorCode)
{
	kdDebug() << k_funcinfo << d->endpoint->errorString() << endl;
	QObject::disconnect(d->endpoint, SIGNAL(gotError(int)), this,
	SLOT(onListenEndpointError(int)));
	emit error();
}

//END

//BEGIN Socket Functions

void TcpTransportBridge::onSocketClosed()
{
	setState(TransportBridge::Disconnected);
	kdDebug() << k_funcinfo << "Socket " << d->socket->socketDevice()->socket() << " closed" << endl;
	QObject::disconnect(d->socket, 0, this, 0);

	emit disconnected();
}

void TcpTransportBridge::onSocketConnected()
{
	setState(TransportBridge::Connected);
	kdDebug() << k_funcinfo << "Connected on socket " << d->socket->socketDevice()->socket()
	<< " (addr=" << d->addresses[0] << " port=" << d->port << ")" << endl;

	if (d->endpoint == 0l)
	{
		QByteArray bytes(8);
		QDataStream stream(bytes, IO_WriteOnly);
		stream.setByteOrder(QDataStream::LittleEndian);

		QByteArray ccheck(4);
		ccheck[0] = 0x66;
		ccheck[1] = 0x6f;
		ccheck[2] = 0x6f;
		ccheck[3] = 0x00;

		kdDebug() << k_funcinfo << "Sending CONNECTION CHECK packet " << ccheck
		<< " on socket " << d->socket->socketDevice()->socket() << endl;

		// Write the length preamble to the network stream
		stream << ccheck.size();
		// Write the connection check data to the stream.
		stream.writeRawBytes(ccheck.data(), ccheck.size());

		// Write the data bytes to the network stream.
		Q_UINT32 bytesWritten = d->socket->writeBlock(bytes.data(), bytes.size());

		kdDebug() << k_funcinfo << "Sent " << bytesWritten << " bytes on socket "
		<< d->socket->socketDevice()->socket() << endl;
	}

	emit connected();
}

void TcpTransportBridge::onSocketConnectTimeout()
{
	kdDebug() << k_funcinfo << "Connect timeout on socket " << d->socket->socketDevice()->socket() << endl;
	d->socket->disconnect();

	emit timeout();
}

void TcpTransportBridge::onSocketError(int errorCode)
{
	kdDebug() << k_funcinfo << "Got error, " << d->socket->errorString() << ", on socket "
	<< d->socket->socketDevice()->socket() << endl;

	d->socket->disconnect();

	// Disconnect the signal/slot
	QObject::disconnect(d->socket, SIGNAL(gotError(int)), this,
	SLOT(onSocketError(int)));
	emit error();
}

void TcpTransportBridge::onSocketRead()
{
	const Q_UINT32 bytesAvailable = d->socket->bytesAvailable();
	if (bytesAvailable > 0)
	{
		d->bytesToRead += bytesAvailable;

		QDataStream binaryReader(d->socket);
		binaryReader.setByteOrder(QDataStream::LittleEndian);

		while(d->bytesToRead > d->length)
		{
			if (d->bytesToRead >= 4 && d->length == 0)
			{
				// Read the byte array length prefix from the stream.
				binaryReader >> d->length;
				d->bytesToRead -= 4;
			}

			if (d->length > 0 && d->bytesToRead >= d->length)
			{
				kdDebug() << k_funcinfo << "Reading " << d->length << " bytes on socket "
				<< d->socket->socketDevice()->socket() << endl;


				QByteArray bytes(d->length);
				// Read the 'length' bytes of data from the stream.
				d->socket->readBlock(bytes.data(), bytes.size());

				QDataStream stream(bytes, IO_ReadOnly);
				Packet packet = BinaryPacketFormatter::deserialize(&stream);
				emit packetReceived(packet);

				d->bytesToRead -= packet.size();
				d->length = 0;
			}
			else
			{
				kdDebug() << k_funcinfo << "Waiting for " << (d->length - d->bytesToRead)
				<< " bytes on socket " << d->socket->socketDevice()->socket() << endl;
			}
		}
	}
}

//END

}

#include "tcptransportbridge.moc"
