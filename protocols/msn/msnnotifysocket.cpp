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

#include <qdatetime.h>

#include "kmsnservicesocket.h"
#include "msnprotocol.h"

// kde
#include <kglobal.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kmdcodec.h>

//qt
#include <qregexp.h>

KMSNServiceSocket::KMSNServiceSocket()
{
}

KMSNServiceSocket::~KMSNServiceSocket()
{
}

/* Connect to MSN Service */
void KMSNServiceSocket::connectToMSN( const QString &handle,
	const QString &password, uint serial, bool silent )
{
	_handle = handle;
	_password = password;
	_serial = serial;
	_silent = silent;
	mailCount = 0;

	connectInternal( "messenger.hotmail.com", 1863 );
}

void KMSNServiceSocket::connectInternal( const QString &server, uint port )
{
	isConnected = false;
	m_id = 0;

	socket = new KExtendedSocket( server, port, 0x600000 );
	socket->enableRead( true );

	connect( socket, SIGNAL( readyRead() ), this, SLOT( slotDataReceived() ) );
	connect( socket, SIGNAL( connectionFailed( int ) ),
		this, SLOT( slotSocketError( int ) ) );

	socket->connect();

	// We're connected. Send the protocol. Reply from the server should be
	// a 'VER' reply, handled in slotDataReceived
	kdDebug() << "Sending protocol" << endl;
	sendCommand( "VER", "MSNP7 MSNP6 MSNP5 MSNP4 CVR0" );
}

void KMSNServiceSocket::close()
{
}

void KMSNServiceSocket::kill()
{
	emit connected(false);

	isConnected = false;
	delete socket;
	socket = 0L;
}

void KMSNServiceSocket::closeService()
{
	QString command = "OUT\r\n";
	socket->writeBlock(command,command.length());
	slotSocketClose();
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
	char buf[1025];
	int ret = socket->readBlock(buf,1024);
	if( ret <= 0 )
		return;
	buf[ ret ] = '\0'; // Make it properly null-terminated
	QString data = QString::fromUtf8( buf );
	kdDebug() << QTime::currentTime().toString() <<
		" - KMSNServiceSocket::slotDataReceived: Received '" <<
		data << "'" << endl;
//	showError(data);

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
	QString cmd  = str.section( ' ', 0, 0 );
	QString data = str.section( ' ', 2 ).replace( QRegExp( "\r\n" ), "" );

	// For some reason the RNG command doesn't send a valid id as second
	// argument, but a session id instead. As long as that's the only command
	// suffering from this the below will work, otherwise we might be in
	// need for a better solution...
	if( cmd != "RNG" )
		m_id = str.section( ' ', 1, 1 ).toUInt();

	kdDebug() << "KMSNServiceSocket::parseCommand: Parsing command " << cmd <<
		" (ID " << m_id << "): '" << data << "'" << endl;

	str.replace( QRegExp( "\r\n" ), "" );
	if( cmd == "911" )
	{
		emit connected(false);
		if(!_silent)
			KMessageBox::error(0,i18n("Authentication failed"));
	}
	else if( cmd == "XFR" )
	{
		// XFR is a bit special. When connected it's used to start a chat,
		// but when connecting it's a redirect to another server, essentially
		// aborting the connect
		if( isConnected )
		{
			// Address, AuthInfo
			emit startChat( data.section( ' ', 1, 1 ),
				data.section( ' ', 2, 2 ) );
		}
		else
		{
			// new server reconnect
			QString host = data.section( ' ', 1, 1 );
			QString server = host.section( ':', 0, 0 );
			uint port = host.section( ':', 1, 1 ).toUInt();
			newConnect( server, port );
		}
	}
	else if( cmd == "VER" )
	{
		kdDebug() << "Sending server policy" << endl;
		sendCommand( "INF" );
	}
	else if( cmd == "INF" )
	{
		kdDebug() << "Sending initial Authentication" << endl;
		sendCommand( "USR", "MD5 I " + _handle );
	}
	else if( cmd == "USR" )
	{
		if( data.section( ' ', 1, 1 ) == "S" )
		{
			kdDebug() << "Sending response Authentication" << endl;

			QString hashRes = str.right( str.length() -
				str.findRev( " ", -1, false ) - 1 );
			if(hashRes.contains("\r\n",true))
				hashRes = hashRes.left(hashRes.find("\r\n",0,true));

			KMD5 context( hashRes + _password );
			sendCommand( "USR", "MD5 S " + context.hexDigest() );
		}
		else
		{
			kdDebug() << "Sending serial number" << endl;
			sendCommand( "SYN", QString::number( _serial ) );

			// this is our current user and friendly name
			// do some nice things with it  :-)
			_publicName = data.section( ' ', 2, 2 ).replace(
				QRegExp( "%20" ), " " );
			emit newPublicName(_publicName);
		}
	}
	else if( cmd == "NLN" )
	{
		// handle, publicName, status
		emit contactStatusChanged( data.section( ' ', 0, 0 ),
			data.section( ' ', 1, 1 ).replace( QRegExp( "%20" ), " " ),
			data.section( ' ', 2, 2 ) );
	}
	else if( cmd == "LST" )
	{
		// handle, publicName, group, list
		emit contactList( data.section( ' ', 4, 4 ),
			data.section( ' ', 5, 5 ).replace( QRegExp( "%20" ), " " ),
			data.section( ' ', 6, 6 ), data.section( ' ', 0, 0 ) );
	}
	else if( cmd == "MSG" )
	{
		QString miss = readBlock( data.section( ' ', 1, 1 ).toUInt() );
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
	}
	else if( cmd == "FLN" )
	{
		emit contactStatusChanged( data.section( ' ', 1, 1 ), "", "FLN" );
	}
	else if( cmd == "ILN" )
	{
		// handle, publicName, Status
		emit contactStatus( data.section( ' ', 1, 1 ),
			data.section( ' ', 2, 2 ).replace( QRegExp( "%20" ), " " ),
			data.section( ' ', 0, 0 ) );
	}
	else if( cmd == "GTC" )
	{
		kdDebug() << "GTC: is not implemented!" << endl;
	}
	else if( cmd == "BLP" )
	{
		kdDebug() << "BLP: is not implemented!" << endl;
	}
	else if( cmd == "RNG" )
	{
		// SessionID, Address, AuthInfo, handle, publicName
		emit invitedToChat( str.section( ' ', 1, 1 ),
			data.section( ' ', 0, 0 ), data.section( ' ', 2, 2 ),
			data.section( ' ', 3, 3 ),
			data.section( ' ', 4, 4 ).replace( QRegExp( "%20" ), " " ) );
	}
	else if( cmd == "ADD" )
	{
		uint group;
		if( data.section( ' ', 0, 0 ) == "FL" )
			group = data.section( ' ', 4, 4 ).toUInt();
		else
			group = 0;

		// handle, publicName, List, serial , group
		emit contactAdded( data.section( ' ', 2, 2 ),
			data.section( ' ', 2, 2 ).replace( QRegExp( "%20" ), " " ),
			data.section( ' ', 0, 0 ), data.section( ' ', 1, 1 ).toUInt(),
			group );
	}
	else if( cmd == "REM" ) // someone is removed from a list
	{
		uint group;
		if( data.section( ' ', 0, 0 ) == "FL" )
			group = data.section( ' ', 3, 3 ).toUInt();
		else
			group = 0;

		// handle, list, serial, group
		contactRemoved( data.section( ' ', 2, 2 ), data.section( ' ', 0, 0 ),
			data.section( ' ', 1, 1 ).toUInt(), group );
	}
	else if( cmd == "OUT" )
	{
		if( str.section( ' ', 1, 1 ) == "OTH" )
		{
			KMessageBox::information( 0,
				i18n( "You have connected from an other client" ) );
		}

		slotSocketClose();
		emit sessionClosed( str.section( ' ', 1, 1 ) );
	}
	else if( cmd == "CHG" )
	{
		if( !isConnected )
		{
			isConnected = true;
			emit connected( true );
		}
		statusChanged( data.section( ' ', 0, 0 ) );
	}
	else if( cmd == "REA" )
	{
		emit publicNameChanged( data.section( ' ', 1, 1 ),
			data.section( ' ', 2, 2 ).replace( QRegExp( "%20" ), " " ) );
	}
	else if( cmd == "LSG" )
	{
		if( str.contains("1 1 0 ~ 0") )
		{
			emit groupName( i18n( "Friends" ), 0 );
			renameGroup( i18n( "Friends" ),0 );
		}
		else
		{
			// groupName, group
			emit groupName( data.section( ' ', 4, 4 ).replace(
				QRegExp( "%20" )," " ), data.section( ' ', 3, 3 ).toUInt() );
		}
	}
	else if( cmd == "ADG" )
	{
		// groupName, serial, group
		emit groupAdded( data.section( ' ', 1, 1 ).replace(
			QRegExp( "%20" ), " " ), data.section( ' ', 0, 0 ).toUInt(),
			data.section( ' ', 2, 2 ).toUInt() );
	}
	else if( cmd == "REG" )
	{
		// groupName, serial, group
		emit groupRenamed( data.section( ' ', 2, 2 ).replace(
			QRegExp( "%20" ), " " ), data.section( ' ', 0, 0 ).toUInt(),
			data.section( ' ', 1, 1 ).toUInt() );
	}
	else if( cmd == "RMG" )
	{
		emit groupRemoved( data.section( ' ', 0, 0 ).toUInt(),
			data.section( ' ', 1, 1 ).toUInt() );
	}
	else if( cmd  == "CHL" )
	{
		kdDebug() << "Sending final Authentication" << endl;
		KMD5 context(
			data.section( ' ', 0, 0 ).replace( QRegExp( "\r\n" ), "" ) +
			"Q1P7W2E4J9R8U3S5" );
		sendCommand( "QRY", "msmsgs@msnmsgr.com 32\r\n" +
			context.hexDigest() );
	}
	else if( cmd == "SYN" )
	{
		// this is the current serial on the server, if its different with the own we can get the user list
		//isConnected = true;
		uint serial = data.section( ' ', 0, 0 ).toUInt();
		if( serial != _serial)
		{
			emit newSerial(serial);  // remove all contacts, msn sends a new contact list
			_serial = serial;
		}
		//emit connected(true);
		// set the status to online
		// create an option in config dialog to select the state to be set
		setStatus(MSNProtocol::NLN);
		sendCVR();
	}
	else
	{
		kdDebug() << "KMSNServiceSocket::parseCommand: Unknown command '" <<
			cmd << " " << m_id << " " << data << "'!" << endl;
	}
}

