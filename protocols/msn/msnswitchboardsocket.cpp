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
#include "msnprotocol.h"
#include "msncontact.h"
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
	msgSocket = 0L;
}

KMSNChatService::~KMSNChatService()
{
	delete msgSocket;
	msgSocket = 0L;
}

void KMSNChatService::connectToSwitchBoard(QString ID, QString address, QString auth)
{
	m_id = 1;

	QString command, args;
	QString server = address.left( address.find( ":" ) );
	uint port = address.right( address.length() - address.findRev( ":" ) - 1 ).toUInt();
	msgSocket = new KExtendedSocket( server, port, 0x600000 );
	msgSocket->enableRead(true);
	connect(msgSocket, SIGNAL(readyRead()),this, SLOT(slotDataReceived()));
//	connect(msgSocket, SIGNAL(connectionSuccess()),this, SLOT(slotReady()));
//	connect(msgSocket, SIGNAL(connectionFailed(int)), this, SLOT(slotSocketError(int)));
	msgSocket->connect();

	if( !ID )
	{
		command = "USR";
		args = myHandle + " " + auth;
	}
	else
	{
		command = "ANS";
		args = myHandle + " " + auth + " " + ID;
	}
	sendCommand( command, args );
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
	int avail = msgSocket->bytesAvailable();
	int toRead = avail;
	if( avail == 0 )
	{
		kdDebug() << "KMSNChatService::slotDataReceived:\n"
			"** WARNING ** bytesAvailable() returned 0!\n"
			"If you are running KDE 3.0, please upgrade to current CVS or to\n"
			"KDE 3.0.1 to fix a bug in KExtendedSocket always returning 0.\n"
			"Trying to read 4kb blocks instead, but be prepared for problems!"
			<< endl;
		toRead = 4096;
	}

	// incoming data
	char *buf = new char[ toRead + 1 ];
	int ret = msgSocket->readBlock( buf, toRead );
	if( ret < 0 )
	{
		kdDebug() << "KMSNChatService:slotDataReceived: WARNING: "
			"readBlock() returned " << ret << "!" <<endl;
	}
	else if( ret == 0 )
	{
		kdDebug() << "KMSNChatService:slotDataReceived: WARNING: "
			"readBlock() returned no data!" <<endl;
	}
	else
	{
		if( avail )
		{
			if( ret != avail)
			{
				kdDebug() << "KMSNChatService:slotDataReceived: WARNING: "
					<< avail << " bytes were reported available, "
					<< "but readBlock() returned only " << ret
					<< " bytes! Proceeding anyway." << endl;
			}
		}
		else
		{
			kdDebug() << "KMSNChatService:slotDataReceived: Info: "
				<< "Read " << ret << " bytes into 4kb block." << endl;
		}

		buf[ ret ] = '\0'; // Make it properly null-terminated
		QString data = QString::fromUtf8( buf );
		kdDebug() << QTime::currentTime().toString() <<
			" - KMSNChatService::slotDataReceived: Received '" <<
			data << "'" << endl;
	//	showError(data);

		buffer += data; // fill the buffer with the received data

		// FIXME: The below code is, uhm... CRAP!
redo:
		QString str, len;
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
				QString handle = kstr.word( str, 1 );
				emit updateChatMember( handle, "JOI", false );

				if( !m_chatMembers.contains( handle ) )
					m_chatMembers.append( handle );
			}
			if(str.left(3) == "IRO")
			{
				// we have joined a multi chat session- this are the users in this chat
				emit switchBoardIsActive(true);
				QString handle = kstr.word( str, 4 );
				if( !m_chatMembers.contains( handle ) )
					m_chatMembers.append( handle );

				if( kstr.word( str, 2 ) == kstr.word( str, 3 ) )
					emit updateChatMember( handle, "IRO", true );
				else
					emit updateChatMember( handle, "IRO", false );
			}
			if(str.left(3) == "USR")
			{
				callUser();
			}
			if(str.left(3) == "BYE")
			{
				// some has disconnect from chat, update user in chat list
				QString handle = kstr.word( str, 1 ).replace(
					QRegExp( "\r\n" ), "" );

				emit updateChatMember( handle, "BYE", false );
				if( m_chatMembers.contains( handle ) )
					m_chatMembers.remove( handle );
			}
			if(str.left(3) == "MSG")
			{
				QString miss;
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

					// FIXME: THIS IS UGLY!!!!!!!!!!!!!!!!!!!!!!
					KopeteContactList others;
					others.append( MSNProtocol::protocol()->myself() );
					KopeteMessage msg(
						MSNProtocol::protocol()->contacts()[ handle ] , others, miss,
						KopeteMessage::Inbound );
					msg.setFg( fg );
					msg.setFont( font );
					emit msgReceived( msg );
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
						kdDebug() << "cookie: " << cocki << endl;
						QString message =
							"MIME-Version: 1.0\r\n"
							"Content-Type: text/x-msmsgsinvite; charset=UTF-8\r\n"
							"\r\n"
							"Invitation-Command: ACCEPT\r\n";
							"Invitation-Cookie: " + cocki + "\r\n"
							"Launch-Application: FALSE\r\n"
							"Request-Data: IP-Address: 192.168.0.2\r\n"
							"Port: 6891";
							//Application-Name: FileTransfer\r\nApplication-GUID: {5D3E02AB-6190-11d3-BBBB-00C04F795683}\r\nInvitation-Command: INVITE\r\nInvitation-Cookie: 58273\r\nApplication-File: lvback.gif\r\nApplication-FileSize: 4256\r\n\r\n";
						QString command = "MSG";
						QString args = QString( "N %1\r\n" ).arg( message.length() );
						command += message;
						sendCommand( command, args + message, false );
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

	// Cleanup
	delete[] buf;
}

// this sends the user is typing msg
void KMSNChatService::slotTypingMsg()
{
	QString message =
		"MIME-Version: 1.0\r\n"
		"Content-Type: text/x-msmsgscontrol\r\n"
		"TypingUser: " + myHandle + "\r\n"
		"\r\n";
	QString args = QString( "U %1 \r\n" ).arg( message.length() );
	sendCommand( "MSG", args + message, false );
}

// this Invites an Contact
void KMSNChatService::slotInviteContact(QString handle)
{
	sendCommand( "CAL", handle );
}

// this sends a short message to the server
void KMSNChatService::slotSendMsg( const KopeteMessage &msg )
{
	QString head =
		"MIME-Version: 1.0\r\n"
		"Content-Type: text/plain; charset=UTF-8\r\n"
		"X-MMS-IM-Format: FN=MS%20Sans%20Serif; EF=; ";

	// Color support
	if (msg.fg().isValid()) {
		QString colorCode = msg.fg().name().remove(0,1);
		head += "CO=" + colorCode;
	} else {
		head += "CO=0";
	}

	head += "; CS=0; PF=0\r\n"
			"\r\n";

	head += msg.body();
	QString args = QString( "A %1\r\n" ).arg( head.length() );
	sendCommand( "MSG", args + head, false );

	// TODO: send our fonts and colors as well
	KopeteContactList others;
	others.append( MSNProtocol::protocol()->contacts()[ myHandle ] );
	emit msgReceived( msg );    // send the own msg to chat window
}

void KMSNChatService::sendCommand( const QString &cmd, const QString &args,
	bool addNewLine )
{
	QString data = cmd + " " + QString::number( m_id );
	if( !args.isEmpty() )
		data += " " + args;
	if( addNewLine )
		data += "\r\n";

	kdDebug() << "KMSNChatService::sendCommand: Sending command " << data
		<< endl;

	msgSocket->writeBlock( data, data.length() );

	m_id++;
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
	sendCommand( "CAL", msgHandle );
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

// vim: set noet ts=4 sts=4 sw=4:

