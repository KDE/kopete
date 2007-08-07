//
// ByteStreamBase class
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

#ifndef PAPILLON_BYTESTREAM_H
#define PAPILLON_BYTESTREAM_H

#include <QtCore/QObject>
#include <QtCore/QByteArray>

namespace Papillon
{

/** @class ByteStream <Papillon/Base/ByteStreamBase>
	@brief Provides the base implementation for all ByteStreams. */
class ByteStreamBase : public QObject
{
	Q_OBJECT

	public:
		/** @brief Creates a new instance of the ByteStream class. */
		ByteStreamBase(QObject *parent);
		/** @brief Frees resources and performs other cleanup operations. */
		~ByteStreamBase();

	public:
		/** @brief Returns the number of incoming bytes that are waiting to be read. */
		virtual qint64 bytesAvailable() const;
		/** @brief Returns the number of bytes that are waiting to be written. */
		virtual qint64 bytesToWrite() const;
		/** @brief When overriden in a derived class, returns a value indicating whether the ByteStream is open. */
		virtual bool isOpen() const = 0;
		/** @brief Closes the stream. */
		virtual void close();
		/** @brief When overriden in a derived class, returns a byte array containing count bytes read from the stream. */
		virtual QByteArray read(qint64 count) = 0;
		/** @brief When overriden in a derived class, writes the contents of the buufer to the stream. */
		virtual qint64 write(const QByteArray & buffer) = 0;

	signals:
		void bytesWritten(const qint64 count);
		void readyRead();

}; // ByteStream
}

#endif