void KMSNServiceSocket::sendCVR()
{

}

void KMSNServiceSocket::sendCommand( const QString &cmd, const QString &args )
{
	QString data = cmd + " " + QString::number( m_id );
	if( !args.isEmpty() )
		data += " " + args;
	data += "\r\n";

	kdDebug() << "KMSNServiceSocket::sendCommand: Sending command " << data;

	socket->writeBlock( data, data.length() );

	m_id++;
}

/* MSN Service has send a new IP , so connect to it */
void KMSNServiceSocket::newConnect( const QString &server, uint port )
{
	socket->flush();
	socket->closeNow();
	delete socket;

	kdDebug() << "Connecting to new server " << server << ":" << port << endl;
	connectInternal( server, port );
}

void KMSNServiceSocket::addGroup(QString groupName)
{
	// escape spaces
	sendCommand( "ADG", groupName.replace( QRegExp( " " ), "%20" ) + " 0" );
}

void KMSNServiceSocket::renameGroup( QString groupName, uint group )
{
	// escape spaces
	sendCommand( "REG", QString::number( group ) + " " +
		groupName.replace( QRegExp( " " ), "%20" ) + " 0" );
}

void KMSNServiceSocket::removeGroup( uint group )
{
	sendCommand( "RMG", QString::number( group ) );
}

void KMSNServiceSocket::addContact( const QString &handle,
	QString publicName, uint group, int list )
{
	QString args;
	switch( list )
	{
	case MSNProtocol::FL:
		args = "FL " + handle + " " + handle + " " + QString::number( group );
		break;
	case MSNProtocol::AL:
		args = "AL " + handle + " " +
			publicName.replace( QRegExp( " " ), "%20" );
		break;
	case MSNProtocol::BL:
		args = "BL " + handle + " " +
			publicName.replace( QRegExp( " " ), "%20" );
		break;
	default:
		kdDebug() << "KMSNServiceSocket::addContact: WARNING! Unknown list " <<
			list << "!" << endl;
		return;
	}
	sendCommand( "ADD", args );
}

void KMSNServiceSocket::removeContact( const QString &handle, uint group,
	int list )
{
	QString args;
	switch( list )
	{
	case MSNProtocol::FL:
		args = "FL " + handle + " " + QString::number( group );
		break;
	case MSNProtocol::AL:
		args = "AL " + handle;
		break;
	case MSNProtocol::BL:
		args = "BL " + handle;
		break;
	default:
		kdDebug() << "KMSNServiceSocket::removeContact: " <<
			"WARNING! Unknown list " << list << "!" << endl;
		return;
	}
	sendCommand( "REM", args );
}

void KMSNServiceSocket::setStatus( int status )
{
	sendCommand( "CHG", statusToString( status ) );
}

void KMSNServiceSocket::changePublicName( const QString &publicName )
{
	QString pn = publicName;
	sendCommand( "REA", _handle + " " + pn.replace( QRegExp( " " ), "%20" ) );
}

void KMSNServiceSocket::createChatSession()
{
	sendCommand( "XFR", "SB" );
}

QString KMSNServiceSocket::statusToString( int status ) const
{
	switch( status )
	{
	case MSNProtocol::NLN:
		return "NLN";
	case MSNProtocol::BSY:
		return "BSY";
	case MSNProtocol::BRB:
		return "BRB";
	case MSNProtocol::AWY:
		return "AWY";
	case MSNProtocol::PHN:
		return "PHN";
	case MSNProtocol::LUN:
		return "LUN";
	case MSNProtocol::FLN:
		return "FLN";
	case MSNProtocol::HDN:
		return "HDN";
	case MSNProtocol::IDL:
		return "IDL";
	default:
		kdDebug() << "KMSNServiceSocket::statusToString: " <<
			"WARNING! Unknown status " << status << "!" << endl;
		return QString::null;
	}
}

#include "kmsnservicesocket.moc"

// vim: set noet ts=4 sts=4 sw=4:

