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
#include "oscardebug.h"
#include "oscardirectconnection.h"

OscarDirectConnection::OscarDirectConnection(const QString &sn,
	const QString &connName, const QByteArray &cookie, QObject *parent,
	const char *name)
		: OscarConnection(sn, connName, DirectIM, cookie, parent, name)
{
	kdDebug(14151) << k_funcinfo <<  "called, sn='" << sn << "' connName='" <<
		connName << "'" << endl;

	socket()->enableWrite(false); // don't spam us with readyWrite() signals
	socket()->enableRead(true);
	connect(this, SIGNAL(connectionClosed()), this, SLOT(slotConnectionClosed()));
}

OscarDirectConnection::~OscarDirectConnection()
{
	kdDebug(14151) << k_funcinfo << "Called." << endl;
}

void OscarDirectConnection::slotRead()
{
	ODC2 fl = getODC2();
	char *buf = new char[fl.length];
	Buffer inbuf;

	if (socket()->bytesAvailable() < (int)fl.length)
	{
		while (socket()->waitForMore(500) < (int)fl.length)
			kdDebug(14151) << k_funcinfo << "Not enough data read yet... waiting" << endl;
	}

	int bytesread = socket()->readBlock(buf,fl.length);
	inbuf.setBuf(buf, bytesread);
	QString screenName = QString::fromLatin1(fl.sn);

	if (fl.type == 0x000e) // started typing
		emit gotMiniTypeNotification(screenName, 2);
	else if (fl.type == 0x0002) //finished typing
		emit gotMiniTypeNotification(screenName, 0);
	else
		emit gotMiniTypeNotification(screenName, 1);

	if((fl.length > 0) && !screenName.isEmpty()) //there is a message here
		parseMessage(inbuf);

	if(inbuf.length() > 0)
		kdDebug(14151) << k_funcinfo << "'" << connectionName() <<
			"': inbuf not empty" << endl;

	delete fl.sn;
	delete fl.cookie;
}

ODC2 OscarDirectConnection::getODC2(void)
{
	ODC2 odc;
	int theword, theword2, start /*, chan */;

	//the ODC2 start byte
	if (
		((start = socket()->getch()) == 0x4f) &&
		((start = socket()->getch()) == 0x44) &&
		((start = socket()->getch()) == 0x43) &&
		((start = socket()->getch()) == 0x32)
		)
	{
		//get the header length
		if ((theword = socket()->getch()) == -1)
		{
			kdDebug(14151) << k_funcinfo <<
				"Error reading length, byte 1: nothing to be read" << endl;
			odc.headerLength = 0x00;
		}
		else if((theword2 = socket()->getch()) == -1)
		{
			kdDebug(14151) << k_funcinfo <<
				"Error reading data field length, byte 2: nothing to be read" << endl;
			odc.headerLength = 0x00;
		}
		else
		{
			odc.headerLength = (theword << 8) | theword2;
		}

		//convert header to a buffer
		char *buf = new char[odc.headerLength-6];  // the -6 is there because we have already read 6 bytes
		socket()->readBlock(buf,odc.headerLength-6);
		Buffer inbuf;
		inbuf.setBuf(buf,odc.headerLength-6);

		//get channel
		odc.channel = inbuf.getWord();
		//0x0006 is next
		if (inbuf.getWord() != 0x0006)
			kdDebug(14151) << k_funcinfo << "1: expected a 0x0006, didn't get it" << endl;
		//0x0000 is next
		if (inbuf.getWord() != 0x0000)
			kdDebug(14151) << k_funcinfo << "2: expected a 0x0000, didn't get it" << endl;

		//get the 8 byte cookie
		odc.cookie = inbuf.getBlock(8);

		//for (int i=0;i<8;i++)
		//	mCookie[i] = odc.cookie[i];

		// 10 bytes of 0
		if (inbuf.getDWord() != 0x00000000)
			kdDebug(14151) << k_funcinfo << "3: expected a 0x00000000, didn't get it" << endl;
		if (inbuf.getDWord() != 0x00000000)
			kdDebug(14151) << k_funcinfo << "4: expected a 0x00000000, didn't get it" << endl;
		//if (inbuf.getWord() != 0x0000)
		//	kdDebug(14151) << "[OscarDirectConnection] getODC2: 5: expected a 0x0000, didn't get it" << endl;

		// message length
		odc.length = inbuf.getDWord();

		// 6 bytes of 0
		if (inbuf.getDWord() != 0x00000000)
			kdDebug(14151) << k_funcinfo << "6: expected a 0x00000000, didn't get it" << endl;
		if (inbuf.getWord() != 0x0000)
			kdDebug(14151) << k_funcinfo << "7: expected a 0x0000, didn't get it" << endl;

		// ODC2 type
		odc.type = inbuf.getWord();

		// 4 bytes of 0
		if (inbuf.getDWord() != 0x00000000)
			kdDebug(14151) << k_funcinfo << "8: expected a 0x00000000, didn't get it" << endl;

		// screen name (not sure how to get length yet, so we'll just take it and the 0's after it)
		odc.sn = inbuf.getBlock(inbuf.length());

		// 25 bytes of 0
		//might as well just clear buffer, since there won't be anything after those 25 0x00's
		inbuf.clear();
	}
	else
	{
		kdDebug(14151) << k_funcinfo
			<< "Error reading ODC2 header... start byte is '" << start << "'" << endl;
	}

	kdDebug(14151) << k_funcinfo <<
		"Read an ODC2 header!  header length: " << odc.headerLength <<
		", channel: " << odc.channel << ", message length: " << odc.length <<
		", type: " << odc.type << ", screen name: " << odc.sn << endl;

	return odc;
}

void OscarDirectConnection::sendIM(const QString &message, bool /*isAuto*/)
{
	sendODC2Block(message, 0x0000); // 0x0000 means message
}

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

	kdDebug(14151) << k_funcinfo << "Sending ODC2 block, message: " <<
		message << "typingnotify: " << typingnotify << endl;

#ifdef OSCAR_PACKETLOG
		kdDebug(14151) << "=== OUTPUT ===" << outbuf.toString();
#endif

	socket()->writeBlock(outbuf.buffer(), outbuf.length());
}

void OscarDirectConnection::parseMessage(Buffer &inbuf)
{
	kdDebug(14151) << k_funcinfo << "Buffer length is " << inbuf.length() << endl;
	// The message will come first, followed by binary files
	// so let's parse until we see "<BINARY>"
	QString message;
	while (!message.contains("<BINARY>",false))
	{
		//kdDebug(14151) << "[OscarDirect] message is: " << message << endl;
		// while the message does not contain the string "<BINARY>"
		message.append(inbuf.getByte());
		if(inbuf.length()==0)
		{
			//if we are at the end of the buffer
			kdDebug(14151) << k_funcinfo << "Got IM: " << message << endl;

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
		kdDebug(14151) << k_funcinfo << "Datatag matching '" <<
			datatag << "'" << endl;

		if(!inbuf.length())
		{
			//message ended in the middle of the data tag
			kdDebug(14151) << k_funcinfo << "Got IM: " << message << endl;
			//remove the <BINARY> at the end of the string
			emit gotIM(message.remove(message.length()-8, 8),
				connectionName(), false);
		}
	}

	kdDebug(14151) << k_funcinfo << "Got IM: " << message << endl;
	emit gotIM(message.remove(message.length()-8, 8), connectionName(), false);
}

#include "oscardirectconnection.moc"
// vim: set noet ts=4 sts=4 sw=4:
