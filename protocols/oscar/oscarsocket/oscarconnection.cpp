/*
    oscarconnection.h  -  Implementation of an oscar connection

    Copyright (c) 2002 by Tom Linsky <twl6@po.cwru.edu>
    Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "oscarconnection.h"
#include <kdebug.h>

OscarConnection::OscarConnection(const QString &sn, const QString &connName,
	ConnectionType type, const QByteArray &cookie, QObject *parent, const char *name)
	: QSocket(parent, name)
{
	kdDebug(14150) << k_funcinfo <<  "called, sn='" << sn << "' connName='" << connName << "'" << endl;

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

/* Called when there is data to be read.
 * If you want your connection to be able to receive data, you
 * should override this
 * No need to connect the signal in derived classes, just override this slot
 */
void OscarConnection::slotRead()
{
	kdDebug(14150) << k_funcinfo << bytesAvailable() <<
		" bytes, connection name='" << mConnName << "'" << endl;

	Buffer inbuf;
	int len = bytesAvailable();
	char *buf = new char[len];
	readBlock(buf,len);
	inbuf.setBuf(buf,len);
	inbuf.print();

	delete buf;
}

void OscarConnection::slotError(int errornum)
{
	switch(errornum)
	{
		case QSocket::ErrConnectionRefused:
		{
			kdDebug(14150) << k_funcinfo << "Connection refused." << endl;
			slotConnectionClosed();
			break;
		}
		case QSocket::ErrHostNotFound:
		{
			kdDebug(14150) << k_funcinfo << "Host not found." << endl;
			slotConnectionClosed();
			break;
		}
		case QSocket::ErrSocketRead:
		{
			kdDebug(14150) << k_funcinfo << "Problem with reading socket." <<
				" Problems may be present from here on out..." << endl;
			break;
		}
	}
}

/** Sets the currently logged in user's screen name */
void OscarConnection::setSN(const QString &newSN)
{
	mSN = newSN;
}

void OscarConnection::sendIM(const QString &/*message*/, bool /*isAuto*/)
{}

/** Sends a typing notification to the server
		@param notifyType Type of notify to send
 */
void OscarConnection::sendTypingNotify(TypingNotify /*notifyType*/)
{
	kdDebug(14150) << k_funcinfo <<
		"Not implemented in this object! " << endl;
}

/** Called when we have established a connection */
void OscarConnection::slotConnected()
{
	kdDebug(14150) << k_funcinfo <<
		"We are connected to '" <<
		connectionName() << "'" << endl;

	// Announce that we are ready for use, if it's not the server socket
	if(mConnType != Server)
		emit connectionReady(connectionName());
}

/** Called when the connection is closed */
void OscarConnection::slotConnectionClosed()
{
	kdDebug(14150) << k_funcinfo << "connection with '" <<
		connectionName() << "' lost." << endl;

	emit protocolError(QString("Connection with %1 lost").arg(connectionName()), 0);
	emit connectionClosed(connectionName());
}

/** Sends request to the client telling he/she that we want to send this file */
void OscarConnection::sendFileSendRequest()
{
	kdDebug(14150) << k_funcinfo <<
		"Not implemented in this object! " << endl;
}

/** Sets the socket to use socket, state() to connected, and emit connected() */
void OscarConnection::setSocket(int socket)
{
	QSocket::setSocket(socket);
	emit connected();
}

#include "oscarconnection.moc"
// vim: set noet ts=4 sts=4 sw=4:
