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


#include "msnswitchboardsocket.h"
#include "msnprotocol.h"
#include "msncontact.h"
#include <time.h>
// qt
#include <qdatetime.h>
#include <qfont.h>
#include <qcolor.h>
// kde
#include <klocale.h>
#include <kglobal.h>
#include <kdebug.h>
#include <kmessagebox.h>

MSNSwitchBoardSocket::MSNSwitchBoardSocket()
{
}

MSNSwitchBoardSocket::~MSNSwitchBoardSocket()
{
}

void MSNSwitchBoardSocket::connectToSwitchBoard(QString ID, QString address, QString auth)
{
	QString server = address.left( address.find( ":" ) );
	uint port = address.right( address.length() - address.findRev( ":" ) - 1 ).toUInt();

	QObject::connect( this, SIGNAL( blockRead( const QString & ) ),
		this, SLOT(slotReadMessage( const QString & ) ) );

	QObject::connect( this, SIGNAL( onlineStatusChanged( MSNSocket::OnlineStatus ) ),
		this, SLOT( slotOnlineStatusChanged( MSNSocket::OnlineStatus ) ) );

	QObject::connect( this, SIGNAL( socketClosed( int ) ),
		this, SLOT( slotSocketClosed( int ) ) );

	connect( server, port );

	// we need these for the handshake later on (when we're connected)
	m_ID = ID;
	m_auth = auth;

}

void MSNSwitchBoardSocket::handleError( uint code, uint id )
{
	switch( code )
	{
		case 217:
		{
			// TODO: we need to know the nickname instead of the handle.
			QString msg = i18n( "The user %1 is currently not signed in.\n"
				"Messages will not be delivered." ).arg( m_msgHandle );
			KMessageBox::error( 0, msg, i18n( "MSN Plugin - Kopete" ) );
			slotSocketClosed( 0 ); // emit signal to get ourselves removed...
			break;
		}
		default:
			MSNSocket::handleError( code, id );
			break;
	}
}

void MSNSwitchBoardSocket::parseCommand( const QString &cmd, uint id,
	const QString &data )
{
	kdDebug() << "MSNSwitchBoardSocket::parseCommand" << endl;

	if( cmd == "NAK" )
	{
		emit msgAcknowledgement(false);    // msg was not accepted
	}
	else if( cmd == "ACK" )
	{
		emit msgAcknowledgement(true);   // msg has received
	}
	else if( cmd == "JOI" )
	{
		// new user joins the chat, update user in chat list
		emit switchBoardIsActive(true);
		QString handle = data.section( ' ', 0, 0 );
		emit updateChatMember( handle, "JOI", false );

		if( !m_chatMembers.contains( handle ) )
			m_chatMembers.append( handle );
	}
	else if( cmd == "IRO" )
	{
		// we have joined a multi chat session- this are the users in this chat
		emit switchBoardIsActive(true);
		QString handle = data.section( ' ', 3, 3 );
		if( !m_chatMembers.contains( handle ) )
			m_chatMembers.append( handle );

		if( data.section( 1, 1 )  == data.section( ' ', 2, 2 ) )
			emit updateChatMember( handle, "IRO", true );
		else
			emit updateChatMember( handle, "IRO", false );
	}
	else if( cmd == "USR" )
	{
		callUser();
	}
	else if( cmd == "BYE" )
	{
		// some has disconnect from chat, update user in chat list
		QString handle = data.section( ' ', 0, 0 ).replace(
			QRegExp( "\r\n" ), "" );

		emit updateChatMember( handle, "BYE", false );
		if( m_chatMembers.contains( handle ) )
			m_chatMembers.remove( handle );

		kdDebug() << "MSNSwitchBoardSocket::parseCommand: " <<
			handle << " left the chat." << endl;

		// FIXME: When a user leaves the chat, we still try to contact him by
		// this socket. We have to let MSNProtocol know the switchboard was
		// disconnected, so a new one has to be created when sending a message.
		// Currently, new messages sent when the socket is closed will not appear
		// in the chat window, nor be sent.

		// When this happens, ANY communication with the other party is
		// IMPOSSIBLE! We *need* to fix this soon, but it will require
		// modification to MSNProtocol most likely.
		disconnect();
	}
	else if( cmd == "MSG" )
	{
		kdDebug() << "MSNSwitchBoardSocket::parseCommand: Received MSG: " << endl;
		QString len = data.section( ' ', 2, 2 );

		// we need to know who's sending is the block...
		m_msgHandle = data.section( ' ', 0, 0 );

		readBlock(len.toUInt());
	}
}
void MSNSwitchBoardSocket::slotReadMessage( const QString &msg )
{
	kdDebug() << "MSNSwitchBoardSocket::slotReadMessage" << endl;

	// incoming message for File-transfer
	if( msg.contains("Content-Type: text/x-msmsgsinvite; charset=UTF-8") )
	{
		// filetransfer ,this comes in a later release
		// needs some debugging time
		kdDebug() << "filetransfer : " << msg << endl;

		/*if( msg.contains("Invitation-Command: ACCEPT") )
		{

		}
		else
		{
			QString cookie = msg.right( msg.length() - msg.find( "Invitation-Cookie:" ) - 19 );
			cookie.truncate( cookie.find("\r\n") );

			kdDebug() << "MSNSwitchBoardService::slotReadMessage: " <<
				"invitation cookie: " << cookie << endl;

			QCString message=QString(
				"MIME-Version: 1.0\r\n"
				"Content-Type: text/x-msmsgsinvite; charset=UTF-8\r\n"
				"\r\n"
				"Invitation-Command: ACCEPT\r\n"
				"Invitation-Cookie: " + cookie + "\r\n"
				"Launch-Application: FALSE\r\n"
				"Request-Data: IP-Address: 192.168.0.2\r\n" // hardcoded IP?!
				"Port: 6891").utf8();

				//Application-Name: FileTransfer\r\nApplication-GUID: {5D3E02AB-6190-11d3-BBBB-00C04F795683}\r\nInvitation-Command: INVITE\r\nInvitation-Cookie: 58273\r\nApplication-File: lvback.gif\r\nApplication-FileSize: 4256\r\n\r\n";
			QCString command = "MSG";
			QCString args = QString( "N %1\r\n" ).arg( message.length() ).utf8();
			sendCommand( command + message, args + message, false );
		}*/

		QString contact = MSNProtocol::protocol()->contacts()[ m_msgHandle ]->nickname();
		QString message = i18n("%1 tried to send you a file.\nUnfortunately,"
			" file tranfer is currently not supported.\n").arg( contact );

		KMessageBox::error( 0, message, i18n( "MSN Plugin - Kopete" ) );

	}
	else if(msg.contains("MIME-Version: 1.0\r\nContent-Type: text/x-msmsgscontrol\r\nTypingUser:"))
	{
		QString message;
		message = msg.right(msg.length() - msg.findRev(" ")-1);
		message = message.replace(QRegExp("\r\n"),"");
		emit userTypingMsg(message);    // changed 20.10.2001
	}
	else// if(msg.contains("Content-Type: text/plain;"))
	{
		// Some MSN Clients (like CCMSN) don't like to stick to the rules.
		// In case of CCMSN, it doesn't send what the content type is when
		// sending a text message. So if it's not supplied, we'll just
		// assume its that.

		QColor fontColor;
		QFont font;

		if ( msg.contains( "X-MMS-IM-Format" ) )
		{
			QString fontName;
			QString fontInfo;
			QString color;
			int pos1 = msg.find( "X-MMS-IM-Format" ) + 15;

			fontInfo = msg.mid(pos1, msg.find("\r\n\r\n") - pos1 );
			color = parseFontAttr(fontInfo, "CO");

			// FIXME: this is so BAAAAAAAAAAAAD :(
			if (!color.isEmpty())
			{
				if ( color.length() == 2) // only #RR (red color) given
					fontColor.setRgb(
						color.mid(0,2).toInt(0,16),
						0,
						0);
				else if ( color.length() == 4) // #GGRR (green, red) given.
				{
					fontColor.setRgb(
						color.mid(2,2).toInt(0,16),
						color.mid(0,2).toInt(0,16),
						0);
				}
				else if ( color.length() == 6) // full #BBGGRR given
				{
					fontColor.setRgb(
						color.mid(4,2).toInt(0, 16),
						color.mid(2,2).toInt(0,16),
						color.mid(0,2).toInt(0,16));
				}
			}

			// FIXME: The below regexps do work, but are quite ugly.
			// Reason is that a \1 inside the replacement string is
			// not possible.
			// When importing kopete into kdenetwork, convert this to
			// KRegExp3 from libkdenetwork, which does exactly this.
			fontName = fontInfo.replace(
				QRegExp( ".*FN=" ), "" ).replace(
				QRegExp( ";.*" ), "" ).replace( QRegExp( "%20" ), " " );

			if( !fontName.isEmpty() )
			{
				kdDebug() << "MSNSwitchBoardService::slotReadMessage: Font: '" <<
					fontName << "'" << endl;

				font = QFont( fontName,
					parseFontAttr( fontInfo, "PF" ).toInt(), // font size
					parseFontAttr( fontInfo, "EF" ).contains( 'B' ) ? QFont::Bold : QFont::Normal,
					parseFontAttr( fontInfo, "EF" ).contains( 'I' ) ? true : false );
			}
		}

		kdDebug() << "MSNSwitchBoardService::slotReadMessage: Message: " <<
			endl << msg.right( msg.length() - msg.find("\r\n\r\n") - 4) <<
			endl;

		kdDebug() << "MSNSwitchBoardService::slotReadMessage: User handle: "
			<< m_msgHandle << endl;

		// FIXME: THIS IS UGLY!!!!!!!!!!!!!!!!!!!!!!
		KopeteContactPtrList others;
		others.append( MSNProtocol::protocol()->myself() );

		KopeteMessage kmsg(
			MSNProtocol::protocol()->contacts()[ m_msgHandle ] , others,
			msg.right( msg.length() - msg.find("\r\n\r\n") - 4 ),
			KopeteMessage::Inbound );

		kmsg.setFg( fontColor );
		kmsg.setFont( font );

		emit msgReceived( kmsg );
	}
}

// this sends the user is typing msg
void MSNSwitchBoardSocket::slotTypingMsg()
{
	QCString message = QString("MIME-Version: 1.0\r\n"
		"Content-Type: text/x-msmsgscontrol\r\n"
		"TypingUser: " + m_myHandle + "\r\n"
		"\r\n").utf8();
	QCString args = QString( "U %1 \r\n" ).arg( message.length() ).utf8();
	sendCommand( "MSG", args + message, false );
}

// this Invites an Contact
void MSNSwitchBoardSocket::slotInviteContact(QString handle)
{
	sendCommand( "CAL", handle.utf8());
}

// this sends a short message to the server
void MSNSwitchBoardSocket::slotSendMsg( const KopeteMessage &msg )
{
	if ( onlineStatus() != Connected )
	// emit msgAcknowledgement( false ); // should we do this?
		return;

	kdDebug() << "MSNSwitchBoardSocket::slotSendMsg" << endl;

	QCString head = QString(
		"MIME-Version: 1.0\r\n"
		"Content-Type: text/plain; charset=UTF-8\r\n"
		"X-MMS-IM-Format: FN=MS%20Serif; EF=; ").utf8();

	// Color support
	if (msg.fg().isValid()) {
		QString colorCode = msg.fg().name().remove(0,1);
		head += "CO=" + colorCode;
	} else {
		head += "CO=0";
	}

	head += "; CS=0; PF=0\r\n"
		"\r\n";

	head += msg.body().replace( QRegExp( "\n" ), "\r\n" ).utf8();
	QCString args = QString( "A %1\r\n" ).arg( head.length() ).utf8();
	sendCommand( "MSG", args + head, false );

	// TODO: send our fonts and colors as well
	KopeteContactPtrList others;
	others.append( MSNProtocol::protocol()->contacts()[ m_myHandle ] );
	emit msgReceived( msg );    // send the own msg to chat window
}

void MSNSwitchBoardSocket::slotSocketClosed( int /*state */)
{
	// we have lost the connection, send a message to chatwindow (this will not displayed)
	emit switchBoardIsActive(false);
	emit switchBoardClosed( this );

}

void MSNSwitchBoardSocket::slotCloseSession()
{
	sendCommand( "OUT", "", true, false );
}

void MSNSwitchBoardSocket::callUser()
{
	sendCommand( "CAL", m_msgHandle.utf8() );
}

// Check if we are connected. If so, then send the handshake.
void MSNSwitchBoardSocket::slotOnlineStatusChanged( MSNSocket::OnlineStatus status )
{
	if ( status != Connected )
		return;

	QCString command;
	QString args;

	if( !m_ID ) // we're inviting
	{
		command = "USR";
		args = m_myHandle + " " + m_auth;
	}
	else // we're invited
	{
		command = "ANS";
		args = m_myHandle + " " + m_auth + " " + m_ID;
	}
	
	sendCommand( command, args.utf8() );
	
	// send active message
	emit switchBoardIsActive(true);

}

// FIXME: This is nasty... replace with a regexp or so.
QString MSNSwitchBoardSocket::parseFontAttr(QString str, QString attr)
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

#include "msnswitchboardsocket.moc"

// vim: set noet ts=4 sts=4 sw=4:

