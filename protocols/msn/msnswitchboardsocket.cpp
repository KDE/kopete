/***************************************************************************
                          imchatservice.cpp  -  description
                             -------------------
    begin                : Tue Nov 27 2001
    copyright            : (C) 2001 by Olaf Lueg
    email                : olueg@olsd.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "kmsnchatservice.h"
#include "kmsnservice.h"
#include "msnprotocol.h"
#include <time.h>
// qt
#include <qdatetime.h>
#include <qsocket.h>
#include <kextsock.h>
#include <qfont.h>
#include <qcolor.h>
// kde
#include <klocale.h>
#include <kglobal.h>
#include <kdebug.h>

KMSNChatService::KMSNChatService()
{
	socketTimer = 0L;
}
KMSNChatService::~KMSNChatService(){
	
}

void KMSNChatService::connectToSwitchBoard(QString ID, QString address, QString auth){
	QString server,port,command;
	time_t Tr_ID = time((time_t *)NULL);
	if(ID == 0L)
	{
		command.sprintf("USR %lu ",Tr_ID);
		command += myHandle +" "+ auth+"\r\n";
		server = address;
		server = server.left(server.find(":"));
		port = address;
		port = port.right(port.length() -port.findRev(":")-1);
	}
	else
	{
		command.sprintf("ANS %lu ",Tr_ID);
		command += myHandle +" "+ auth + " "+ ID +"\r\n";
		server = address;
		server = server.left(server.find(":"));
		port = address;
		port = port.right(port.length() -port.findRev(":")-1);
	}
	msgSocket = new KExtendedSocket(server,port.toUInt(),0x00 | 0x600000 );
	msgSocket->enableRead(true);
	connect(msgSocket, SIGNAL(readyRead()),this, SLOT(slotDataReceived()));
//	connect(msgSocket, SIGNAL(connectionSuccess()),this, SLOT(slotReady()));
//	connect(msgSocket, SIGNAL(connectionFailed(int)), this, SLOT(slotSocketError(int)));
	msgSocket->connect();
	msgSocket->writeBlock(command,command.length());
	// send active message
	emit switchBoardIsActive(true);
	/** FIXME : we have no socketClosed signal */
	if( socketTimer != 0L)
		killTimer(socketTimer);
	socketTimer = startTimer(1000);
}

void KMSNChatService::timerEvent(QTimerEvent *ev)
{
	if(ev->timerId() == socketTimer )
	{
		if(msgSocket->socketStatus() == 21 )
		{
			emit switchBoardIsActive(true);
		}
		else
			emit switchBoardIsActive(false);
	}
}

/* reads a line from the buffer */
QString KMSNChatService::readLine()
{
	QString command;
	int index = buffer.find("\r\n");
	if(index != -1 )
	{
		command = buffer.left(index );
		buffer = buffer.remove(0,index+2 );
		command.replace(QRegExp("\r\n"),"");
	}
	return command;
}

/* check if a new line is in the buffer */
bool KMSNChatService::canReadLine()
{
	if(buffer.contains("\r\n") )
		return true;
	return false;
}

/* reads a block of data from the buffer from  ( 0 to len ) */
QString KMSNChatService::readBlock(uint len)
{
	QString block;
	block = buffer.left(len);
	buffer = buffer.remove(0,len);
	return block;
}
void KMSNChatService::slotDataReceived()
{
	QString str,len,miss, data;
	char dat[ 1025 ];
	int ret;
	ret = msgSocket->readBlock(dat,1024);
	if( ret <= 0 )
		return;
	dat[ ret ] = '\0'; // Make it properly null-terminated
	data = QString::fromUtf8( dat );
	buffer += data;
redo:
	if(canReadLine())
	{
		str = readLine();
		//str = QString::fromUtf8(dat);
		if(str.left(3) == "NAK")
		{
			emit msgAcknowledgement(false);    // msg has not reveived
		}
		if(str.left(3) == "ACK")
		{
			emit msgAcknowledgement(true);   // msg has received
		}
		if(str.left(3) == "JOI")
		{
			// new user joins the chat, update user in chat list
			emit switchBoardIsActive(true);
			emit updateChatMember(kstr.word(str,1),"JOI",false);
		}
		if(str.left(3) == "IRO")
		{
			// we have joined a multi chat session- this are the users in this chat
			emit switchBoardIsActive(true);
			if(kstr.word(str,2) == kstr.word(str,3) )
			{
				emit updateChatMember(kstr.word(str,4),"IRO",true);
			}
			else
			{
				emit updateChatMember(kstr.word(str,4),"IRO",false);
			}
		}
		if(str.left(3) == "USR")
		{
			callUser();
		}
		if(str.left(3) == "BYE")
		{
			// some has disconnect from chat, update user in chat list
			emit updateChatMember(kstr.word(str,1).replace(QRegExp("\r\n"),""),"BYE",false);
		}
		if(str.left(3) == "MSG")
		{
			kdDebug() << "MSG Plugin: ChatBoard: Received data MSG" << endl;

			len = kstr.word(str,3);
			miss = readBlock(len.toUInt());
			miss = miss.left(len.toUInt());

			if(miss.contains("Content-Type: text/plain;"))
			{
				QString fontinfo = miss.left(miss.find("\r\n\r\n"));
				QString tmp = parseFontAttr(fontinfo, "CO");
				QFont font = QFont();
				QColor fg = QColor();

				if (!tmp.isEmpty())
				{
					if (tmp.length() == 2) // only #RR (red color) given
						fg.setRgb(tmp.mid(0,2).toInt(0,16), 0, 0);
					else if (tmp.length() == 4) // #GGRR (green, red) given.
						fg.setRgb(tmp.mid(2,2).toInt(0,16), tmp.mid(0,2).toInt(0,16), 0);
					else if (tmp.length() == 6) // full #BBGGRR given
						fg.setRgb(tmp.mid(4,2).toInt(0, 16), tmp.mid(2,2).toInt(0,16), tmp.mid(0,2).toInt(0,16));
				}

				// if FN= is supplied, then i assume the rest is too...
				if (fontinfo.contains("FN"))
				{
					font = QFont(
								parseFontAttr(fontinfo, "FN").replace(QRegExp("%20"), " "), // font family
								parseFontAttr(fontinfo, "PF").toInt(), // font size
								parseFontAttr(fontinfo, "EF").contains('B') ? QFont::Bold : QFont::Normal, // bold?
								parseFontAttr(fontinfo, "EF").contains('I') ? true : false ); // italic?
				}

				kdDebug() << "MSN Plugin: PLAIN TExT; -" << miss << "-" << endl;
				miss = miss.right(miss.length() -miss.findRev("\r\n\r\n") - 4); // we wanna get rid of the \r\n\r\n as well...
				kdDebug() << "MSG Plugin: ChatBoard: miss seria " << miss << endl;
				QString handle = kstr.word(str,1);
				kdDebug() << "MSG Plugin: ChatBoard: handle seria " << handle << endl;

				emit msgReceived(handle,MSNProtocol::protocol()->publicName(handle), miss, font, fg);//.replace(QRegExp("\r\n"),""));
				//emit msgReceived(handle,"Nick", miss);//.replace(QRegExp("\r\n"),""));
			}
			// incoming message for File-transfer
			if(miss.contains("Content-Type: text/x-msmsgsinvite; charset=UTF-8"))
			{
				// filetransfer ,this comes in a later release
				// needs some debugging time
				kdDebug() << "filetransfer : " << miss << endl;
				if(miss.contains("Invitation-Command: ACCEPT"))
				{
			 		//fileSocket = new QSocket(this);
					//connect(fileSocket,SIGNAL(readyRead()),this,SLOT(slotFileData()));
					//fileSocket->connectToHost("192.168.0.1",6891);
					//kdDebug() << "socket erstellt" << endl;

				}
				else
				{
					QString cocki = miss.right(miss.length() - miss.find("Invitation-Cookie:")-19);
					cocki = cocki.left(cocki.find("\r\n"));
					debug("cocki"+cocki);
					time_t Tr_ID = time((time_t *)NULL);
					QString command, message = "MIME-Version: 1.0\r\nContent-Type: text/x-msmsgsinvite; charset=UTF-8\r\n\r\nInvitation-Command: ACCEPT\r\n";
					message += "Invitation-Cookie: "+cocki+"\r\nLaunch-Application: FALSE\r\nRequest-Data: IP-Address: 192.168.0.2\r\nPort: 6891"; //Application-Name: FileTransfer\r\nApplication-GUID: {5D3E02AB-6190-11d3-BBBB-00C04F795683}\r\nInvitation-Command: INVITE\r\nInvitation-Cookie: 58273\r\nApplication-File: lvback.gif\r\nApplication-FileSize: 4256\r\n\r\n";
					command.sprintf("MSG %lu N %d\r\n",Tr_ID,message.length());
					command += message;
					msgSocket->writeBlock(command,command.length());
					debug("gesendet : "+command);
				}
			}
			if(miss.contains("MIME-Version: 1.0\r\nContent-Type: text/x-msmsgscontrol\r\nTypingUser:"))
			{
				QString message;
				message = miss.right(miss.length() - miss.findRev(" ")-1);
				message = message.replace(QRegExp("\r\n"),"");
				emit userTypingMsg(message);    // changed 20.10.2001
			}
		}
	}
	if(canReadLine())goto redo;
}

// this sends the user is typing msg
void KMSNChatService::slotTypingMsg()
{
	time_t Tr_ID = time((time_t *)NULL);
	QString command, message = "MIME-Version: 1.0\r\nContent-Type: text/x-msmsgscontrol\r\nTypingUser: "+myHandle+"\r\n\r\n";
	command.sprintf("MSG %lu U %d \r\n",Tr_ID,message.length());
	command += message;
	msgSocket->writeBlock(command,command.length());
}

// this Invites an Contact
void KMSNChatService::slotInviteContact(QString handle)
{
	time_t Tr_ID = time((time_t *)NULL);
	QString command;
	command.sprintf("CAL %lu ",Tr_ID);
	command += handle+"\r\n";
	msgSocket->writeBlock(command,command.length());
}

// this sends a short message to the server
void KMSNChatService::slotSendMsg(QString message)
{
	time_t Tr_ID = time((time_t *)NULL);
	QString command,head;
	head ="MIME-Version: 1.0\r\nContent-Type: text/plain; charset=UTF-8\r\nX-MMS-IM-Format: FN=MS%20Sans%20Serif; EF=; CO=0; CS=0; PF=0\r\n\r\n";
	head += message.utf8();
	command.sprintf("MSG %lu A %d\r\n",Tr_ID,head.length());
	command += head;
	msgSocket->writeBlock(command,command.length());
#warning TODO: send our colors as well
	emit msgReceived(myHandle, MSNProtocol::protocol()->publicName(), message, QFont(), QColor() );    // send the own msg to chat window
}

void KMSNChatService::slotSocketClosed()
{
	// we have lost the connection, send a message to chatwindow (this will not displayed)
	emit switchBoardIsActive(false);
	delete this;
}

void KMSNChatService::slotCloseSession()
{
	QString command = "OUT\r\n";
	msgSocket->writeBlock(command,command.length());
}

void KMSNChatService::callUser()
{
	time_t Tr_ID = time((time_t *)NULL);
	QString command;
	command.sprintf("CAL %lu ",Tr_ID);
	command += msgHandle+"\r\n";
	msgSocket->writeBlock(command,command.length());
}

QString KMSNChatService::parseFontAttr(QString str, QString attr)
{
	QString tmp;
	int pos1=0, pos2=0;

	pos1 = str.find(attr + "=");

	if (pos1 == -1)
		return "";

	pos2 = str.find(";", pos1+3);

	if (pos2 == -1)
		tmp = str.mid(pos1+3, str.length() - pos1 - 3);
	else
		tmp = str.mid(pos1+3, pos2 - pos1 - 3);

	return tmp;
}

#include "kmsnchatservice.moc"
