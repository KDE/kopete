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

#include "msnnotifysocket.h"
#include "msndispatchsocket.h"
#include "msnprotocol.h"

#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kmdcodec.h>
#include <kmessagebox.h>

MSNNotifySocket::MSNNotifySocket( const QString &msnId )
: MSNAuthSocket( msnId )
{
	QObject::connect( this, SIGNAL( blockRead( const QString & ) ),
		this, SLOT( slotReadMessage( const QString & ) ) );
		
	m_dispatchSocket = 0L;
	m_newstatus=MSNProtocol::NLN;
}

MSNNotifySocket::~MSNNotifySocket()
{
	kdDebug() << "MSNNotifySocket::~MSNNotifySocket" << endl;
}

void MSNNotifySocket::connect( const QString &pwd )
{
	m_password = pwd;

	m_dispatchSocket = new MSNDispatchSocket( msnId() );
	QObject::connect( m_dispatchSocket,
		SIGNAL( receivedNotificationServer( const QString &, uint ) ),
		this,
		SLOT( slotReceivedServer( const QString &, uint ) ) );

	QObject::connect( m_dispatchSocket,
		SIGNAL( connectionFailed( ) ),
		this,
		SLOT( slotDispatchFailed( ) ) );

	m_dispatchSocket->connect();
}

void MSNNotifySocket::slotReceivedServer( const QString &server, uint port )
{
	MSNAuthSocket::connect( server, port );
	m_dispatchSocket->deleteLater();
	m_dispatchSocket = 0L;
}

void MSNNotifySocket::disconnect()
{
	if( onlineStatus() != Disconnected )
		sendCommand( "OUT", QString::null, false );

	MSNAuthSocket::disconnect();
}

void MSNNotifySocket::handleError( uint code, uint id )
{
	// See http://www.hypothetic.org/docs/msn/basics.php for a
	// description of all possible error codes.
	// TODO: Add support for all of these!
	switch( code )
	{
	case 215:
	{
		QString msg = i18n( "This MSN user already exists in this group!\n"
			"Please note that Kopete doesn't really handle users that exist "
			"in multiple groups yet!\n"
			"If this is not the case, please send us a detailed bug report "
			"at kopete-devel@kde.org containing the raw output on the "
			"console (in gzipped format, as it is probably a lot of output!)" );
		KMessageBox::error( 0, msg, i18n( "MSN Plugin - Kopete" ) );
		break;
	}
	case 911:
	{
		QString msg = i18n( "Authentication failed.\n"
			"Check your username and password in the "
			"MSN Preferences dialog." );
		disconnect();
		KMessageBox::error( 0, msg, i18n( "MSN Plugin - Kopete" ) );
		break;
	}
	case 201:  
	case 205:
	{
		QString msg = i18n( "Invalid user! \n"
			"This MSN user does not exist. Please check the MSN ID" );
		KMessageBox::error( 0, msg, i18n( "MSN Plugin - Kopete" ) );
		break;
	}
	default:
		MSNAuthSocket::handleError( code, id );
		break;
	}
}
void MSNNotifySocket::parseCommand( const QString &cmd, uint id,
	const QString &data )
{
	//kdDebug() << "MSNNotifySocket::parseCommand: Command: " << cmd << endl;

	if( cmd == "USR" )
	{
		if( data.section( ' ', 1, 1 ) == "S" )
		{
			kdDebug() << "Sending response Authentication" << endl;

			KMD5 context( data.section( ' ', 2, 2 ) + m_password );
			sendCommand( "USR", "MD5 S " + context.hexDigest() );
		}
		else
		{
			// Successful auth, sync contact list
			kdDebug() << "Sending serial number" << endl;

			//sendCommand( "SYN", QString::number( _serial ) );
			sendCommand( "SYN", "0" );

			// this is our current user and friendly name
			// do some nice things with it  :-)
			QString publicName = unescape( data.section( ' ', 2, 2 ) );
			emit publicNameChanged( msnId(), publicName );
		}
	}
	else if( cmd == "NLN" )
	{
		// handle, publicName, status
		emit contactStatusChanged( data.section( ' ', 1, 1 ),
			unescape( data.section( ' ', 2, 2 ) ),
			MSNProtocol::convertStatus( data.section( ' ', 0, 0 ) ) );
	}
	else if( cmd == "LST" )
	{
		// handle, publicName, group, list
		emit contactList( data.section( ' ', 4, 4 ),
			unescape( data.section( ' ', 5, 5 ) ), data.section( ' ', 6, 6 ),
			data.section( ' ', 0, 0 ) );
	}
	else if( cmd == "MSG" )
	{
		readBlock( data.section( ' ', 2, 2 ).toUInt() );
	}
	else if( cmd == "FLN" )
	{
		emit contactStatusChanged( data.section( ' ', 0, 0 ),QString::null, MSNProtocol::FLN );
	}
	else if( cmd == "ILN" )
	{
		// handle, publicName, Status
		emit contactStatus( data.section( ' ', 1, 1 ),
			unescape( data.section( ' ', 2, 2 ) ), data.section( ' ', 0, 0 ) );
	}
	else if( cmd == "GTC" )
	{
		kdDebug() << "GTC: is not implemented!" << endl;
	}
	else if( cmd == "BLP" )
	{
		kdDebug() << "BLP: is not implemented!" << endl;
	}
	else if( cmd == "XFR" )
	{
		// Address, AuthInfo
		emit startChat( data.section( ' ', 1, 1 ), data.section( ' ', 3, 3 ) );
	}
	else if( cmd == "RNG" )
	{
		// SessionID, Address, AuthInfo, handle, publicName
		emit invitedToChat( QString::number( id ),
			data.section( ' ', 0, 0 ), data.section( ' ', 2, 2 ),
			data.section( ' ', 3, 3 ),
			unescape( data.section( ' ', 4, 4 ) ) );
	}
	else if( cmd == "ADD" )
	{
		QString msnId = data.section( ' ', 2, 2 );
		uint group;
		if( data.section( ' ', 0, 0 ) == "FL" )
			group = data.section( ' ', 4, 4 ).toUInt();
		else
			group = 0;

		// handle, publicName, List, serial , group
		emit contactAdded( msnId, unescape( msnId ),
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
		emit contactRemoved( data.section( ' ', 2, 2 ), data.section( ' ', 0, 0 ),
			data.section( ' ', 1, 1 ).toUInt(), group );
	}
	else if( cmd == "OUT" )
	{
		if( data.section( ' ', 0, 0 ) == "OTH" )
		{
			kdDebug() << "[MSN-PLUGIN] You have connected from another client" << endl;
			//this provoque crash
			//KMessageBox::information( 0, i18n( "You have connected from another client" ) );
		}

		disconnect();
	}
	else if( cmd == "CHG" )
	{
		QString status = data.section( ' ', 0, 0 );
		setOnlineStatus( Connected );
		emit statusChanged( status );
	}
	else if( cmd == "REA" )
	{
		emit publicNameChanged( data.section( ' ', 1, 1 ),
			unescape( data.section( ' ', 2, 2 ) ) );
	}
	else if( cmd == "LSG" )
	{
		emit groupName( unescape( data.section( ' ', 4, 4 ) ),
		data.section( ' ', 3, 3 ).toUInt() );
	}
	else if( cmd == "ADG" )
	{
		// groupName, serial, group
		emit groupAdded( unescape( data.section( ' ', 1, 1 ) ),
			data.section( ' ', 0, 0 ).toUInt(),
			data.section( ' ', 2, 2 ).toUInt() );
	}
	else if( cmd == "REG" )
	{
		// groupName, serial, group
		emit groupRenamed( unescape( data.section( ' ', 2, 2 ) ),
			data.section( ' ', 0, 0 ).toUInt(),
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
		KMD5 context( data.section( ' ', 0, 0 ) + "Q1P7W2E4J9R8U3S5" );
		sendCommand( "QRY", "msmsgs@msnmsgr.com", true,
			context.hexDigest());
	}
	else if( cmd == "SYN" )
	{
/*		// this is the current serial on the server, if its different with the own we can get the user list
		//isConnected = true;
		uint serial = data.section( ' ', 0, 0 ).toUInt();
		if( serial != _serial)
		{
			emit newSerial(serial);  // remove all contacts, msn sends a new contact list
			_serial = serial;
		}*/
		//emit connected(true);
		// set the status 
		setStatus(m_newstatus);
	}
	else
	{
		// Let the base class handle the rest
		MSNAuthSocket::parseCommand( cmd, id, data );
	}
}

void MSNNotifySocket::slotReadMessage( const QString &msg )
{
	if(msg.contains("Inbox-Unread:"))
	{
/*			 //this sends the server if we are going online, contains the unread message count
		 msg = msg.right(msg.length() - msg.find("Inbox-Unread:") );
		 msg = msg.left(msg.find("\r\n"));
		 mailCount = msg.right(msg.length() -msg.find(" ")-1).toUInt();
		 emit newMail("",mailCount);*/
		 return;
	}
	if(msg.contains("Message-Delta:"))
	{
/*			 //this sends the server if mails are deleted
		 msg = msg.right(msg.length() - msg.find("Message-Delta:") );
		 msg = msg.left(msg.find("\r\n"));
		 mailCount = mailCount - msg.right(msg.length() -msg.find(" ")-1).toUInt();
		 emit newMail("",mailCount);*/
		 return;
	}
	if(msg.contains("From-Addr:"))
	{
/*			 //this sends the server if a new mail has arrived
		 msg = msg.right(msg.length() - msg.find("From-Addr:") );
		 msg = msg.left(msg.find("\r\n"));
		 mailCount++;
		 msg = msg.right(msg.length() -msg.find(" ")-1);
		 emit newMail(msg,mailCount);*/
		 return;
	}
}

void MSNNotifySocket::addGroup(QString groupName)
{
	// escape spaces
	sendCommand( "ADG", escape( groupName ) + " 0" );
}

void MSNNotifySocket::renameGroup( QString groupName, uint group )
{
	// escape spaces
	sendCommand( "REG", QString::number( group ) + " " +
		escape( groupName ) + " 0" );
}

void MSNNotifySocket::removeGroup( uint group )
{
	sendCommand( "RMG", QString::number( group ) );
}

void MSNNotifySocket::addContact( const QString &handle,
	QString publicName, uint group, int list )
{
	QString args;
	switch( list )
	{
	case MSNProtocol::FL:
		args = "FL " + handle + " " + handle + " " + QString::number( group );
		break;
	case MSNProtocol::AL:
		args = "AL " + handle + " " + escape( publicName );
		break;
	case MSNProtocol::BL:
		args = "BL " + handle + " " +
			escape( publicName );
		break;
	default:
		kdDebug() << "MSNNotifySocket::addContact: WARNING! Unknown list " <<
			list << "!" << endl;
		return;
	}
	sendCommand( "ADD", args );
}

void MSNNotifySocket::removeContact( const QString &handle, uint group,
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
		kdDebug() << "MSNNotifySocket::removeContact: " <<
			"WARNING! Unknown list " << list << "!" << endl;
		return;
	}
	sendCommand( "REM", args );
}

void MSNNotifySocket::setStatus( int status )
{
	kdDebug() << "MSNNotifySocket::setStatus : " <<statusToString(status) <<endl;
	if(onlineStatus() == Disconnected)
		m_newstatus=status;
	else
		sendCommand( "CHG", statusToString( status ) );

}

void MSNNotifySocket::changePublicName( const QString &publicName )
{
	QString pn = publicName;
	sendCommand( "REA", msnId() + " " + escape( pn ) );
}

void MSNNotifySocket::createChatSession()
{
	sendCommand( "XFR", "SB" );
}

QString MSNNotifySocket::statusToString( int status ) const
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
		kdDebug() << "MSNNotifySocket::statusToString: " <<
			"WARNING! Unknown status " << status << "!" << endl;
		return QString::null;
	}
}

void MSNNotifySocket::slotDispatchFailed()
{
	emit( onlineStatusChanged( Disconnected ) );
	m_dispatchSocket->deleteLater();
	m_dispatchSocket = 0L;
}

#include "msnnotifysocket.moc"



/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

