/*
    oscarconnection.h  -  Implementation of an oscar direct connection

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
#include "oscardirectconnection.h"
#include "oscardebugdialog.h"
#include "oscarsocket.h"

OscarDirectConnection::OscarDirectConnection(OscarSocket *serverconn, const QString &connName,
	QObject *parent, const char *name)
	: OscarConnection(connName, CONN_TYPE_DIRECTIM, parent, name)
{
	if (serverconn)
		mMainConn = serverconn;
	else
		kdDebug() << "[OscarDirectConnection] serverconn is NULL!!!  BAD!" << endl;
	connect(this, SIGNAL(connected()), this, SLOT(slotConnected()));
	connect(this, SIGNAL(connectionClosed()), this, SLOT(slotConnectionClosed()));
	setSN(mMainConn->getSN());
}

OscarDirectConnection::~OscarDirectConnection()
{
}

/** Called when there is data to be read from peer */
void OscarDirectConnection::slotRead(void)
{
	ODC2 fl = getODC2();
	char *buf = new char[fl.length];
	Buffer inbuf;

	if (bytesAvailable() < fl.length)
	{
		while (waitForMore(500) < fl.length)
			kdDebug() << "[OSCAR][OnRead()] not enough data read yet... waiting" << endl;
	}

	int bytesread = readBlock(buf,fl.length);
	if (bytesAvailable())
	{
		emit readyRead(); //there is another packet waiting to be read
	}

	inbuf.setBuf(buf,bytesread);

	//kdDebug() << "[OSCAR] Input: " << endl;
 	if(hasDebugDialog()){
			debugDialog()->addMessageFromServer(inbuf.toString(),connectionName());
	}

	fl.message = inbuf.getBlock(fl.length);

	if ( inbuf.getLength() )
		kdDebug() << "[OscarDirectConnection] slotread (" << connectionName() << "): inbuf not empty" << inbuf.toString() << endl;

	if (fl.type == 0x000e) // started typing
	{
		emit gotMiniTypeNotification(fl.sn, 2);
	}
	else if (fl.type == 0x0002) //finished typing
	{
		emit gotMiniTypeNotification(fl.sn, 0);
	}
	else
	{
		emit gotMiniTypeNotification(fl.sn, 1);
	}
	if ( (fl.length > 0) && fl.message && fl.sn)
		emit gotIM(fl.message, fl.sn, false);
		
  if (fl.sn)
  	delete fl.sn;
  if (fl.cookie)
  	delete fl.cookie;
  if (fl.message)
  	delete fl.message;
}

/** Gets an ODC2 header */
ODC2 OscarDirectConnection::getODC2(void)
{
	ODC2 odc;
	int theword, theword2;
  int start;
  int chan;
	//the ODC2 start byte
  if (((start = getch()) == 0x4f) &&
    		((start = getch()) == 0x44) &&
      	((start = getch()) == 0x43) &&
        ((start = getch()) == 0x32))
	{
		//get the header length
		if ((theword = getch()) == -1) {
			kdDebug() << "[OSCAR] Error reading length, byte 1: nothing to be read" << endl;
			odc.headerLength = 0x00;
		} else if((theword2 = getch()) == -1){
			kdDebug() << "[OSCAR] Error reading data field length, byte 2: nothing to be read" << endl;
			odc.headerLength = 0x00;
		} else {
			odc.headerLength = (theword << 8) | theword2;
		}

		//convert header to a buffer
		char *buf = new char[odc.headerLength-6];  // the -6 is there because we have already read 6 bytes
		readBlock(buf,odc.headerLength-6);
		Buffer inbuf;
		inbuf.setBuf(buf,odc.headerLength-6);

		inbuf.print();
		if(hasDebugDialog()){
			debugDialog()->addMessageFromServer(inbuf.toString(),connectionName());
		}
		
		//get channel
		odc.channel = inbuf.getWord();
		//0x0006 is next
		if (inbuf.getWord() != 0x0006)
			kdDebug() << "[OscarDirectConnection] getODC2: 1: expected a 0x0006, didn't get it" << endl;
		//0x0000 is next
		if (inbuf.getWord() != 0x0000)
			kdDebug() << "[OscarDirectConnection] getODC2: 2: expected a 0x0000, didn't get it" << endl;

		//get the 8 byte cookie
		odc.cookie = inbuf.getBlock(8);

		//for (int i=0;i<8;i++)
		//	mCookie[i] = odc.cookie[i];

		// 10 bytes of 0
		if (inbuf.getDWord() != 0x00000000)
			kdDebug() << "[OscarDirectConnection] getODC2: 3: expected a 0x00000000, didn't get it" << endl;
		if (inbuf.getDWord() != 0x00000000)
			kdDebug() << "[OscarDirectConnection] getODC2: 4: expected a 0x00000000, didn't get it" << endl;
		//if (inbuf.getWord() != 0x0000)
		//	kdDebug() << "[OscarDirectConnection] getODC2: 5: expected a 0x0000, didn't get it" << endl;

		// message length
		odc.length = inbuf.getDWord();

    // 6 bytes of 0
		if (inbuf.getDWord() != 0x00000000)
			kdDebug() << "[OscarDirectConnection] getODC2: 6: expected a 0x00000000, didn't get it" << endl;
		if (inbuf.getWord() != 0x0000)
			kdDebug() << "[OscarDirectConnection] getODC2: 7: expected a 0x0000, didn't get it" << endl;

		// ODC2 type
		odc.type = inbuf.getWord();

		// 4 bytes of 0
		if (inbuf.getDWord() != 0x00000000)
			kdDebug() << "[OscarDirectConnection] getODC2: 8: expected a 0x00000000, didn't get it" << endl;
		
		// screen name (not sure how to get length yet, so we'll just take it and the 0's after it)
		odc.sn = inbuf.getBlock(inbuf.getLength());

		// 25 bytes of 0
		
		//might as well just clear buffer, since there won't be anything after those 25 0x00's
		inbuf.clear();
  } else {
		kdDebug() << "[OSCAR] Error reading ODC2 header... start byte is " << start << endl;
	}

	kdDebug() << "[OSCAR] Read an ODC2 header!  header length: " << odc.headerLength
		<< ", channel: " << odc.channel << ", message length: " << odc.length
		<< ", type: " << odc.type << ", screen name: " << odc.sn << endl;

  return odc;
}

/** Called when we have established a connection */
void OscarDirectConnection::slotConnected(void)
{
 	// Connect protocol error signal
	QObject::connect(this, SIGNAL(protocolError(QString, int)),
			mMainConn, SLOT(OnDirectIMError(QString, int)));
	// Got IM
	QObject::connect(this, SIGNAL(gotIM(QString, QString, bool)),
			mMainConn, SLOT(OnDirectIMReceived(QString,QString,bool)));
	// Disconnected
	QObject::connect(this, SIGNAL(connectionClosed(OscarDirectConnection *)),
			mMainConn, SLOT(OnDirectIMConnectionClosed(OscarDirectConnection *)));
	// Typing notification
	QObject::connect(this, SIGNAL(gotMiniTypeNotification(QString,int)),
			mMainConn, SLOT(OnDirectMiniTypeNotification(QString, int)));
	// Ready signal
	QObject::connect(this, SIGNAL(directIMReady(QString)),
			mMainConn, SLOT(OnDirectIMReady(QString)));

	// Announce that we are ready for use!
	emit directIMReady(connectionName());
}

/** Sets the socket to use socket, state() to connected, and emit connected() */
void OscarDirectConnection::setSocket( int socket )
{
	QSocket::setSocket(socket);
	emit connected();
}

/** Sends the direct IM message to buddy */
void OscarDirectConnection::sendIM(const QString &message, bool /*isAuto*/)
{
	sendODC2Block(message, 0x0000); // 0x0000 means message
}

/** Called when the connection is closed */
void OscarDirectConnection::slotConnectionClosed(void)
{
	kdDebug() << "[OscarDirectConnection] connection with " << connectionName() << "lost." << endl;
	emit protocolError(QString("Connection with %1 lost").arg(connectionName()), 0);
	emit connectionClosed(this);
}

/** Sends a typing notification to the server
		@param notifyType Type of notify to send
	 */
void OscarDirectConnection::sendTypingNotify(TypingNotify notifyType)
{
	switch (notifyType)
	{
		case TypingBegun:
			sendODC2Block(QString::null, 0x000e);
			break;
		case TypingFinished:
			sendODC2Block(QString::null, 0x0002);
			break;
		case TextTyped:  //we will say TextTyped means the user has finished typing, for now
			sendODC2Block(QString::null, 0x0002);
			break;
	}
}

/** Prepares and sends a block with the given message and typing notify flag attached */
void OscarDirectConnection::sendODC2Block(const QString &message, WORD typingnotify)
{
	Buffer outbuf;
	outbuf.addDWord(0x4f444332); // "ODC2"
	outbuf.addWord(0x004c); // not sure if this is always the header length
	outbuf.addWord(0x0001); // channel
	outbuf.addWord(0x0006); // 0x0006
	outbuf.addWord(0x0000); // 0x0000
  outbuf.addString(mCookie,8);
	outbuf.addDWord(0x00000000);
	outbuf.addDWord(0x00000000);
	outbuf.addWord(0x0000);
	if (typingnotify == 0x0000)
		outbuf.addWord(message.length());
	else
		outbuf.addWord(0x0000);
  outbuf.addDWord(0x00000000);
  outbuf.addWord(0x0000);
  outbuf.addWord(typingnotify);
  outbuf.addDWord(0x00000000);
  outbuf.addString(getSN().latin1(),getSN().length());
  while (outbuf.getLength() < 0x004c)
  	outbuf.addByte(0x00);
  if (typingnotify == 0x0000)
	  outbuf.addString(message.latin1(), message.length());
  kdDebug() << "Sending ODC2 block, message: " << message << "typingnotify: " << typingnotify << endl;
  outbuf.print();

 	if(hasDebugDialog()){
			debugDialog()->addMessageFromClient(outbuf.toString(),connectionName());
	}

  writeBlock(outbuf.getBuf(),outbuf.getLength());
}
