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

OscarConnection::OscarConnection(const QString &connName, int type,
	QObject *parent, const char *name)
	: QSocket(parent, name)
{
	mConnName = connName;
	mConnType = type;
  connect(this, SIGNAL(readyRead()), this, SLOT(slotRead()));
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
	kdDebug() << "[OSCAR] OscarConnection: in slotRead(), " << bytesAvailable() << " bytes, name: " << mConnName << endl;
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
#include "oscarconnection.moc"
