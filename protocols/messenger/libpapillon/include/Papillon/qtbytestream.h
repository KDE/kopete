/*
   qtbytestream.h - ByteStream using QtNetwork's QTcpSocket.

   Copyright (c) 2006 by Michaël Larouche <larouche@kde.org>

   *************************************************************************
   *                                                                       *
   * This library is free software; you can redistribute it and/or         *
   * modify it under the terms of the GNU Lesser General Public            *
   * License as published by the Free Software Foundation; either          *
   * version 2 of the License, or (at your option) any later version.      *
   *                                                                       *
   *************************************************************************
*/
#ifndef PAPILLONQTBYTESTREAM_H
#define PAPILLONQTBYTESTREAM_H

#include <Papillon/Base/ByteStream>
#include <Papillon/Macros>

namespace Papillon {

/**
 * @class QtByteStream qtbytestream.h <Papillon/QtByteStream>
 * ByteStream for QTcpSocket.
 * 
 * @author Michaël Larouche
*/
class PAPILLON_EXPORT QtByteStream : public ByteStream
{
	Q_OBJECT
public:
	QtByteStream(QObject *parent = 0);
    ~QtByteStream();

	bool connect(const QString &host, quint16 port);
	virtual bool isOpen() const;
	virtual void close();

signals:
	void connected();

protected:
	virtual int tryWrite();

private slots:
	void slotConnected();
	void slotConnectionClosed();
	void slotReadyRead();
	void slotBytesWritten(qint64 bytes);
	void slotError(int);

private:
	class Private;
	Private *d;
};

}

#endif
