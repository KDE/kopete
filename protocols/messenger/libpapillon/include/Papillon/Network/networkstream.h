//
// NetworkStream class
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

#ifndef PAPILLON_NETWORKSTREAM_H
#define PAPILLON_NETWORKSTREAM_H

#include <Papillon/Base/ByteStreamBase>
class QTcpSocket;

namespace Papillon
{

/** @class NetworkStream <Papillon/Network/NetworkStream>
	@brief Provides methods to access the underlying network socket's data stream. */
class NetworkStream : public ByteStreamBase
{
	Q_OBJECT

	public:
		/** @brief Creates a new instance of the NetworkStream class. */
		NetworkStream(QTcpSocket *socket, bool ownsSocket, QObject *parent);
		/** @brief Frees resources and performs other cleanup operations. */
		virtual ~NetworkStream();

	public:
		/** @brief Returns the number of incoming bytes that are waiting to be read. */
		qint64 bytesAvailable() const;
		/** @brief Returns a value which indicates whether the network stream is open. */
		bool isOpen() const;
		/** @brief Closes the stream. */
		void close();
		/** @brief Returns a byte array containing 'count' bytes read from the stream. */
		QByteArray read(qint64 count);
		/** @brief Returns a byte array containing all data available from the stream. */
		QByteArray readAll();
		/** @brief Writes the contents of the supplied byte array to the stream. */
		qint64 write(const QByteArray & buffer);

	private Q_SLOTS:
		/** @brief Called when the stream's underlying socket has data to be read. */
		void socket_OnRead();

	private:
		class NetworkStreamPrivate;
		NetworkStreamPrivate *d;

}; // NetworkStream
}

#endif
