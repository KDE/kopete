//
// NetworkStream
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

#include "Papillon/Network/NetworkStream"
#include <QtNetwork/QTcpSocket>
#include <QtDebug>

namespace Papillon
{

class NetworkStream::NetworkStreamPrivate
{
	public:
		NetworkStreamPrivate() : closed(false) {}

		bool closed;
		bool ownsSocket;
		QByteArray rbuffer;
		QTcpSocket *socket;
};

NetworkStream::NetworkStream(QTcpSocket *socket, bool ownsSocket, QObject *parent) : ByteStreamBase(parent), d(new NetworkStreamPrivate())
{
	Q_ASSERT(socket != 0l);

	d->socket = socket;
	// Connect the signal/slot
	QObject::connect(d->socket, SIGNAL(readyRead()), this,
	SLOT(socket_OnRead()));

	d->ownsSocket = ownsSocket;
}

NetworkStream::~NetworkStream()
{
	delete d;
	d = 0l;
}

qint64 NetworkStream::bytesAvailable() const
{
	return d->rbuffer.size();
}

bool NetworkStream::isOpen() const
{
	return d->closed != true;
}

void NetworkStream::close()
{
	// Determine whether the network stream
	// owns the underlying socket.
	if (d->ownsSocket)
	{
		// If so, close the socket.
		d->socket->close();
	}

	d->closed = true;
}

QByteArray NetworkStream::read(qint64 count)
{
	if (isOpen() == false)
	{
		qDebug("%s: stream not open", Q_FUNC_INFO);
		return QByteArray();
	}

	// Read count bytes from the buffer.
	QByteArray bytes = d->rbuffer.left(count);
	// Remove the read bytes from the buffer.
	d->rbuffer.remove(0, count);

	qDebug("%s: read %lld bytes", Q_FUNC_INFO, count);
 	return bytes;
}

QByteArray NetworkStream::readAll()
{
	// Return a byte array with all the available
	// data from the read buffer.
	return read(bytesAvailable());
}

qint64 NetworkStream::write(const QByteArray & buffer)
{
	qint64 count = -1;
	if (isOpen() == false)
	{
		qDebug("%s: stream not open", Q_FUNC_INFO);
		return count;
	}

	// Write the contents of the buffer to the socket.
	count = d->socket->write(buffer);

	qDebug("%s: sent %lld bytes", Q_FUNC_INFO, count);
	return count;
}

//BEGIN Socket Event Handling Functions

void NetworkStream::socket_OnRead()
{
	const qint64 count = d->socket->bytesAvailable();
	qDebug("%s: %lld bytes available", Q_FUNC_INFO, count);

	// If there is data to be read, write all received
	// data into the internal buffer.
	if (count > 0)
	{
		// Read all the data available from the socket.
		QByteArray bytes = d->socket->read(count);
		qDebug("%s: %i bytes read", Q_FUNC_INFO, bytes.size());

		// Append the read bytes to the buffer.
		d->rbuffer.append(bytes);
		// Signal that there is data to be read.
		emit readyRead();
	}
}

//END

}

#include "networkstream.moc"
