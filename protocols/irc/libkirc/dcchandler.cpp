/***************************************************************************
                          dcchandler.cpp  -  description
                             -------------------
    begin                : Fri Apr 19 2002
    copyright            : (C) 2002 by nbetcher
    email                : nbetcher@usinternet.com
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

DCCServer::DCCServer(Type type)
	: QServerSocket((short unsigned int)0)
{
	mClient = new DCCClient(QHostAddress(), 0, DCCClient::Chat);
}

void DCCServer::newConnection(int socket)
{
	mClient->setSocket(socket);
	connect(mClient, SIGNAL(connectionClosed()), mClient, SLOT(slotConnectionClosed()));
	connect(mClient, SIGNAL(readyRead()), mClient, SLOT(slotReadyRead()));
	connect(mClient, SIGNAL(delayedCloseFinished()), mClient, SLOT(slotConnectionClosed()));
	connect(mClient, SIGNAL(error(int)), mClient, SLOT(slotError(int)));
	emit clientConnected();
}
