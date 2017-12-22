/*
 * bytestream.h - base class for bytestreams
 * Copyright (C) 2003  Justin Karneges <justin@affinix.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef CS_BYTESTREAM_H
#define CS_BYTESTREAM_H

#include <QObject>
#include "libgroupwise_export.h"
class QByteArray;

// CS_NAMESPACE_BEGIN

// CS_EXPORT_BEGIN
class LIBGROUPWISE_EXPORT ByteStream : public QObject
{
    Q_OBJECT
public:
    enum Error {
        ErrRead, ErrWrite, ErrCustom = 10
    };
    ByteStream(QObject *parent = nullptr);
    virtual ~ByteStream() = 0;

    virtual bool isOpen() const;
    virtual void close();
    virtual void write(const QByteArray &);
    virtual QByteArray read(int bytes = 0);
    virtual int bytesAvailable() const;
    virtual int bytesToWrite() const;

    static void appendArray(QByteArray *a, const QByteArray &b);
    static QByteArray takeArray(QByteArray *from, int size = 0, bool del = true);

Q_SIGNALS:
    void connectionClosed();
    void delayedCloseFinished();
    void readyRead();
    void bytesWritten(int);
    void error(int);

protected:
    void clearReadBuffer();
    void clearWriteBuffer();
    void appendRead(const QByteArray &);
    void appendWrite(const QByteArray &);
    QByteArray takeRead(int size = 0, bool del = true);
    QByteArray takeWrite(int size = 0, bool del = true);
    QByteArray &readBuf();
    QByteArray &writeBuf();
    virtual int tryWrite();

private:
//! \if _hide_doc_
    class Private;
    Private *const d;
//! \endif
};
// CS_EXPORT_END

// CS_NAMESPACE_END

#endif
