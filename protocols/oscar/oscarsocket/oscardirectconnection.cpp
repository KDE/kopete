/*
    oscardirectconnection.cpp  -  Implementation of an oscar direct connection

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

#include <kdebug.h>
#include "oscardirectconnection.h"

OscarDirectConnection::OscarDirectConnection(const QString &sn,
	const QString &connName, const QByteArray &cookie,
	QObject *parent, const char *name)
	: OscarConnection(sn, connName, DirectIM, cookie, parent, name)
{
	connect(this, SIGNAL(connectionClosed()), this, SLOT(slotConnectionClosed()));
}

OscarDirectConnection::~OscarDirectConnection()
{
}

void OscarDirectConnection::slotRead(void)
{
	ODC2 fl = getODC2();
	char *buf = new char[fl.length];
	Buffer inbuf;

	if (bytesAvailable() < fl.length)
	{
		while (waitForMore(500) < fl.length)
		{
			kdDebug(14150) << k_funcinfo <<
				"Not enough data read yet... waiting" << endl;
		}
	}

	int bytesread = readBlock(buf,fl.length);
	if (bytesAvailable())
	{
		emit readyRead(); //there is another packet waiting to be read
	}

	inbuf.setBuf(buf,bytesread);

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

	if ( (fl.length > 0) && fl.sn) //there is a message here
		parseMessage(inbuf);

	if ( inbuf.length() )
		kdDebug(14150) << k_funcinfo << "'" << connectionName() <<
			"': inbuf not empty" << endl;

	if (fl.sn)
		delete fl.sn;

	if (fl.cookie)
		delete fl.cookie;
}

/** Gets an ODC2 header */
ODC2 OscarDirectConnection::getODC2(void)
{
	ODC2 odc;
	int theword, theword2, start, chan;

	//the ODC2 start byte
	if (
		((start = getch()) == 0x4f) &&
		((start = getch()) == 0x44) &&
		((start = getch()) == 0x43) &&
		((start = getch()) == 0x32)
		)
	{
		//get the header length
		if ((theword = getch()) == -1)
		{
			kdDebug(14150) << "[OSCAR] Error reading length, byte 1: nothing to be read" << endl;
			odc.headerLength = 0x00;
		}
		else if((theword2 = getch()) == -1)
		{
			kdDebug(14150) << "[OSCAR] Error reading data field length, byte 2: nothing to be read" << endl;
			odc.headerLength = 0x00;
		}
		else
		{
			odc.headerLength = (theword << 8) | theword2;
		}

		//convert header to a buffer
		char *buf = new char[odc.headerLength-6];  // the -6 is there because we have already read 6 bytes
		readBlock(buf,odc.headerLength-6);
		Buffer inbuf;
		inbuf.setBuf(buf,odc.headerLength-6);

		//get channel
		odc.channel = inbuf.getWord();
		//0x0006 is next
		if (inbuf.getWord() != 0x0006)
			kdDebug(14150) << "[OscarDirectConnection] getODC2: 1: expected a 0x0006, didn't get it" << endl;
		//0x0000 is next
		if (inbuf.getWord() != 0x0000)
			kdDebug(14150) << "[OscarDirectConnection] getODC2: 2: expected a 0x0000, didn't get it" << endl;

		//get the 8 byte cookie
		odc.cookie = inbuf.getBlock(8);

		//for (int i=0;i<8;i++)
		//	mCookie[i] = odc.cookie[i];

		// 10 bytes of 0
		if (inbuf.getDWord() != 0x00000000)
			kdDebug(14150) << "[OscarDirectConnection] getODC2: 3: expected a 0x00000000, didn't get it" << endl;
		if (inbuf.getDWord() != 0x00000000)
			kdDebug(14150) << "[OscarDirectConnection] getODC2: 4: expected a 0x00000000, didn't get it" << endl;
		//if (inbuf.getWord() != 0x0000)
		//	kdDebug(14150) << "[OscarDirectConnection] getODC2: 5: expected a 0x0000, didn't get it" << endl;

		// message length
		odc.length = inbuf.getDWord();

		// 6 bytes of 0
		if (inbuf.getDWord() != 0x00000000)
			kdDebug(14150) << "[OscarDirectConnection] getODC2: 6: expected a 0x00000000, didn't get it" << endl;
		if (inbuf.getWord() != 0x0000)
			kdDebug(14150) << "[OscarDirectConnection] getODC2: 7: expected a 0x0000, didn't get it" << endl;

		// ODC2 type
		odc.type = inbuf.getWord();

		// 4 bytes of 0
		if (inbuf.getDWord() != 0x00000000)
			kdDebug(14150) << "[OscarDirectConnection] getODC2: 8: expected a 0x00000000, didn't get it" << endl;

		// screen name (not sure how to get length yet, so we'll just take it and the 0's after it)
		odc.sn = inbuf.getBlock(inbuf.length());

		// 25 bytes of 0
		//might as well just clear buffer, since there won't be anything after those 25 0x00's
		inbuf.clear();
	}
	else
	{
		kdDebug(14150) << "[OSCAR] Error reading ODC2 header... start byte is " << start << endl;
	}

	kdDebug(14150) << "[OSCAR] Read an ODC2 header!  header length: " << odc.headerLength
		<< ", channel: " << odc.channel << ", message length: " << odc.length
		<< ", type: " << odc.type << ", screen name: " << odc.sn << endl;

	return odc;
}

/** Sends the direct IM message to buddy */
void OscarDirectConnection::sendIM(const QString &message, bool /*isAuto*/)
{
	sendODC2Block(message, 0x0000); // 0x0000 means message
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
	outbuf.addString(cookie().data(),8);
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

	while (outbuf.length() < 0x004c)
		outbuf.addByte(0x00);

	if (typingnotify == 0x0000)
		outbuf.addString(message.latin1(), message.length());

	kdDebug(14150) << k_funcinfo << "Sending ODC2 block, message: " <<
		message << "typingnotify: " << typingnotify << endl;
	//outbuf.print();

	writeBlock(outbuf.buffer(), outbuf.length());
}

void OscarDirectConnection::parseMessage(Buffer &inbuf)
{
	kdDebug(14150) << k_funcinfo << "Buffer length is " << inbuf.length() << endl;
	// The message will come first, followed by binary files
	// so let's parse until we see "<BINARY>"
	QString message;
	while (!message.contains("<BINARY>",false))
	{
		//kdDebug(14150) << "[OscarDirect] message is: " << message << endl;
		// while the message does not contain the string "<BINARY>"
		message.append(inbuf.getByte());
		if(inbuf.length()==0)
		{
			//if we are at the end of the buffer
			kdDebug(14150) << k_funcinfo << "Got IM: " << message << endl;

			emit gotIM(message, connectionName(), false);
			return;
		}
	}
	//now, we have the message, and we need to d/l the binary content

	//TODO: implement a FOR loop to handle multiple binary file ID's

	//first comes the <DATA> tag
	//fields of <DATA ID="n" SIZE="n">(binary file here)
	QString datatag;
	while (!datatag.contains(">",false))
	{
		datatag.append(inbuf.getByte());
		kdDebug(14150) << k_funcinfo << "Datatag matching '" <<
			datatag << "'" << endl;

		if(!inbuf.length())
		{
			//message ended in the middle of the data tag
			kdDebug(14150) << k_funcinfo << "Got IM: " << message << endl;
			//remove the <BINARY> at the end of the string
			emit gotIM(message.remove(message.length()-8, 8),
				connectionName(), false);
		}
	}

	kdDebug(14150) << k_funcinfo << "Got IM: " << message << endl;
	emit gotIM(message.remove(message.length()-8, 8), connectionName(), false);
}

#include "oscardirectconnection.moc"
// vim: set noet ts=4 sts=4 sw=4:
