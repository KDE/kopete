/***************************************************************************
                          dcchandler.cpp  -  description
                             -------------------
    begin                : Fri Apr 19 2002
    copyright            : (C) 2002 by nbetcher
    email                : nbetcher@usinternet.com
    Coutesy of Szymon (from KVirc) for many of the ideas (from the source) on how to implement this!
    ...but I didn't copy his code :)
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "dcchandler.h"
#include <kdebug.h>
#include <qdatastream.h>
#include <qregexp.h>
#include <netinet/in.h>
#include <qcstring.h>

DCCClient::DCCClient(QHostAddress host, unsigned int port, Type type)
	: QSocket()
{
	mHost = host;
	mPort = port;
	connect(this, SIGNAL(connectionClosed()), this, SLOT(slotConnectionClosed()));
	connect(this, SIGNAL(readyRead()), this, SLOT(slotReadyRead()));
	connect(this, SIGNAL(delayedCloseFinished()), this, SLOT(slotConnectionClosed()));
	connect(this, SIGNAL(error(int)), this, SLOT(slotError(int)));
}

void DCCClient::dccAccept()
{
	connectToHost(mHost.toString(), mPort);
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

void DCCClient::slotReadyRead()
{
	while(canReadLine())
	{
		QString message = readLine();
		message.replace(QRegExp("[\\r\\n]*$"), "");
		emit incomingDccMessage(message, false);
	}
}

void DCCClient::slotConnectionClosed()
{
	emit terminating();
	delete this;
}

bool DCCClient::sendMessage(const QString &message)
{
	if (state() != QSocket::Connected)
	{
		return false;
	} else {
		QCString block = message.local8Bit();
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
		mClient = new DCCClient(QHostAddress(), 0, DCCClient::Chat);
	} else if (type == File)
	{
		if (filename.isEmpty())
		{
			// This shouldn't happen, and if it does, it's your own damn fault
			delete this;
		} else {
			mFile = new QFile(filename);
			mSocket = new QSocket();
		}
	}
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
			delete this;
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
	kdDebug() << "IRC Plugin: slotReadyRead() mSocket->canReadLine() == " << mSocket->canReadLine() << endl;
	int available = mSocket->bytesAvailable();
	if (available != 0)
	{
		QCString data;
		data.resize(available);
		mSocket->readBlock(data.data(), available);
		unsigned long int ack;
		for (int i = 0; i != 5; i++)
		{
			kdDebug() << "IRC Plugin: data.data()[i] == " << data.data()[i] << endl;
		}
		QString percentage = QString::number(((ack * 100) / mFile->size())); // people ask me if I like lisp, I just ask them "What's Lisp?" :)
		kdDebug() << "IRC Plugin: percentage == " << percentage << endl;
		emit incomingAckPercent(percentage);
		kdDebug() << "IRC Plugin: mFile->atEnd() == " << mFile->atEnd() << endl;
		if (mFile->atEnd())
		{
			emit sendFinished();
		} else {
			sendNextPacket();
		}
	}
}

void DCCServer::sendNextPacket()
{
	//QCString data = ""; // Assign it to "" so it at least initializes the array internally
	char data[1024];
	int len = mFile->readBlock(data, 1024);
	if (len != -1)
	{
		// Use data.length(), not 1024 because we might not have 1024 bytes (if we are at the end of the file for example)
		mSocket->writeBlock(data, len);
		QString percentage = QString::number(((mFile->at() * 100) / mFile->size()));
		kdDebug() << "IRC Plugin: sendNextPacket: QString::number(((mFile->at() * 100) / mFile->size())) == " << QString::number(((mFile->at() * 100) / mFile->size())) << endl;
		emit sendingNonAckPercent(percentage);
	}
}
