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
	ConnectionType type, char *cookie, QObject *parent, const char *name)
	: QSocket(parent, name)
{
	mConnName = connName;
	mConnType = type;
	mSN = sn;
  for (int i=0;i<8;i++)
	 	mCookie[i] = cookie[i];

  connect(this, SIGNAL(readyRead()), this, SLOT(slotRead()));
 	connect(this, SIGNAL(connected()), this, SLOT(slotConnected()));
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
	kdDebug(14150) << "[OSCAR] OscarConnection: in slotRead(), " << bytesAvailable() << " bytes, name: " << mConnName << endl;
  Buffer inbuf;
  int len = bytesAvailable();
	char *buf = new char[len];
	readBlock(buf,len);
	inbuf.setBuf(buf,len);
  inbuf.print();

	if(hasDebugDialog()){
			debugDialog()->addMessageFromServer(inbuf.toString(),mConnName);
	}

	delete buf;
}

void OscarConnection::setDebugDialog(OscarDebugDialog *dialog)
{
		if(dialog){
				mDebugDialog = dialog;
				mHaveDebugDialog = true;
		} else {
				mHaveDebugDialog = false;
		}
}

/** Sets the currently logged in user's screen name */
void OscarConnection::setSN(const QString &newSN)
{
	mSN = newSN;
}

/** Sends the direct IM message to buddy */
void OscarConnection::sendIM(const QString &message, bool isAuto)
{
	kdDebug() << "[OscarConnection] sendIM not implemented in this object! " << endl;
}

/** Sends a typing notification to the server
		@param notifyType Type of notify to send
 */
void OscarConnection::sendTypingNotify(TypingNotify notifyType)
{
	kdDebug() << "[OscarConnection] sendTypingNotify not implemented in this object! " << endl;
}

/** Called when we have established a connection */
void OscarConnection::slotConnected(void)
{
	kdDebug() << "[OscarConnection] We are connected to " << connectionName() << endl;
	// Announce that we are ready for use, if it's not the server socket
	if ( mConnType != Server )
		emit connectionReady(connectionName());
}

/** Called when the connection is closed */
void OscarConnection::slotConnectionClosed(void)
{
	kdDebug() << "[OscarDirectConnection] connection with " << connectionName() << "lost." << endl;
	emit protocolError(QString("Connection with %1 lost").arg(connectionName()), 0);
	emit connectionClosed(connectionName());
}

#include "oscarconnection.moc"
