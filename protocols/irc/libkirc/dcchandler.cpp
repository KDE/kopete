/*
    dcchandler.cpp - DCC Handler

    Copyright (c) 2002      by Nick Betcher <nbetcher@kde.org>

    Coutesy of Szymon (from KVirc) for many of the ideas
    (from the source) on how to implement this!
    ...but I didn't copy his code :)

    Kopete    (c) 2002      by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "dcchandler.h"

#include <kglobal.h>
#include <klocale.h>

#include <qfile.h>
#include <qregexp.h>
#include <qtextcodec.h>

#ifdef NEED_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif
#ifndef __FreeBSD__
#  include <netinet/in.h>
#else
#  include <arpa/inet.h>
#endif
#include <unistd.h>
#include <stdint.h>

DCCClient::DCCClient(QHostAddress host, unsigned int port, unsigned int size, Type type)
	: QSocket()
{
	mType = type;
	mHost = host;
	mPort = port;
	mSize = size;
	mFile = 0L;
	connect(this, SIGNAL(connectionClosed()), this, SLOT(slotConnectionClosed()));
	if (mType == Chat)
	{
		connect(this, SIGNAL(readyRead()), this, SLOT(slotReadyRead()));
	} else {
		connect(this, SIGNAL(readyRead()), this, SLOT(slotReadyReadFile()));
	}
	connect(this, SIGNAL(delayedCloseFinished()), this, SLOT(slotConnectionClosed()));
	connect(this, SIGNAL(error(int)), this, SLOT(slotError(int)));
	codec = QTextCodec::codecForLocale();
	if (KGlobal::locale()->country() == "jp") {
		codec = QTextCodec::codecForName("iso-2022-jp");
	}
}

DCCClient::~DCCClient()
{
	delete mFile;
}

void DCCClient::dccAccept(const QString &filename)
{
	if (mType == File)
	{
		mFile = new QFile(filename);
		if (!mFile->open(IO_ReadWrite))
		{
			slotConnectionClosed();
			return;
		}
		connectToHost(mHost.toString(), mPort);
	} else {
		slotConnectionClosed();
	}
}

void DCCClient::dccAccept()
{
	if (mType == Chat)
	{
		connectToHost(mHost.toString(), mPort);
	} else {
		slotConnectionClosed();
	}
}

void DCCClient::dccCancel()
{
	slotConnectionClosed();
}

void DCCClient::slotError(int error)
{
	if (error == QSocket::ErrConnectionRefused || error == QSocket::ErrHostNotFound)
	{
		slotConnectionClosed();
	}
}

void DCCClient::slotReadyReadFile()
{
	int bytes = bytesAvailable();
	QCString data(bytes);
	int actualBytes = readBlock(data.data(), bytes);
	mFile->writeBlock(data.data(), actualBytes);
	uint32_t ack = htonl(mFile->at());
	writeBlock((char *)&ack, sizeof(ack));
	if (mSize != 0)
	{
		emit receiveAckPercent((mFile->at() * 100) / mSize);
	}
	if (mFile->size() == mSize)
	{
		emit sendFinished();
	}
}

void DCCClient::slotReadyRead()
{
	while(canReadLine())
	{
		// FIXME: readLine already returns a QString (i.e.
		// unicode, it makes no sense to call QTextCodec
		// methods on it at all! Use readBlock() instead.
		// - Martijn
		QString message = codec->toUnicode( readLine().utf8() );
		message.replace(QRegExp("[\\r\\n]*$"), "");
		emit incomingDccMessage(message, false);
	}
}

void DCCClient::slotConnectionClosed()
{
	if (mFile != 0)
	{
		if (mFile->size() != 0 && mSize != 0 && mFile->size() == mSize)
		{
			emit sendFinished();
			delete this;
			return;
		}
	}
	emit terminating();
	delete this;
}

bool DCCClient::sendMessage(const QString &message)
{
	if (mType == File)
	{
		return false;
	}
	if (state() != QSocket::Connected)
	{
		return false;
	} else {
		QCString block = codec->fromUnicode(message);
		block.append("\r\n");
		writeBlock(block, block.length());
	}
	emit incomingDccMessage(message, true);
	// Put the return here to make G++ stay quiet
	return true;
}

DCCServer::DCCServer(Type type, const QString filename)
	: QServerSocket((short unsigned int)0)
{
	mType = type;
	if (type == Chat)
	{
		mClient = new DCCClient(QHostAddress(), 0, 0, DCCClient::Chat);
	} else if (type == File)
	{
		if (filename.isEmpty())
		{
			// This shouldn't happen, and if it does, it's your own damn fault
			delete this;
			return;
		} else {
			mFile = new QFile(filename);
			mSocket = new QSocket();
		}
	}
}

DCCServer::~DCCServer()
{
	delete mFile;
	delete mSocket;
}


void DCCServer::newConnection(int socket)
{
	if (mType == Chat)
	{
		mClient->setSocket(socket);
		connect(mClient, SIGNAL(connectionClosed()), mClient, SLOT(slotConnectionClosed()));
		connect(mClient, SIGNAL(readyRead()), mClient, SLOT(slotReadyRead()));
		connect(mClient, SIGNAL(delayedCloseFinished()), mClient, SLOT(slotConnectionClosed()));
		connect(mClient, SIGNAL(error(int)), mClient, SLOT(slotError(int)));
	} else if (mType == File)
	{
		if (!mFile->open(IO_ReadOnly))
		{
			emit readAccessDenied();
			delete this;
			return;
		}
		mSocket->setSocket(socket);
		connect(mSocket, SIGNAL(connectionClosed()), this, SLOT(slotConnectionClosed()));
		connect(mSocket, SIGNAL(readyRead()), this, SLOT(slotReadyRead()));
		connect(mSocket, SIGNAL(delayedCloseFinished()), this, SLOT(slotConnectionClosed()));
		connect(mSocket, SIGNAL(error(int)), this, SLOT(slotError(int)));
		sendNextPacket();
	}
	emit clientConnected();
}

void DCCServer::abort()
{
	mSocket->close();
	slotConnectionClosed();
}

void DCCServer::slotConnectionClosed()
{
	emit terminating();
	delete this;
}

void DCCServer::slotError(int error)
{
	if (error == QSocket::ErrConnectionRefused || error == QSocket::ErrHostNotFound)
	{
		slotConnectionClosed();
	}
}

void DCCServer::slotReadyRead()
{
	uint32_t ack;

	mSocket->readBlock((char*)&ack, sizeof(ack));
	ack = ntohl(ack);

	int percentage = (ack * 100 / mFile->size());

	emit incomingAckPercent(percentage);

	if (mFile->atEnd())
		emit sendFinished();
	else
		sendNextPacket();
}

void DCCServer::sendNextPacket()
{
	char data[1024];
	int len = mFile->readBlock(data, 1024);
	if (len != -1)
	{
		// Use data.length(), not 1024 because we might not have 1024 bytes (if we are at the end of the file for example)
		mSocket->writeBlock(data, len);
		int percentage = (mFile->at() * 100 / mFile->size());
		emit sendingNonAckPercent(percentage);
	}
}

#include "dcchandler.moc"
/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

