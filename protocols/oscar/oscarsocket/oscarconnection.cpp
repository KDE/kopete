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
#include <qobject.h>
#include <kdebug.h>

OscarConnection::OscarConnection(const QString &sn, const QString &connName,
	ConnectionType type, const QByteArray &cookie, QObject *parent, const char *name) :
	QObject(parent, name)
{
//	kdDebug(14150) << k_funcinfo <<  "called, sn='" << sn << "' connName='" << connName << "'" << endl;

	mConnName = connName;
	mConnType = type;
	mSN = sn;
	mCookie.duplicate(cookie);
	mSocket = new KExtendedSocket();
	mSocket->setSocketFlags(KExtendedSocket::inetSocket | KExtendedSocket::bufferedSocket);

	connect(mSocket, SIGNAL(connectionSuccess()), this, SLOT(slotConnected()));
	connect(mSocket, SIGNAL(connectionFailed(int)), this, SLOT(slotError(int)));
}

OscarConnection::~OscarConnection()
{
//	kdDebug(14150) << k_funcinfo << "Called." << endl;
}

/* Called when there is data to be read.
 * If you want your connection to be able to receive data, you
 * should override this
 * No need to connect the signal in derived classes, just override this slot
 */
void OscarConnection::slotRead()
{
	kdDebug(14150) << k_funcinfo << mSocket->bytesAvailable() <<
		" bytes, connection name='" << mConnName << "'" << endl;

	Buffer inbuf;
	int len = mSocket->bytesAvailable();
	char *buf = new char[len];
	mSocket->readBlock(buf,len);
	inbuf.setBuf(buf,len);
//	inbuf.print();
	delete buf;
}

void OscarConnection::slotError(int errornum)
{
	kdDebug(14150) << k_funcinfo << mSocket->strError(mSocket->status(), errornum) << endl;
	slotConnectionClosed();
}

void OscarConnection::setSN(const QString &newSN)
{
	mSN = newSN;
}

void OscarConnection::sendIM(const QString &/*message*/, bool /*isAuto*/)
{}

void OscarConnection::sendTypingNotify(TypingNotify /*notifyType*/)
{
	kdDebug(14150) << k_funcinfo <<
		"Not implemented in this object! " << endl;
}

void OscarConnection::slotConnected()
{
	kdDebug(14150) << k_funcinfo << "Connected" << endl;

//	mSocket->enableRead(true);
//	mSocket->enableWrite(true);
//	mSocket->setBufferSize(-1);
	connect(mSocket, SIGNAL(readyRead()), this, SLOT(slotRead()));

	// Announce that we are ready for use, if it's not the server socket
	if(mConnType != Server)
		emit connectionReady(connectionName());
}

/** Called when the connection is closed */
void OscarConnection::slotConnectionClosed()
{
	kdDebug(14150) << k_funcinfo << "connection with '" <<
		connectionName() << "' lost." << endl;

	emit protocolError(QString("Connection with %1 lost").arg(mSocket->host()), 1);
	emit connectionClosed(connectionName());
}

/** Sends request to the client telling he/she that we want to send this file */
void OscarConnection::sendFileSendRequest()
{
	kdDebug(14150) << k_funcinfo <<
		"Not implemented in this object! " << endl;
}

#include "oscarconnection.moc"
// vim: set noet ts=4 sts=4 sw=4:
