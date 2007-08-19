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
		TcpTransportBridgePrivate() : bytesToRead(0), connectivityVerified(false), length(0),
			endpoint(0l), socket(0l) {}

		QByteArray buffer;
		Q_INT32 bytesToRead;
		bool connectivityVerified;
		Q_UINT32 length;
		KServerSocket *endpoint;
		Q_UINT32 bridgeId;
		QMap<QString, QVariant> properties;
		KStreamSocket *socket;
		Q_INT32 socketId;
};

TcpTransportBridge::TcpTransportBridge(const QValueList<QString>& addresses, const Q_UINT16 port, const Q_UINT32 bridgeId, QObject *parent) : DirectTransportBridge(addresses, port, parent), d(new TcpTransportBridgePrivate())
{
	d->bridgeId = bridgeId;
	d->properties.insert("mtu", 1352);
	d->properties.insert("throttle", 15);
}

TcpTransportBridge::~TcpTransportBridge()
{
	delete d;
}

const QMap<QString, QVariant> & TcpTransportBridge::getProperties() const
{
	return d->properties;
}

Q_UINT32 TcpTransportBridge::id() const
{
	return d->bridgeId;
}

void TcpTransportBridge::send(const QByteArray& datachunk, const Q_UINT32 id)
{
	QByteArray bytes(4 + datachunk.size());
	QDataStream stream(bytes, IO_WriteOnly);
	stream.setByteOrder(QDataStream::LittleEndian);
	// Write the length preamble to the stream.
	stream << datachunk.size();
	stream.writeRawBytes(datachunk.data(), datachunk.size());

	kdDebug() << k_funcinfo << "About to send datachunk of size "
		<< datachunk.size() << " bytes" << endl;

	// Get the size of the datachunk to send.
	const Q_UINT32 size = bytes.size();
	Q_UINT32 offset = 0;
	while(offset < size)
	{
		Q_INT32 bytesWritten = d->socket->writeBlock(bytes.data() + offset, size - offset);
		if (bytesWritten > 0)
		{
			kdDebug() << k_funcinfo << "Sent " << bytesWritten << " bytes on socket " << d->socketId << endl;
			offset += bytesWritten;
		}
		else
		{
			// Otherwise, an error has occurred, bail.
			break;
		}
	}

	emit dataSent(id);
}

