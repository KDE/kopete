//
// IpEndpointConnector
//
// Authors:
//   Gregg Edghill (Gregg.Edghill@gmail.com)
//
// Copyright (C) 2007, Kopete (http://kopete.kde.org)
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of this software.
//
// THIS LIBRARY IS FREE SOFTWARE; YOU CAN REDISTRIBUTE IT AND/OR
// MODIFY IT UNDER THE TERMS OF THE GNU LESSER GENERAL PUBLIC
// LICENSE AS PUBLISHED BY THE FREE SOFTWARE FOUNDATION; EITHER
// VERSION 2 OF THE LICENSE, OR (AT YOUR OPTION) ANY LATER VERSION.
//

#include "Papillon/Network/IpEndpointConnector"
#include "Papillon/Network/NetworkStream"
#include <QtGlobal>
#include <QtNetwork/QNetworkProxy>
#include <QtNetwork/QSslSocket>
#include <QtDebug>

namespace Papillon
{

class IpEndpointConnector::IpEndpointConnectorPrivate
{
	public:
		IpEndpointConnectorPrivate() : connectorState(IpEndpointConnector::Created),
			port(0), underlyingSocketUsesTls(false) {}

		IpEndpointConnector::State connectorState;
		QString ipAddress;
		NetworkStream *networkStream;
		quint16 port;
		QSslSocket *socket;
		bool underlyingSocketUsesTls;
};

IpEndpointConnector::IpEndpointConnector(bool enableTls, QObject *parent) : QObject(parent), d(new IpEndpointConnectorPrivate())
{
	d->underlyingSocketUsesTls = enableTls;
	// Create the underlying socket.
	d->socket = new QSslSocket(this);
	// Set the socket proxy information using the application settings.
	d->socket->setProxy(QNetworkProxy::applicationProxy());
	// Set the socket's inner read buffer capacity.
	d->socket->setReadBufferSize(4096);
	// Create the network stream.
	d->networkStream = new NetworkStream(d->socket, false, this);
}

IpEndpointConnector::~IpEndpointConnector()
{
	delete d;
	d = 0l;
}

NetworkStream * IpEndpointConnector::networkStream()
{
	return state() == IpEndpointConnector::Connected ? d->networkStream : 0l;
}

const IpEndpointConnector::State & IpEndpointConnector::state() const
{
	return d->connectorState;
}

void IpEndpointConnector::close()
{
	if (d->connectorState != IpEndpointConnector::Closing ||
		d->connectorState != IpEndpointConnector::Closed)
	{
		// Set the connector state to closing.
		d->connectorState = IpEndpointConnector::Closing;
		// Signal that the connector is closing.
		emit closing();

		if (d->socket->state() != QSslSocket::UnconnectedState)
		{
			// If the socket is active then gracefully close it;
			// closeConnector will be called asynchronously.
			d->socket->close();
		}
		else
		{
			qDebug("%s: Closing synchronously", Q_FUNC_INFO);

			// Otherwise, just close the connector.
			closeConnector();
		}
	}
}

void IpEndpointConnector::connectWithAddressInfo(const QString& ipAddress, const quint16 port)
{
	if (d->connectorState == IpEndpointConnector::Faulted ||
		d->connectorState == IpEndpointConnector::Closing)
	{
		return;
	}

	if (d->connectorState != IpEndpointConnector::Connected ||
	    d->connectorState != IpEndpointConnector::Connecting)
	{
		d->ipAddress = ipAddress;
		d->port = port;

		// Connect the signal/slot
		QObject::connect(d->socket, SIGNAL(connected()), this,
		SLOT(socket_OnConnect()));
		QObject::connect(d->socket, SIGNAL(error(QAbstractSocket::SocketError)), this,
		SLOT(socket_OnError(QAbstractSocket::SocketError)));
		QObject::connect(d->socket, SIGNAL(disconnected()), this,
		SLOT(socket_OnClose()));

		// Set the connector state to connecting.
		d->connectorState = IpEndpointConnector::Connecting;

		qDebug("%s: Connecting to (addr=%s port=%d)", Q_FUNC_INFO,
			qPrintable(ipAddress), port);

		// Signal that the connector is connecting.
		emit connecting();

		// Try to connect the socket using the address information.
		d->socket->connectToHost(ipAddress, port);
	}
}

void IpEndpointConnector::closeConnector()
{
	// Close the network stream.
	d->networkStream->close();
	// Set the connector state to closed.
	d->connectorState = IpEndpointConnector::Closed;
	// Signal that the connector has closed.
	emit closed();
}

//BEGIN Socket Event Handling Functions

void IpEndpointConnector::socket_OnClose()
{
	// Disconnect all signal/slot.
	QObject::disconnect(d->socket, 0, this, 0);

	qDebug("%s: Connection to (addr=%s port=%d) closed", Q_FUNC_INFO,
		qPrintable(d->ipAddress), d->port);

	// Close the connector.
	closeConnector();
}

void IpEndpointConnector::socket_OnConnect()
{
	// Set the connector state to connected.
	d->connectorState = IpEndpointConnector::Connected;

	qDebug("%s: Connected to (addr=%s port=%d)", Q_FUNC_INFO,
		qPrintable(d->ipAddress), d->port);

	if (d->underlyingSocketUsesTls)
	{
		// If the underlying socket uses TLS, try to start authentication;
		// Connect the signal/slot.
		QObject::connect(d->socket, SIGNAL(encrypted()), this,
		SLOT(socket_OnTlsConnect()));
		QObject::connect(d->socket, SIGNAL(sslErrors(const QList<QSslError>&)), this,
		SLOT(socket_OnTlsError(const QList<QSslError>&)));

		qDebug("%s: starting client authentication.", Q_FUNC_INFO);
		// Start the authentication as the client.
		d->socket->startClientEncryption();
	}
	else
	{
		// Otherwise, signal that the connector is connected.
		emit connected();
	}
}

void IpEndpointConnector::socket_OnError(QAbstractSocket::SocketError socketError)
{
	Q_UNUSED(socketError);

	// Set the connector state to faulted.
	d->connectorState = IpEndpointConnector::Faulted;

	qDebug("%s: error=%s", Q_FUNC_INFO, qPrintable(d->socket->errorString()));

	// Disconnect the signal/slot
	QObject::disconnect(d->socket, SIGNAL(error(QAbstractSocket::SocketError)), this,
	SLOT(socket_OnError(QAbstractSocket::SocketError)));

	// Signal that the connector has faulted.
	emit faulted();
}

void IpEndpointConnector::socket_OnTlsConnect()
{
	qDebug("%s: network stream is now encrypted.", Q_FUNC_INFO);

	// Signal that the connector is connected.
	emit connected();
}

void IpEndpointConnector::socket_OnTlsError(const QList<QSslError>& errors)
{
	// Set the connector state to faulted.
	d->connectorState = IpEndpointConnector::Faulted;

    // Disconnect the signal/slot
	QObject::disconnect(d->socket, SIGNAL(sslErrors(const QList<QSslError>&)), this,
	SLOT(socket_OnTlsError(const QList<QSslError>&)));

	QSslError error;
	foreach(error, errors)
	{
		qDebug("%s: error=%s", Q_FUNC_INFO, qPrintable(error.errorString()));
	}

	// Signal that the connector has faulted.
	emit faulted();
}

//END

}

#include "ipendpointconnector.moc"
