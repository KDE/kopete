/***************************************************************************
                          imservicesocket.cpp  -  description
                             -------------------
    begin                : Mon Nov 12 2001
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


#include "kmsnservicesocket.h"
#include "kmsnservice.h"

// kde
#include <kglobal.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kmdcodec.h>

//qt
#include <qregexp.h>



KMSNServiceSocket::KMSNServiceSocket(){
}
KMSNServiceSocket::~KMSNServiceSocket(){
}

/* Connect to MSN Service */
void KMSNServiceSocket::connectToService(QString handle, QString password, uint serial, bool silent)
{
	_handle = handle;
	_password = password;
	_serial = serial;
	isConnected = false;
	_silent = silent;
	mailCount = 0;
	
	socket = new KExtendedSocket("messenger.hotmail.com",1863,0x00 | 0x600000 );
	socket->enableRead(true);
	connect(socket, SIGNAL(readyRead()),this, SLOT(slotDataReceived()));
	connect(socket, SIGNAL(connectionFailed(int)), this, SLOT(slotSocketError(int)));
	socket->connect();

/** FIXME - KExtendetSocket doesn't send the connectionSuccess() signal
	calling slotSocketConnected from here */
	slotSocketConnected();
}

void KMSNServiceSocket::close()
{

}

void KMSNServiceSocket::kill()
{
	emit connected(false);
	
	isConnected = false;
	delete socket;
}

void KMSNServiceSocket::closeService()
{
	QString command = "OUT\r\n";
	socket->writeBlock(command,command.length());
	slotSocketClose();
}
void KMSNServiceSocket::slotSocketConnected()
{
	sendProtocol();
}

void KMSNServiceSocket::slotSocketClose()
{
	kdDebug() << "socket closed" << endl;
	emit connected(false);
	isConnected = false;
}

/** FIME for KExtendedSocket */
void KMSNServiceSocket::slotSocketError(int error)
{
	if(!_silent)
	{
		switch(error)
		{
			case 0:
			{
				KMessageBox::error(0,i18n("Connection refused"));
				break;
			}
			case 1:
			{
				KMessageBox::error(0,i18n("Host not found"));
				break;
			}
			default:
			{
				KMessageBox::error(0,i18n("Socket error"));
			}
		}
	}
	emit connected(false);
	isConnected = false;
	kdDebug() << "Socket error: " << error << endl;
}

/* new data has received */
void KMSNServiceSocket::slotDataReceived()
{
	// incoming data
	QString data;
	int ret;
	char buf[1024];
	ret =socket->readBlock(buf,1024);
	if(ret <= 0) return;
	data = buf;
	data = data.left(ret);
	data = QString::fromUtf8(data);
//	showError(data);
	if((data.left(3)) == "911")
	{
		emit connected(false);
		if(!_silent)
		{
			KMessageBox::error(0,i18n("Authentication failed"));
		}
		return;
	}
	if(!isConnected)
	{
		if((data.left(3)) == "XFR")
		{
			// new server reconnect
			newConnect(data);
			return;
		}
	}
	if(data.left(3) == "VER")
	{
		sendServerPolicy();
		return;
	}
	if(data.left(3) == "INF")
	{
		sendInitialInfo();
		return;
	}
	if(data.left(3) == "USR")
	{
		if(kstr.word(data,3) == "S")
		{
			hashRes =data.right(data.length() - data.findRev(" ",-1,false)-1);
			sendResponseInfo();
		}
		else
		{
			sendSerial();
			_publicName = kstr.word( data, 4 ).replace( QRegExp( "%20" ), " " );
			emit newPublicName(_publicName);
			// this is our current user and friendly name
			// do some nice things with it  :-)
		}
		return;
	}
	buffer += data; // fill the buffer with the received data
	while(canReadLine()) // if we have a complete command in the buffer - parseCommand
	{
		parseCommand(readLine());
	}
	
}

/* reads a line from the buffer */
QString KMSNServiceSocket::readLine()
{
	QString command;
	int index = buffer.find("\r\n");
	if(index != -1 )
	{
		command = buffer.left(index );
		buffer = buffer.remove(0,index+2 );
		command.replace(QRegExp("\r\n"),"");
	}
	kdDebug() <<  command << endl;
	return command;
}

/* check if a new line is in the buffer */
bool KMSNServiceSocket::canReadLine()
{
	if(buffer.contains("\r\n") )
		return true;
	return false;
}

/* reads a block of data from the buffer from  ( 0 to len ) */
QString KMSNServiceSocket::readBlock(uint len)
{
	QString block;
	block = buffer.left(len);
	buffer = buffer.remove(0,len);
	return block;
}

void KMSNServiceSocket::parseCommand(QString str)
{
	kdDebug() << "MSN Plugin: LibKmerlin: Parsing command " << str << endl;
	QString miss,len;
	str = str.replace(QRegExp("\r\n"),"");
	QString command = str.left(3);
	if(command == "NLN")
	{
		// handle, publicName, status
		emit contactStatusChanged( kstr.word(str,2), kstr.word(str,3).replace(QRegExp("%20")," "), kstr.word(str,1) );
		return;
	}
	if(command == "LST")
	{
		if( kstr.word(str,2) == "FL")
		{
			// handle, publicName, group, list
			emit contactList( kstr.word(str,6), kstr.word(str,7).replace(QRegExp("%20")," "), kstr.word(str,8), kstr.word(str,2) );
		}
		else
		{
			// handle, publicName, 0L,  list
			emit contactList( kstr.word(str,6), kstr.word(str,7).replace(QRegExp("%20")," "), 0L, kstr.word(str,2) );
		}
		return;
	}
	if(command == "MSG")
	{
		len = kstr.word(str,3);
		miss = readBlock(len.toUInt());
		miss = QString::fromUtf8(miss);
		//miss = miss.left(len.toUInt());
		if(miss.contains("Inbox-Unread:"))
		{
			 //this sends the server if we are going online, contains the unread message count
			 miss = miss.right(miss.length() - miss.find("Inbox-Unread:") );
			 miss = miss.left(miss.find("\r\n"));
			 mailCount = miss.right(miss.length() -miss.find(" ")-1).toUInt();
			 emit newMail("",mailCount);
			 return;
		}
		if(miss.contains("Message-Delta:"))
		{
			 //this sends the server if mails are deleted
			 miss = miss.right(miss.length() - miss.find("Message-Delta:") );
			 miss = miss.left(miss.find("\r\n"));
			 mailCount = mailCount - miss.right(miss.length() -miss.find(" ")-1).toUInt();
			 emit newMail("",mailCount);
			 return;
		}
		if(miss.contains("From-Addr:"))
		{
			 //this sends the server if a new mail has arrived
			 miss = miss.right(miss.length() - miss.find("From-Addr:") );
			 miss = miss.left(miss.find("\r\n"));
			 mailCount++;
			 miss = miss.right(miss.length() -miss.find(" ")-1);
			 emit newMail(miss,mailCount);
			 return;
		}
		return;
	}
	if(command == "FLN")
	{
		emit contactStatusChanged(kstr.word(str,1), "", "FLN" );
		return;
	}
	if(command == "ILN")
	{
		// handle, publicName, Status
		emit contactStatus(kstr.word(str,3), kstr.word(str,4).replace(QRegExp("%20")," "), kstr.word(str,2) );
		return;
	}
	if(command == "GTC")
	{
		kdDebug() << "GTC: is not implemented!" << endl;
		return;
	}
	if(command == "BLP")
	{
		kdDebug() << "BLP: is not implemented!" << endl;
		return;
	}
	if(command == "RNG")
	{
		// SessionID, Address, AuthInfo, handle, publicName
		emit invitedToChat( kstr.word(str,1), kstr.word(str,2), kstr.word(str,4), kstr.word(str,5), kstr.word(str,6).replace(QRegExp("%20")," ") );
		return;
	}

	if(command == "XFR")
	{
		// Address, AuthInfo
		emit startChat(kstr.word(str,3), kstr.word(str,5) );
		return;
	}
	if(command == "ADD")
	{
		//
		if( kstr.word(str,2) == "FL")
		{
			// handle, publicName, List, serial , group
			emit contactAdded( kstr.word(str,4), kstr.word(str,4).replace(QRegExp("%20")," "), kstr.word(str,2), kstr.word(str,3).toUInt(), kstr.word(str,6).toUInt() );
		}
		else
		{
			// handle, publicName, List, serial
			emit contactAdded( kstr.word(str,4), kstr.word(str,4).replace(QRegExp("%20")," "), kstr.word(str,2), kstr.word(str,3).toUInt(), 0L );
		}
		return;
	}
	if(command == "REM") // someone is removed from a list
	{
		if( kstr.word(str, 2) == "FL")
		{
			// handle, list, serial, group
			contactRemoved( kstr.word(str,4), kstr.word(str,2), kstr.word(str,3).toUInt(), kstr.word(str,5).toUInt() );
		}
		else
		{
			contactRemoved( kstr.word(str,4), kstr.word(str,2), kstr.word(str,3).toUInt(), 0L );
		}
		return;
	}
	if(command == "OUT")
	{
		if( kstr.word(str,1) == "OTH" )
		{
			KMessageBox::error(0,i18n("You have connected\r\nfrom an other client"));
		}
		slotSocketClose();
		emit sessionClosed( kstr.word(str,1) );
		return;
	}
	if(command == "CHG")
	{
		if(!isConnected)
		{
			isConnected = true;
			emit connected(true);
		}
		statusChanged( kstr.word(str,2) );
		return;
	}
	if(command == "REA")
	{
		emit publicNameChanged(kstr.word(str,3),kstr.word(str,4).replace(QRegExp("%20")," ") );
		kdDebug() << str << endl;
		return;
	}
	if( command == "LSG" )
	{
		if( str.contains("1 1 0 ~ 0") )
		{
			emit groupName( tr("Friends"), 0 );
			renameGroup(tr("Friends"),0);
			return;
		}
		// groupName, group
		emit groupName( kstr.word(str,6).replace(QRegExp("%20")," ") , kstr.word(str,5).toUInt() );
		return;
	}
	if( command == "ADG" )
	{
		// groupName, serial, group
		emit groupAdded( kstr.word(str,3).replace(QRegExp("%20")," "), kstr.word(str,2).toUInt(), kstr.word(str,4).toUInt() );
		return;
	}
	if( command == "REG" )
	{
		// groupName, serial, group
		emit groupRenamed( kstr.word(str,4).replace(QRegExp("%20")," "), kstr.word(str,2).toUInt(), kstr.word(str,3).toUInt() );
		return;
	}
	if( command == "RMG" )
	{
		emit groupRemoved(kstr.word(str,2).toUInt(), kstr.word(str,3).toUInt() );
	}
	if( command  == "CHL")
	{
		sendFinalAuthentication(kstr.word(str,2));
		return;
	}
	if(command == "SYN")
	{
		// this is the current serial on the server, if its different with the own we can get the user list
		//isConnected = true;
		uint serial = kstr.word(str,2).toUInt();
		if( serial != _serial)
		{
			emit newSerial(serial);  // remove all contacts, msn sends a new contact list
			_serial = serial;
		}
		//emit connected(true);
		// set the status to online
		// create an option in config dialog to select the state to be set
		setStatus(NLN);
		sendCVR();
		return;
	}


}

void KMSNServiceSocket::sendCVR()
{

}

void KMSNServiceSocket::sendFinalAuthentication(QString res)
{
	ID = time((time_t *)NULL);
	res.replace(QRegExp("\r\n"),"");
	QString command;
	QString str3 = res + "Q1P7W2E4J9R8U3S5" ;
	KMD5 context(str3);
	QString str5 = context.hexDigest();
	command.sprintf("QRY %lu msmsgs@msnmsgr.com 32\r\n",ID);
	command += str5 ;
	socket->writeBlock(command,command.length());
	kdDebug() << "Sending final Authentication" << endl;
}

//
void KMSNServiceSocket::sendProtocol()
{
	ID = time((time_t *)NULL);
	QString command;
	command.sprintf("VER %lu MSNP7 MSNP6 MSNP5 MSNP4 CVR0\r\n",ID);
	socket->writeBlock(command,command.length());
	kdDebug() << "Sending protocol" << endl;
}

/* MSN Service has send a new IP , so connect to it */
void KMSNServiceSocket::newConnect( QString data)
{
	QString port,server;
	data =  kstr.word(data,3);
	port = data.right(data.length() - data.findRev(":") - 1);
	server = data.right(data.length() - data.findRev(" ") - 1);
	server = server.left(server.find(":"));
	socket->flush();
	socket->closeNow();
	delete socket;
	socket = new KExtendedSocket(server,port.toUInt(),0x00 | 0x600000 );
	socket->enableRead(true);
	connect(socket, SIGNAL(readyRead()),this, SLOT(slotDataReceived()));
	connect(socket, SIGNAL(connectionFailed(int)), this, SLOT(slotSocketError(int)));
	kdDebug() << "Connect to new Server... " << server << ":" << port << endl;
	socket->connect();
	slotSocketConnected(); // FIXME
}

void KMSNServiceSocket::sendServerPolicy()
{
	ID = time((time_t *)NULL);
	QString command;
	command.sprintf("INF %lu\r\n",ID);
	socket->writeBlock(command,command.length());
	kdDebug() << "Sending server policy" << endl;
}

void KMSNServiceSocket::sendInitialInfo()
{
	md5_tr = time((time_t *)NULL);
	QString command;
	command.sprintf("USR %lu MD5 I ",md5_tr);
	command += _handle.utf8() +"\r\n";
	socket->writeBlock(command,command.length());
	kdDebug() << "Sending initial Authentication" << endl;
}

void KMSNServiceSocket::sendResponseInfo()
{
	if(hashRes.contains("\r\n",true))
	{
	  hashRes = hashRes.left(hashRes.find("\r\n",0,true));
	}
	QString str3 = hashRes +_password;
	KMD5 context(str3);
	QString strcommand1,str5 = context.hexDigest();
	strcommand1.sprintf("USR %lu ",md5_tr);
	strcommand1 += "MD5 S ";
	strcommand1 += str5 + "\r\n";
	socket->writeBlock(strcommand1,strcommand1.length());
	kdDebug() << "Sending response Authentication" << endl;
}

void KMSNServiceSocket::sendSerial()
{
	ID = time((time_t *)NULL);
	QString command;
	command.sprintf("SYN %lu %u\r\n",ID,_serial);
	socket->writeBlock(command,command.length());
	kdDebug() << "Sending serial number" << endl;
}
// end



void KMSNServiceSocket::addGroup(QString groupName)
{
	groupName.replace(QRegExp(" "),"%20");        // convert whitespaces to %20
	ID = time((time_t *)NULL);
	QString command;
	command.sprintf("ADG %lu ",ID);
	command += groupName.utf8() + " 0\r\n";
	socket->writeBlock(command,command.length());
}

void KMSNServiceSocket::renameGroup(QString groupName, uint group)
{
	groupName.replace(QRegExp(" "),"%20");        // convert whitespaces to %20
	ID = time((time_t *)NULL);
	QString command;
	command.sprintf("REG %lu %u ",ID, group);
	command += groupName.utf8() + " 0\r\n";
	socket->writeBlock(command,command.length());
}

void KMSNServiceSocket::removeGroup(uint group)
{
	QString command;
	ID = time((time_t *)NULL);
	command.sprintf("RMG %lu %u\r\n",ID, group);
	socket->writeBlock(command,command.length());
}

void KMSNServiceSocket::addContact( const QString &handle, QString publicName, uint group, int list )
{
	time_t ID = time((time_t *)NULL);
	QString command;
	QString strGroup;
	strGroup.setNum(group);
	if( list == FL )
	{
		command.sprintf("ADD %lu FL ",ID);
		command += handle.utf8() + " " + handle.utf8() + " " +strGroup.utf8() + "\r\n";
	}
	if( list == AL )
	{
		command.sprintf("ADD %lu AL ",ID);
		command += handle.utf8() + " " + publicName.replace(QRegExp(" "),"%20").utf8() + "\r\n";
	}
	if( list == BL )
	{
		command.sprintf("ADD %lu BL ",ID);
		command += handle.utf8() + " " + publicName.replace(QRegExp(" "),"%20").utf8() + "\r\n";
	}
	socket->writeBlock(command,command.length());
}

void KMSNServiceSocket::removeContact( const QString &handle, uint group, int list )
{
	ID = time((time_t *)NULL);
	QString command;
	QString strGroup;
	strGroup.setNum(group);
	switch(list)
	{
		case FL:
		{
			command.sprintf("REM %lu FL ",ID);
			command += handle.utf8() + " " +strGroup.utf8() + "\r\n";
			break;
		}
		case AL:
		{
			command.sprintf("REM %lu AL ",ID);
			command += handle.utf8() + "\r\n";
			break;
		}
		case BL:
		{
			command.sprintf("REM %lu BL ",ID);
			command += handle.utf8() + "\r\n";
			break;
		}
	}
	kdDebug() << "Send command: " << command << endl;
	socket->writeBlock(command,command.length());
}

void KMSNServiceSocket::setStatus( int status )
{
	ID = time((time_t *)NULL);
	QString command;
	switch(status)
	{
		case NLN:
		{
			command.sprintf("CHG %lu NLN\r\n",ID);
			break;
		}
		case BSY:
		{
			command.sprintf("CHG %lu BSY\r\n",ID);
			break;
		}
		case BRB:
		{
			command.sprintf("CHG %lu BRB\r\n",ID);
			break;
		}
		case AWY:
		{
			command.sprintf("CHG %lu AWY\r\n",ID);
			break;
		}
		case PHN:
		{	
			command.sprintf("CHG %lu PHN\r\n", ID);
			break;
		}
		case LUN:
		{
			command.sprintf("CHG %lu LUN\r\n",ID);
			break;
		}
		case FLN:
		{
			command.sprintf("CHG %lu FLN\r\n",ID);
			break;
		}
		case HDN:
		{
			command.sprintf("CHG %lu HDN\r\n",ID);
			break;
		}
		case IDL:
		{
			command.sprintf("CHG %lu IDL\r\n",ID);
			break;
		}
	}
	socket->writeBlock(command,command.length());
}

void KMSNServiceSocket::changePublicName(QString publicName )
{
	QString command;
	ID = time(( time_t*)NULL);
	command.sprintf("REA %lu ",ID);
	command += _handle.utf8() + " " + publicName.replace(QRegExp(" "),"%20").utf8() +"\r\n";
	socket->writeBlock(command, command.length());
}

void KMSNServiceSocket::createChatSession()
{
	ID = time((time_t *)NULL);
	QString command;
	command.sprintf("XFR %lu SB\r\n",ID);
	socket->writeBlock(command,command.length());
}

#include "kmsnservicesocket.moc"


