/*
    msnswitchboardsocket.cpp - switch board connection socket

    Copyright (c) 2002      by Martijn Klingens       <klingens@kde.org>
    Copyright (c) 2002-2003 by Olivier Goffart        <ogoffart@tiscalinet.be>
    Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

    Portions of this code are taken from KMerlin,
              (c) 2001 by Olaf Lueg              <olueg@olsd.de>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/


#include "msnswitchboardsocket.h"

#include <time.h>

// qt

// kde
#include <kdebug.h>
#include <kmessagebox.h>
#include <kapplication.h>
#include <kaboutdata.h>

//kopete
#include "msnaccount.h"
#include "kopetemessage.h"
#include "kopetecontact.h"



MSNSwitchBoardSocket::MSNSwitchBoardSocket( MSNAccount *account )
: MSNSocket( account )
{
	m_account = account;
}

MSNSwitchBoardSocket::~MSNSwitchBoardSocket()
{
	kdDebug(14140) <<"MSNSwitchBoardSocket::~MSNSwitchBoardSocket" <<endl;
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
		this, SLOT( slotSocketClosed( ) ) );

	connect( server, port );

	// we need these for the handshake later on (when we're connected)
	m_ID = ID;
	m_auth = auth;

}

void MSNSwitchBoardSocket::handleError( uint code, uint id )
{
	switch( code )
	{
		case 208:
		{
			QString msg = i18n( "Invalid user! \n"
				"This MSN user does not exist. Please check the MSN ID." );
			KMessageBox::error( 0, msg, i18n( "MSN Plugin" ) );
			userLeftChat(m_msgHandle , i18n("user never joined"));
			break;
		}
		case 215:
		{
			QString msg = i18n( "The user %1 is already on this chat." ).arg( m_msgHandle );
			KMessageBox::error( 0, msg, i18n( "MSN Plugin" ) );
			//userLeftChat(m_msgHandle , i18n("user was twice in this chat") ); //(the user shouln't join there
			break;
		}
		case 216:
		{
			QString msg = i18n( "The user %1 is online but has blocked you. \n"
				"You can't start to chat with them." ).arg( m_msgHandle );
			KMessageBox::error( 0, msg, i18n( "MSN Plugin" ) );
			userLeftChat(m_msgHandle, i18n("user never joined"));
			break;
		}
		case 217:
		{
			// TODO: we need to know the nickname instead of the handle.
			QString msg = i18n( "The user %1 is currently not signed in. \n"
				"Messages will not be delivered." ).arg( m_msgHandle );
			KMessageBox::error( 0, msg, i18n( "MSN Plugin" ) );
			userLeftChat(m_msgHandle, i18n("user disconected"));
			break;
		}
		default:
			MSNSocket::handleError( code, id );
			break;
	}
}

void MSNSwitchBoardSocket::parseCommand( const QString &cmd, uint  id ,
	const QString &data )
{
	if( cmd == "NAK" )
	{
		emit msgAcknowledgement(id, false);    // msg was not accepted
	}
	else if( cmd == "ACK" )
	{
		emit msgAcknowledgement(id, true);   // msg has received
	}
	else if( cmd == "JOI" )
	{
		// new user joins the chat, update user in chat list
		QString handle = data.section( ' ', 0, 0 );
		QString screenname = unescape(data.section( ' ', 1, 1 ));
		if( !m_chatMembers.contains( handle ) )
			m_chatMembers.append( handle );
		emit userJoined( handle, screenname, false );
	}
	else if( cmd == "IRO" )
	{
		// we have joined a multi chat session- this are the users in this chat
		QString handle = data.section( ' ', 2, 2 );
		if( !m_chatMembers.contains( handle ) )
			m_chatMembers.append( handle );

		QString screenname = unescape(data.section( ' ', 3, 3));
		emit userJoined( handle, screenname, true );
	}
	else if( cmd == "USR" )
	{
		slotInviteContact(m_msgHandle);
	}
	else if( cmd == "BYE" )
	{
		// some has disconnect from chat, update user in chat list
		QString handle = data.section( ' ', 0, 0 ).replace( "\r\n" , "" );

		userLeftChat( handle,  (data.section( ' ', 1, 1 ) == "1" ) ? i18n("timeout") : QString::null   );
	}
	else if( cmd == "MSG" )
	{
		QString len = data.section( ' ', 2, 2 );

		// we need to know who's sending is the block...
		m_msgHandle = data.section( ' ', 0, 0 );

		/*//This is WRONG! the displayName is never updated on the switchboeardsocket
		  //so we can't trust it.
		  //that's why the official client does not uptade alaws the nickname immediatly.
		if(m_account->contacts()[ m_msgHandle ])
		{
			QString displayName=data.section( ' ', 1, 1 );
			if(m_account->contacts()[ m_msgHandle ]->displayName() != displayName)
				m_account->contacts()[ m_msgHandle ]->rename(displayName);
		}*/

		readBlock(len.toUInt());
	}
}

void MSNSwitchBoardSocket::slotReadMessage( const QString &msg )
{
	// incoming message for File-transfer
	if( msg.contains("Content-Type: text/x-msmsgsinvite; charset=UTF-8") )
	{
		emit invitation(m_msgHandle,msg);
	}
	else if( msg.contains( "MIME-Version: 1.0\r\nContent-Type: text/x-msmsgscontrol\r\nTypingUser:" ) )
	{
		QString message;
		message = msg.right( msg.length() - msg.findRev( " " ) - 1 );
		message = message.replace( QRegExp( "\r\n" ),"" );
		emit receivedTypingMsg( message, true );
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
			fontName = fontInfo.replace( QRegExp( ".*FN=" ), "" ).replace(
				QRegExp( ";.*" ), "" ).replace( QRegExp( "%20" ), " " );

			// Some clients like Trillian and Kopete itself send a font
			// name of 'MS Serif' since MS changed the server to
			// _require_ a font name specified in june 2002.
			// MSN's own client defaults to 'MS Sans Serif', which also
			// has issues.
			// Handle 'MS Serif' and 'MS Sans Serif' as an empty font name
			if( !fontName.isEmpty() && fontName != "MS Serif" && fontName != "MS Sans Serif" )
			{
				kdDebug(14140) << "MSNSwitchBoardService::slotReadMessage: "
					<< "Font: '" << fontName << "'" << endl;

				font = QFont( fontName,
					parseFontAttr( fontInfo, "PF" ).toInt(), // font size
					parseFontAttr( fontInfo, "EF" ).contains( 'B' ) ? QFont::Bold : QFont::Normal,
					parseFontAttr( fontInfo, "EF" ).contains( 'I' ) ? true : false );
			}
		}

		/*kdDebug(14140) << "MSNSwitchBoardService::slotReadMessage: Message: " <<
			endl << msg.right( msg.length() - msg.find("\r\n\r\n") - 4) <<
			endl;

		kdDebug(14140) << "MSNSwitchBoardService::slotReadMessage: User handle: "
			<< m_msgHandle << endl;*/

		QPtrList<KopeteContact> others;
		others.append( m_account->myself() );
		QStringList::iterator it;
		for( it = m_chatMembers.begin(); it != m_chatMembers.end(); ++it )
		{
			if( *it != m_msgHandle )
				others.append( m_account->contacts()[ *it ] );
		}

		QString message=msg.right( msg.length() - msg.find("\r\n\r\n") - 4 );

		//Stupid MSN PLUS colors code. message with incorrect charactère are not showed correctly in the chatwindow.µ
		//TODO: parse theses one to show the color too in Kopete
		message.replace("\3","").replace("\4","").replace("\2","");

		KopeteMessage kmsg( m_account->contacts()[ m_msgHandle ], others,
			message,
			KopeteMessage::Inbound , KopeteMessage::PlainText );

		kmsg.setFg( fontColor );
		kmsg.setFont( font );

		emit msgReceived( kmsg );
	}
}

void MSNSwitchBoardSocket::sendTypingMsg( bool isTyping )
{
	if( !isTyping )
		return;

	QCString message = QString( "MIME-Version: 1.0\r\n"
		"Content-Type: text/x-msmsgscontrol\r\n"
		"TypingUser: " + m_myHandle + "\r\n"
		"\r\n" ).utf8();

	// Length is appended by sendCommand()
	QString args = "U";
	sendCommand( "MSG", args, true, message );
}

// this Invites an Contact
void MSNSwitchBoardSocket::slotInviteContact(const QString &handle)
{
	m_msgHandle=handle;
	sendCommand( "CAL", handle );
}

// this sends a short message to the server
int MSNSwitchBoardSocket::sendMsg( const KopeteMessage &msg )
{
	if ( onlineStatus() != Connected || m_chatMembers.empty())
	{
//		m_messagesQueue.append(msg);
		return -1;
	}

	// User-Agent is not a official flag, but GAIM has it
	QString head =
		"MIME-Version: 1.0\r\n"
		"Content-Type: text/plain; charset=UTF-8\r\n"
		"User-Agent: Kopete/"+escape(kapp->aboutData()->version())+"\r\n"
		"X-MMS-IM-Format: ";

	if(msg.font() != QFont() )
	{
		head += "FN=" + escape( msg.font().family());
		head += "; EF=";
		if(msg.font().bold())
			head += "B";
		if(msg.font().italic())
			head += "I";
		if(msg.font().strikeOut())
			head += "S";
		if(msg.font().underline())
			head += "U";
		head += "; ";
	}
	else head+="FN=MS%20Serif; EF=; ";

	// Color support
	if (msg.fg().isValid())
	{
		QString colorCode = QColor(msg.fg().blue(),msg.fg().green(),msg.fg().red()).name().remove(0,1);  //colours aren't sent in RGB but in BGR (O.G.)
		head += "CO=" + colorCode;
	}
	else
	{
		head += "CO=0";
	}

	head += "; CS=0; PF=0\r\n"
		"\r\n";

	head += msg.plainBody().replace( QRegExp( "\n" ), "\r\n" );

	return sendCommand( "MSG", "A", true, head );
}

void MSNSwitchBoardSocket::slotSocketClosed( )
{
	for( QStringList::Iterator it = m_chatMembers.begin(); it != m_chatMembers.end(); ++it )
	{
		emit userLeft( (*it), i18n("socket closed"));
	}

	// we have lost the connection, send a message to chatwindow (this will not displayed)
//	emit switchBoardIsActive(false);
	emit switchBoardClosed( );
}

void MSNSwitchBoardSocket::slotCloseSession()
{
	sendCommand( "OUT", QString::null, false );
	disconnect();
}

// Check if we are connected. If so, then send the handshake.
void MSNSwitchBoardSocket::slotOnlineStatusChanged( MSNSocket::OnlineStatus status )
{
	if (status == Connected)
	{
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
		sendCommand( command, args );
	}
}

void MSNSwitchBoardSocket::userLeftChat(const QString& handle , const QString &reason)
{
	emit userLeft( handle,  reason );

	if( m_chatMembers.contains( handle ) )
		m_chatMembers.remove( handle );

	if(m_chatMembers.isEmpty())
		disconnect();
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

