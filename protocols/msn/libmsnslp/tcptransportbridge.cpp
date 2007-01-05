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

using namespace KNetwork;

namespace PeerToPeer
{

class TcpTransportBridge::TcpTransportBridgePrivate
{
	public:
		TcpTransportBridgePrivate() : endpoint(0l), maxSendBufferSize(1305),
			port(0), socket(0l) {}

		QString address;
		QByteArray buffer;
		KServerSocket *endpoint;
		Q_UINT32 identifier;
		Q_UINT32 maxSendBufferSize;
		Q_INT16 port;
		KStreamSocket *socket;
};

TcpTransportBridge::TcpTransportBridge(const QString& address, const Q_UINT16 port, QObject *parent) : TransportBridge(parent), d(new TcpTransportBridgePrivate())
{
	d->address = address;
	d->port = port;
}

TcpTransportBridge::~TcpTransportBridge()
{
	delete d;
	d = 0l;
}

const Q_UINT32 TcpTransportBridge::identifier() const
{
	return d->identifier;
}

const Q_UINT32 TcpTransportBridge::maxSendBufferSize()
{
	return d->maxSendBufferSize;
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
		d->socket = new KStreamSocket(d->address, QString::number(d->port), this);
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
		if (d->socket)
		{
			d->socket->disconnect();
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
	request << d->address;
	request << (Q_INT32)d->port;
	request << QString::fromLatin1("TCP");
	request << (Q_INT32)d->port;
	request << true;
	request << QString("Transfer (%1:%2) %3 %4").arg(d->address).arg(d->port).arg(d->port).arg("TCP");
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
	d->endpoint = new KServerSocket(d->address, QString::number(d->port), this);
	d->endpoint->setResolutionEnabled(false);
	d->endpoint->setAcceptBuffered(false);
	// Create the callback that will try to accept incoming connections.
	QObject::connect(d->endpoint, SIGNAL(readyAccept()), this, SLOT(onListenEndpointAccept()));
	QObject::connect(d->endpoint, SIGNAL(gotError(int)), this, SLOT(onListenEndpointError(int)));

	d->endpoint->bind();
	// Listen for incoming connections.
	bool listening = d->endpoint->listen(10);
	if (listening)
	{
		kdDebug() << k_funcinfo << "Listening on socket " << d->endpoint->socketDevice()->socket()
		<< " (addr=" << d->address << " port=" << d->port << ")" << endl;
	}

	return listening;
}

void TcpTransportBridge::onListenEndpointAccept()
{
	d->socket = static_cast<KStreamSocket*>(d->endpoint->accept());

	kdDebug() << k_funcinfo << "Accepted socket " << d->socket->socketDevice()->socket() << endl;
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
	QObject::disconnect(d->endpoint, SIGNAL(gotError(int)), this, SLOT(onListenEndpointError(int)));
	emit error();
}

//END

//BEGIN Socket Functions

void TcpTransportBridge::onSocketClosed()
{
	setState(TransportBridge::Disconnected);
	kdDebug() << k_funcinfo << "Socket " << d->socket->socketDevice()->socket() << " closed" << endl;
	emit disconnected();
}

void TcpTransportBridge::onSocketConnected()
{
	setState(TransportBridge::Connected);
	kdDebug() << k_funcinfo << "Connected on socket " << d->socket->socketDevice()->socket() << endl;
	emit connected();

	if (d->endpoint == 0l)
	{
		const QByteArray ccheck = QCString("foo");
		kdDebug() << k_funcinfo << "Sending CONNECTION CHECK packet " <<  ccheck
		<< " on socket " << d->socket->socketDevice()->socket() << endl;
		d->socket->writeBlock(ccheck.data(), ccheck.size());
	}
}

void TcpTransportBridge::onSocketConnectTimeout()
{
	kdDebug() << k_funcinfo << "Connect timeout on socket " << d->socket->socketDevice()->socket() << endl;
	emit timeout();
}

void TcpTransportBridge::onSocketError(int errorCode)
{
	kdDebug() << k_funcinfo << "Error " << d->socket->errorString() << ", on socket "
	<< d->socket->socketDevice()->socket() << endl;
	emit error();
}

void TcpTransportBridge::onSocketRead()
{
	const Q_UINT32 bytesAvailable = d->socket->bytesAvailable();
	if (bytesAvailable > 0)
	{
		kdDebug() << k_funcinfo << "Socket " << d->socket->socketDevice()->socket()
		<< ", " << bytesAvailable << " bytes available" << endl;

		QDataStream stream(d->socket);
		Q_UINT32 length = 0;
		stream >> length;

		kdDebug() << k_funcinfo << "Reading " << length << " bytes on socket "
		<< d->socket->socketDevice()->socket() << endl;
	}
}

//END

}

#include "tcptransportbridge.moc"
