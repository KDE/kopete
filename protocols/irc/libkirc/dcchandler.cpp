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

DCCChat::DCCChat(QHostAddress host, unsigned int port)
	: QSocket()
{
	mHost = host;
	mPort = port;
	connect(this, SIGNAL(connectionClosed()), this, SLOT(slotConnectionClosed()));
	connect(this, SIGNAL(readyRead()), this, SLOT(slotReadyRead()));
	connect(this, SIGNAL(delayedCloseFinished()), this, SLOT(slotConnectionClosed()));
	connect(this, SIGNAL(error(int)), this, SLOT(slotError(int)));
}

void DCCChat::dccAccept()
{
	connectToHost(mHost.toString(), mPort);
}

void DCCChat::dccCancel()
{
	slotConnectionClosed();
}

void DCCChat::slotError(int error)
{
	if (error == QSocket::ErrConnectionRefused || error == QSocket::ErrHostNotFound)
	{
		slotConnectionClosed();
	}
}

void DCCChat::slotReadyRead()
{
	while(canReadLine())
	{
		QString message = readLine();
		message.replace(QRegExp("[\\r\\n]*$"), "");
		emit incomingDccMessage(message, false);
	}
}

void DCCChat::slotConnectionClosed()
{
	emit terminating();
	delete this;
}

bool DCCChat::sendMessage(const QString &message)
{
	if (state() != QSocket::Connected)
	{
		return false;
	} else {
		QCString block = message.local8Bit();
		block.append("\n");
		writeBlock(block, block.length());
	}
	emit incomingDccMessage(message, true);
	// Put the return here to make G++ stay quiet
	return true;
}

DCCSend::DCCSend(const QString &)
	: QServerSocket()
{

}

void DCCSend::newConnection(int socket)
{

}
