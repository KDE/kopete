/*
    oscarconnection.h  -  Implementation of an oscar connection

    Copyright (c) 2002 by Tom Linsky <twl6@po.cwru.edu>

    Kopete    (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include <kdebug.h>
#include "oscardebugdialog.h"
#include "oscarconnection.h"

OscarConnection::OscarConnection(const QString &sn, const QString &connName,
	ConnectionType type, const QByteArray &cookie, QObject *parent, const char *name)
	: QSocket(parent, name)
{
	mConnName = connName;
	mConnType = type;
	mSN = sn;
	mCookie.duplicate(cookie);

	connect(this, SIGNAL(readyRead()), this, SLOT(slotRead()));
	connect(this, SIGNAL(connected()), this, SLOT(slotConnected()));
	connect(this, SIGNAL(error(int)), this, SLOT(slotError(int)));
}

OscarConnection::~OscarConnection()
{
}

/** Called when there is data to be read.
		If you want your connection to be able to receive data, you
		should override this
		No need to connect the signal in derived classes, just override this slot */
void OscarConnection::slotRead()
{
	kdDebug(14150) << "[OSCAR] OscarConnection: in slotRead(), " << bytesAvailable() <<
		" bytes, name: " << mConnName << endl;
	Buffer inbuf;
	int len = bytesAvailable();
	char *buf = new char[len];
	readBlock(buf,len);
	inbuf.setBuf(buf,len);
	inbuf.print();

	if(hasDebugDialog())
		debugDialog()->addMessageFromServer(inbuf.toString(),mConnName);

	delete buf;
}

void OscarConnection::slotError(int errornum)
{
	switch(errornum)
	{
		case QSocket::ErrConnectionRefused:
		{
			kdDebug(14150) << "[OSCAR] OscarConnection: in slotError() and error is connection refused." << endl;
			slotConnectionClosed();
			break;
		}
		case QSocket::ErrHostNotFound:
		{
			kdDebug(14150) << "[OSCAR] OscarConnection: in slotError() and error is host not found." << endl;
			slotConnectionClosed();
			break;
		}
		case QSocket::ErrSocketRead:
		{
			kdDebug(14150) << "[OSCAR] OscarConnection: in slotError() and error is problem with reading socket. Problems may be present from here on out..." << endl;
			break;
		}
	}
}

void OscarConnection::setDebugDialog(OscarDebugDialog *dialog)
{
	if(dialog)
	{
		mDebugDialog = dialog;
		mHaveDebugDialog = true;
	}
	else
	{
		mHaveDebugDialog = false;
	}
}

/** Sets the currently logged in user's screen name */
void OscarConnection::setSN(const QString &newSN)
{
	mSN = newSN;
}

/** Sends the direct IM message to buddy */
void OscarConnection::sendIM(const QString &/*message*/, bool /*isAuto*/)
{
	kdDebug(14150) << "[OscarConnection] sendIM not implemented in this object! " << endl;
}

/** Sends a typing notification to the server
		@param notifyType Type of notify to send
 */
void OscarConnection::sendTypingNotify(TypingNotify /*notifyType*/)
{
	kdDebug(14150) << "[OscarConnection] sendTypingNotify not implemented in this object! " << endl;
}

/** Called when we have established a connection */
void OscarConnection::slotConnected(void)
{
	kdDebug(14150) << "[OscarConnection] We are connected to " << connectionName() << endl;
	// Announce that we are ready for use, if it's not the server socket
	if ( mConnType != Server )
		emit connectionReady(connectionName());
}

/** Called when the connection is closed */
void OscarConnection::slotConnectionClosed(void)
{
	kdDebug(14150) << "[OscarDirectConnection] connection with " << connectionName() << "lost." << endl;
	emit protocolError(QString("Connection with %1 lost").arg(connectionName()), 0);
	emit connectionClosed(connectionName());
}

/** Sends request to the client telling he/she that we want to send this file */
void OscarConnection::sendFileSendRequest(void)
{
	kdDebug(14150) << k_funcinfo << "sendFileSendRequest not implemented in this object! " << endl;
}

/** Sets the socket to use socket, state() to connected, and emit connected() */
void OscarConnection::setSocket( int socket )
{
	QSocket::setSocket(socket);
	emit connected();
}

#include "oscarconnection.moc"
