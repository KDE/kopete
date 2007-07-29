//
// IpEndpointConnector class
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

#ifndef PAPILLON_IPENDPOINTCONNECTOR_H
#define PAPILLON_IPENDPOINTCONNECTOR_H

#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QString>
#include <QtNetwork/QAbstractSocket>
#include <QtNetwork/QSslError>

namespace Papillon
{

class NetworkStream;
/** @class IpEndpointConnector <Papillon/Network/IpEndpointConnector>
	@brief Provides methods for connecting to an ip endpoint.*/
class IpEndpointConnector : public QObject
{
	Q_OBJECT

	public:
		/** @brief Defines the possible states of an endpoint connector.*/
		enum State { Created=0, Connecting=1, Connected=2, Closing=4, Closed=8, Faulted=0x10 };

	public:
		/** @brief Creates a new instance of the IpEndpointConnector class. */
		IpEndpointConnector(bool enableTls, QObject *parent=0);
		/** @brief Frees resources and performs other cleanup operations. */
		~IpEndpointConnector();

	public:
		/** @brief Closes the endpoint connector. */
		void close();
		/** @brief Connects the endpoint connector using the specified ipAddress and port. */
		void connectWithAddressInfo(const QString& ipAddress, const quint16 port);
		/** @brief Gets the underlying data stream between the connected ip endpoints. */
		NetworkStream * networkStream();
		/** @brief Gets the state of the endpoint connector. */
		const State & state() const;

	signals:
		/** @brief Indicates that the endpoint connector is connecting. */
		void connecting();
		/** @brief Indicates that the endpoint connector is connected. */
		void connected();
		/** @brief Indicates that the endpoint connector is closing.*/
		void closing();
		/** @brief Indicates that the endpoint connector is closed.*/
		void closed();
		/** @brief Indicates that the endpoint connector has faulted.*/
		void faulted();

	private Q_SLOTS:
		/** @brief Called when the endpoint connector's underlying socket closes. */
		void socket_OnClose();
		/** @brief Called when the endpoint connector's underlying socket connects. */
		void socket_OnConnect();
		/** @brief Called when the endpoint connector's underlying socket has faulted. */
		void socket_OnError(QAbstractSocket::SocketError socketError);
		/** @brief Called when the endpoint connector's TLS authentication is complete. */
		void socket_OnTlsConnect();
		/** @brief Called when an error occurs during TLS handshake. */
		void socket_OnTlsError(const QList<QSslError>& errors);

	private:
		/** @brief Gracefully closes the endpoint connector.*/
		void closeConnector();

	private:
		class IpEndpointConnectorPrivate;
		IpEndpointConnectorPrivate *d;

};	// IpEndpointConnector
}

#endif