void TcpTransportBridge::onConnect()
{
	if (state() == TransportBridge::Connected)
	{
		kdDebug() << k_funcinfo << "Already connected on socket " << d->socketId << endl;
		return;
	}

	if (d->endpoint == 0l)
	{
		// If the bridge is not connected, connect the bridge.
		d->socket = new KStreamSocket(addresses()[0], QString::number(port()), this);
		// Set the socket to non blocking.
		d->socket->setBlocking(false);
		// Enable asynchronous read operations.
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

//BEGIN Endpoint Functions

bool TcpTransportBridge::listen()
{
	const QString address = *DirectTransportBridge::addresses().at(0);
	// Create a listening socket for direct file transfer.
	d->endpoint = new KServerSocket(address, QString::number(port()), this);
	d->endpoint->setResolutionEnabled(false);
	d->endpoint->setAcceptBuffered(false);
	// Create the callback that will try to accept incoming connections.
	QObject::connect(d->endpoint, SIGNAL(readyAccept()), this, SLOT(onSocketAccept()));
	QObject::connect(d->endpoint, SIGNAL(gotError(int)), this, SLOT(onListenEndpointError(int)));

	// Listen for incoming connections.
	bool listening = d->endpoint->listen();
	if (listening)
	{
		kdDebug() << k_funcinfo << "Listening on socket " << d->endpoint->socketDevice()->socket()
		<< " (addr=" << address << " port=" << port() << ")" << endl;
	}

	return listening;
}

void TcpTransportBridge::onSocketAccept()
{
	d->socket = static_cast<KStreamSocket*>(d->endpoint->accept());

	d->socketId = d->socket->socketDevice()->socket();

	kdDebug() << k_funcinfo << "Accepted socket " << d->socketId
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
	kdDebug() << k_funcinfo << errorCode << ": " << d->endpoint->errorString() << endl;
	// Disconnect the signal/slot
	QObject::disconnect(d->endpoint, SIGNAL(gotError(int)), this,
	SLOT(onListenEndpointError(int)));

	emit error();
}

//END

//BEGIN Socket Functions

void TcpTransportBridge::onSocketClosed()
{
	setState(TransportBridge::Disconnected);
	kdDebug() << k_funcinfo << "Socket " << d->socketId << " closed" << endl;
	// Disconnect the signal/slot
	QObject::disconnect(d->socket, 0, this, 0);

	emit disconnected();
}

void TcpTransportBridge::onSocketConnected()
{
	setState(TransportBridge::Connected);
	d->socketId = d->socket->socketDevice()->socket();

	kdDebug() << k_funcinfo << "Connected on socket " << d->socketId
	<< " (addr=" << addresses()[0] << " port=" << port() << ")" << endl;

	if (d->endpoint == 0l)
	{
		// If the transport bridge is not a listener,
		// send the pseudo hello preamble.
		sendPseudoHello();
	}

	// Signal that we are connected.
	emit connected();
}

void TcpTransportBridge::onSocketConnectTimeout()
{
	kdDebug() << k_funcinfo << "Connect timeout on socket " << d->socketId << endl;
	d->socket->disconnect();

	emit error();
}

void TcpTransportBridge::onSocketError(int errorCode)
{
	Q_UNUSED(errorCode);

	kdDebug() << k_funcinfo << "Got error, " << d->socket->errorString() << ", on socket "
	<< d->socketId << endl;

	d->socket->disconnect();

	// Disconnect the signal/slot
	QObject::disconnect(d->socket, SIGNAL(gotError(int)), this,
	SLOT(onSocketError(int)));

	emit error();
}

void TcpTransportBridge::onSocketRead()
{
	kdDebug() << k_funcinfo << "enter" << endl;

	const Q_UINT32 bytesAvailable = d->socket->bytesAvailable();
	if (bytesAvailable > 0)
	{
		d->bytesToRead += bytesAvailable;
		kdDebug() << k_funcinfo << d->bytesToRead << " bytes to read" << endl;

		QDataStream binaryReader(d->socket);
		binaryReader.setByteOrder(QDataStream::LittleEndian);

		while((d->bytesToRead >= (Q_INT32)d->length) && d->bytesToRead >= 4 && state() == TransportBridge::Connected)
		{
			if (d->length == 0)
			{
				// Read the byte array length prefix from the stream.
				binaryReader >> d->length;
				kdDebug() << k_funcinfo << "About to receive datachunk of size " << d->length << " bytes" << endl;

				d->bytesToRead -= 4;
			}

			if (d->length > 0 && d->bytesToRead >= (Q_INT32)d->length)
			{
				kdDebug() << k_funcinfo << "Received " << d->length << " bytes on socket "
					<< d->socketId << endl;

				if (!d->connectivityVerified)
				{
					// If the connectivity has not been verified,
					// try to process the pseudo hello sent by
					// a connector transport bridge.
					processPseudoHello();
				}
				else
				{
					// Otherwise, read the raw data received.
					QByteArray bytes(d->length);
					// Read the bytes of data from the stream.
					d->socket->readBlock(bytes.data(), bytes.size());
					// Signal that we have received data.
					emit dataReceived(bytes);
				}

				d->bytesToRead -= d->length;
				d->length = 0;
			}
			else
			{
				kdDebug() << k_funcinfo << "Waiting for " << (d->length - d->bytesToRead)
				<< " bytes on socket " << d->socketId << endl;
			}
		}
	}

	kdDebug() << k_funcinfo << "leave" << endl;
}

//END

void TcpTransportBridge::processPseudoHello()
{
	QByteArray preamble(4);
	// Read the preamble bytes of data from the stream.
	d->socket->readBlock(preamble.data(), preamble.size());

	kdDebug() << k_funcinfo << "Received " << preamble.size() << " bytes on socket "
		<< d->socketId << endl;

	if (preamble[0] == 0x66 && preamble[1] == 0x6f &&
		preamble[2] == 0x6f && preamble[3] == 0x00)
	{
		kdDebug() << k_funcinfo << "preamble " << preamble
			<< " -- connectivity verified" << endl;
		d->connectivityVerified = true;
	}
	else
	{
		kdDebug() << k_funcinfo << "Invalid preamble -- disconnecting" << endl;
		d->bytesToRead = -1;
		// Signal that an error has occurred.
		emit error();
	}
}

void TcpTransportBridge::sendPseudoHello()
{
	QByteArray bytes(8);
	QDataStream stream(bytes, IO_WriteOnly);
	stream.setByteOrder(QDataStream::LittleEndian);

	QByteArray preamble(4);
	preamble[0] = 0x66; preamble[1] = 0x6f;
	preamble[2] = 0x6f; preamble[3] = 0x00;

	kdDebug() << k_funcinfo << "Sending preamble " << preamble
	<< " on socket " << d->socketId << endl;

	// Write the length preamble to the network stream
	stream << preamble.size();
	// Write the connection check data to the stream.
	stream.writeRawBytes(preamble.data(), preamble.size());

	// Write the data bytes to the network stream.
	Q_UINT32 bytesWritten = d->socket->writeBlock(bytes.data(), bytes.size());

	kdDebug() << k_funcinfo << "Sent " << bytesWritten << " bytes on socket "
	<< d->socketId << endl;

	d->connectivityVerified = true;
}

}

#include "tcptransportbridge.moc"
