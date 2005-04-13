/*
    msnswitchboardsocket.cpp - switch board connection socket

    Copyright (c) 2002      by Martijn Klingens       <klingens@kde.org>
    Copyright (c) 2002-2005 by Olivier Goffart        <ogoffart@ kde.org>
    Kopete    (c) 2002-2005 by the Kopete developers  <kopete-devel@kde.org>

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
#include <cmath>

// qt
#include <qstylesheet.h>
#include <qregexp.h>
#include <qimage.h>
#include <qtimer.h>

// kde
#include <kdebug.h>
#include <kmessagebox.h>
#include <kapplication.h>
#include <kaboutdata.h>
#include <ktempfile.h>
#include <kconfig.h>

// for the display picture
#include "msnp2pdisplatcher.h"
#include <msncontact.h>

//kopete
#include "msnaccount.h"
#include "msnprotocol.h"
#include "kopetemessage.h"
#include "kopetecontact.h"
#include "kopeteuiglobal.h"



MSNSwitchBoardSocket::MSNSwitchBoardSocket( MSNAccount *account , QObject *parent )
: MSNSocket( parent )
{
	m_account = account;
	m_p2p=0l;
	m_recvIcons=0;
	m_emoticonTimer=0L;
	m_chunks=0;
}

MSNSwitchBoardSocket::~MSNSwitchBoardSocket()
{
	kdDebug(14140) << k_funcinfo << endl;

	QMap<QString , QPair<QString , KTempFile*> >::Iterator it;
	for ( it = m_emoticons.begin(); it != m_emoticons.end(); ++it )
	{
		delete it.data().second;
	}


}

void MSNSwitchBoardSocket::connectToSwitchBoard(QString ID, QString address, QString auth)
{
	// we need these for the handshake later on (when we're connected)
	m_ID = ID;
	m_auth = auth;

	QString server = address.left( address.find( ":" ) );
	uint port = address.right( address.length() - address.findRev( ":" ) - 1 ).toUInt();

	QObject::connect( this, SIGNAL( blockRead( const QString & ) ),
		this, SLOT(slotReadMessage( const QString & ) ) );

	QObject::connect( this, SIGNAL( onlineStatusChanged( MSNSocket::OnlineStatus ) ),
		this, SLOT( slotOnlineStatusChanged( MSNSocket::OnlineStatus ) ) );

	QObject::connect( this, SIGNAL( socketClosed( ) ),
		this, SLOT( slotSocketClosed( ) ) );

	connect( server, port );
}

void MSNSwitchBoardSocket::handleError( uint code, uint id )
{
	switch( code )
	{
		case 208:
		{
			QString msg = i18n( "Invalid user:\n"
				"this MSN user does not exist; please check the MSN ID." );
			KMessageBox::queuedMessageBox( Kopete::UI::Global::mainWidget(), KMessageBox::Error, msg, i18n( "MSN Plugin" ) );
			userLeftChat(m_msgHandle , i18n("user never joined"));
			break;
		}
		case 215:
		{
			QString msg = i18n( "The user %1 is already in this chat." ).arg( m_msgHandle );
			KMessageBox::queuedMessageBox( Kopete::UI::Global::mainWidget(), KMessageBox::Error, msg, i18n( "MSN Plugin" ) );
			//userLeftChat(m_msgHandle , i18n("user was twice in this chat") ); //(the user shouln't join there
			break;
		}
		case 216:
		{
			QString msg = i18n( "The user %1 is online but has blocked you:\nyou can not talk to this user." ).arg( m_msgHandle );
			KMessageBox::queuedMessageBox( Kopete::UI::Global::mainWidget(), KMessageBox::Information, msg, i18n( "MSN Plugin" ) );
			userLeftChat(m_msgHandle, i18n("user blocked you"));
			break;
		}
		case 217:
		{
			// TODO: we need to know the nickname instead of the handle.
			QString msg = i18n( "The user %1 is currently not signed in.\n" "Messages will not be delivered." ).arg( m_msgHandle );
			KMessageBox::queuedMessageBox( Kopete::UI::Global::mainWidget(), KMessageBox::Error, msg, i18n( "MSN Plugin" ) );
			userLeftChat(m_msgHandle, i18n("user disconnected"));
			break;
		}
		case 713:
		{
			QString msg = i18n( "You are trying to invite too many contacts to this chat at the same time" ).arg( m_msgHandle );
			KMessageBox::queuedMessageBox( Kopete::UI::Global::mainWidget(), KMessageBox::Information, msg, i18n( "MSN Plugin" ) );
			userLeftChat(m_msgHandle, i18n("user blocked you"));
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
		cleanQueue(); //in case some message are waiting their emoticons, never mind, send them

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
		  //that's why the official client does not uptade alaws the nickname immediately.
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
	QRegExp rx("Content-Type: ([A-Za-z0-9$!*/\\-]*)");
	rx.search(msg);
	QString type=rx.cap(1);

	rx=QRegExp("User-Agent: ([A-Za-z0-9.$!*/\\-]*)");
	rx.search(msg);
	QString clientStr=rx.cap(1);

	if( !clientStr.isNull() && !m_msgHandle.isNull())
	{
		Kopete::Contact *c=m_account->contacts()[m_msgHandle];
		if(c)
			c->setProperty(  MSNProtocol::protocol()->propClient , clientStr );
	}

	// incoming message for File-transfer
	if( type== "text/x-msmsgsinvite"  )
	{
		emit invitation(m_msgHandle,msg);
	}
	else if( type== "text/x-msmsgscontrol" )
	{
		QString message;
		message = msg.right( msg.length() - msg.findRev( " " ) - 1 );
		message = message.replace(  "\r\n" ,"" );
		emit receivedTypingMsg( message.lower(), true );
	}
	else if(type=="text/plain"   || type.isEmpty() )
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

			rx=QRegExp("X-MMS-IM-Format: ([^\r\n]*)");
			rx.search(msg);
			fontInfo =rx.cap(1);

			color = parseFontAttr(fontInfo, "CO");
			
			// FIXME: this is so BAAAAAAAAAAAAD :(
			if (!color.isEmpty() && color.toInt(0,16)!=0)
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

			fontName = parseFontAttr(fontInfo, "FN").replace(  "%20" , " " );

			// Some clients like Trillian and Kopete itself send a font
			// name of 'MS Serif' since MS changed the server to
			// _require_ a font name specified in june 2002.
			// MSN's own client defaults to 'MS Sans Serif', which also
			// has issues.
			// Handle 'MS Serif' and 'MS Sans Serif' as an empty font name
			if( !fontName.isEmpty() && fontName != "MS Serif" && fontName != "MS Sans Serif" )
			{
				QString ef=parseFontAttr( fontInfo, "EF" );

				font = QFont( fontName,
					parseFontAttr( fontInfo, "PF" ).toInt(), // font size
					ef.contains( 'B' ) ? QFont::Bold : QFont::Normal,
					ef.contains( 'I' ) );
				font.setUnderline(ef.contains( 'U' ));
				font.setStrikeOut(ef.contains( 'S' ));
			}
		}

		QPtrList<Kopete::Contact> others;
		others.append( m_account->myself() );
		QStringList::iterator it2;
		for( it2 = m_chatMembers.begin(); it2 != m_chatMembers.end(); ++it2 )
		{
			if( *it2 != m_msgHandle )
				others.append( m_account->contacts()[ *it2 ] );
		}

		QString message=msg.right( msg.length() - msg.find("\r\n\r\n") - 4 );

		//Stupid MSN PLUS colors code. message with incorrect charactere are not showed correctly in the chatwindow.
		//TODO: parse theses one to show the color too in Kopete
		message.replace("\3","").replace("\4","").replace("\2","").replace("\5","").replace("\6","").replace("\7","");

		if(!m_account->contacts()[m_msgHandle])
		{
			//this may happens if the contact has been deleted.
			kdDebug(14140) << k_funcinfo <<"WARNING: contact is null, adding it" <<endl;
			if( !m_chatMembers.contains( m_msgHandle ) )
				m_chatMembers.append( m_msgHandle );
			emit userJoined( m_msgHandle , m_msgHandle , false);
		}

		Kopete::Message kmsg( m_account->contacts()[ m_msgHandle ], others,
			message,
			Kopete::Message::Inbound , Kopete::Message::PlainText );

		kmsg.setFg( fontColor );
		kmsg.setFont( font );

		rx=QRegExp("Chunks: ([0-9]*)");
		rx.search(msg);
		unsigned int chunks=rx.cap(1).toUInt();
		rx=QRegExp("Chunk: ([0-9]*)");
		rx.search(msg);
		unsigned int chunk=rx.cap(1).toUInt();

		if(chunk != 0 && !m_msgQueue.isEmpty())
		{
			QString msg=m_msgQueue.last().plainBody();
			m_msgQueue.pop_back(); //removes the last item
			kmsg.setBody( msg+ message, Kopete::Message::PlainText );
		}

		if(chunk == 0 )
			m_chunks=chunks;
		else if(chunk+1 >=  m_chunks)
			m_chunks=0;

		if ( m_recvIcons > 0  || m_chunks > 0)
		{ //Some custom emoticons are waiting to be received. so append the message to the queue
		  //Or the message has not been fully received, so same thing
			kdDebug(14140) << k_funcinfo << "Message not fully received => append to queue.  Emoticon left: " << m_recvIcons << "  chunks: " << chunk+1 << " of " << m_chunks   <<endl;
			m_msgQueue.append( kmsg );
			if(!m_emoticonTimer) //to be sure no message will be lost, we will appends message to
			{                    // the queue in 15 secondes even if we have not received emoticons
				m_emoticonTimer=new QTimer(this);
				QObject::connect(m_emoticonTimer , SIGNAL(timeout()) , this, SLOT(cleanQueue()));
				m_emoticonTimer->start( 15000 , true );
			}
		}
		else
			emit msgReceived( parseCustomEmoticons( kmsg ) );

	}
	else if( type== "text/x-mms-emoticon" )
	{
		KConfig *config = KGlobal::config();
		config->setGroup( "MSN" );
		if ( config->readBoolEntry( "useCustomEmoticons", false ) )
		{
			QRegExp rx("([^\\s]*)[\\s]*(<msnobj [^>]*>)");
			rx.setMinimal(true);
			int pos = rx.search(msg);
			while( pos != -1)
			{
				QString msnobj=rx.cap(2);
				QString txt=rx.cap(1);
				kdDebug(14140) << k_funcinfo << "emoticon: " <<  txt << "    msnobj: " << msnobj<<  endl;

				if( !m_emoticons.contains(msnobj) || !m_emoticons[msnobj].second )
				{
					m_emoticons.insert(msnobj, qMakePair(txt,(KTempFile*)0L));
					MSNContact *c=static_cast<MSNContact*>(m_account->contacts()[m_msgHandle]);
					if(!c)
						return;

					if(!m_p2p)
					{
						m_p2p=new MSNP2PDisplatcher(this , "msnp2p protocol" );
						QObject::connect( this, SIGNAL( blockRead( const QByteArray & ) ),    m_p2p, SLOT(slotReadMessage( const QByteArray & ) ) );
						QObject::connect( m_p2p, SIGNAL( sendCommand( const QString &, const QString &, bool , const QByteArray & , bool ) )  ,
								this , SLOT(sendCommand( const QString &, const QString &, bool , const QByteArray & , bool )));
						QObject::connect( m_p2p, SIGNAL( fileReceived( KTempFile *, const QString& ) ) , this , SLOT(slotEmoticonReceived( KTempFile *, const QString& ) ) ) ;
					}

					// we are receiving emoticons, so delay message display until received signal
					m_recvIcons++;
					m_p2p->requestDisplayPicture( m_myHandle, m_msgHandle, msnobj );
				}
				pos=rx.search(msg, pos+rx.matchedLength());
			}
		}
	}
	else if( type== "application/x-msnmsgrp2p" )
	{
		if(!m_p2p)
		{
			m_p2p=new MSNP2PDisplatcher(this , "msnp2p protocol" );
			QObject::connect( this, SIGNAL( blockRead( const QByteArray & ) ),    m_p2p, SLOT(slotReadMessage( const QByteArray & ) ) );
			QObject::connect( m_p2p, SIGNAL( sendCommand( const QString &, const QString &, bool , const QByteArray & , bool ) )  ,
				this , SLOT(sendCommand( const QString &, const QString &, bool , const QByteArray & , bool )));
			QObject::connect( m_p2p, SIGNAL( fileReceived( KTempFile *, const QString& ) ) , this , SLOT(slotEmoticonReceived( KTempFile *, const QString& ) ) ) ;
		}
	}
	else
	{
		kdDebug(14140) << k_funcinfo <<" Unknown type '" << type << "' message: \n"<< msg <<endl;
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
int MSNSwitchBoardSocket::sendMsg( const Kopete::Message &msg )
{
	if ( onlineStatus() != Connected || m_chatMembers.empty())
	{
//		m_messagesQueue.append(msg);
		return -1;
	}

#if MSN_WEBCAM   //this is to test webcam
	if(msg.plainBody().contains("/webcam"))
	{
		if(!m_p2p)
		{
			m_p2p=new MSNP2PDisplatcher(this , "msnp2p protocol" );
			QObject::connect( this, SIGNAL( blockRead( const QByteArray & ) ),    m_p2p, SLOT(slotReadMessage( const QByteArray & ) ) );
			QObject::connect( m_p2p, SIGNAL( sendCommand( const QString &, const QString &, bool , const QByteArray & , bool ) )  ,
					this , SLOT(sendCommand( const QString &, const QString &, bool , const QByteArray & , bool )));
			QObject::connect( m_p2p, SIGNAL( fileReceived( KTempFile *, const QString& ) ) , this , SLOT(slotEmoticonReceived( KTempFile *, const QString& ) ) ) ;
		}
		m_p2p->startWebcam( m_myHandle , m_msgHandle);
		return -3;
	}
#endif


	if( msg.format() & Kopete::Message::RichText )
	{
		QRegExp rx("^\\s*<img src=\"([^>\"]+)\"[^>]*>\\s*$");
		if(rx.search(msg.escapedBody()) != -1)
		{
			if(!m_p2p)
			{
				m_p2p=new MSNP2PDisplatcher(this , "msnp2p protocol" );
				QObject::connect( this, SIGNAL( blockRead( const QByteArray & ) ),    m_p2p, SLOT(slotReadMessage( const QByteArray & ) ) );
				QObject::connect( m_p2p, SIGNAL( sendCommand( const QString &, const QString &, bool , const QByteArray & , bool ) )  ,
						this , SLOT(sendCommand( const QString &, const QString &, bool , const QByteArray & , bool )));
				QObject::connect( m_p2p, SIGNAL( fileReceived( KTempFile *, const QString& ) ) , this , SLOT(slotEmoticonReceived( KTempFile *, const QString& ) ) ) ;
			}

			m_p2p->sendImage(rx.cap(1));

			return -3;
		}
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
	else head+="FN=; EF=; ";
	/**
	 * I don't know what to set by default, so i decided to set nothing.  CF Bug 82734
	 * (but don't forgeto to add an empty FN= and EF= ,  or webmessenger will break. (CF Bug 102371) )
	else head+="FN=MS%20Serif; EF=; ";
	 */

	// Color support
	if (msg.fg().isValid())
	{
		QString colorCode = QColor(msg.fg().blue(),msg.fg().green(),msg.fg().red()).name().remove(0,1);  //colors aren't sent in RGB but in BGR (O.G.)
		head += "CO=" + colorCode;
	}
	else
	{
		head += "CO=0";
	}

	head += "; CS=0; PF=0";
	if (msg.plainBody().isRightToLeft())
		head += "; RL=1";
	head += "\r\n";

	QString message= msg.plainBody().replace(  "\n" , "\r\n" );

	//-- Check if the message isn't too big,  TODO: do that at the libkopete level.
	int len_H=head.utf8().length();	// != head.length()  because i need the size in butes and
	int len_M=message.utf8().length();	//    some utf8 char may be longer than one byte
	if( len_H+len_M >= 1660 ) //1664 is the maximum size of messages allowed by the server
	{
		//We will certenly split the message in several ones.
		//It's possible to made the opposite client join them, as explained in this MS Word document
		//http://www.bot-depot.com/forums/index.php?act=Attach&type=post&id=35110

		head+="Message-ID: {7B7B34E6-7A8D-44FF-926C-1799156B58"+QString::number( rand()%10)+QString::number( rand()%10)+"}\r\n";
		int len_H=head.utf8().length()+ 14;   //14 is the size of "Chunks: x"
		//this is the size of each part of the message (excluding the header)
		int futurmessages_size=1400;  //1400 is a common good size
		//int futurmessages_size=1664-len_H;

		int nb=(int)ceil((float)(len_M)/(float)(futurmessages_size));

		if(KMessageBox::warningContinueCancel(0L /* FIXME: we should try to find a parent somewere*/ ,
			i18n("The message you are trying to send is too long; it will be split into %1 messages.").arg(nb) ,
			i18n("Message too big - MSN Plugin" ), KStdGuiItem::cont() , "SendLongMessages" )
				== KMessageBox::Continue )
		{
			int place=0;
			int result;
			int chunk=0;
			do
			{
				QString m=message.mid(place, futurmessages_size);
				place += futurmessages_size;

				//make sure the size is not too big because of utf8
				int d=m.utf8().length() + len_H -1664;
				if( d > 0 )
				{//it contains some utf8 chars, so we strip the string a bit.
					m=m.left( futurmessages_size - d );
					place -= d;
				}

				//try to snip on space if possible
				int len=m.length();
				d=0;
				while(d<200 && !m[len-d].isSpace() )
					d++;
				if(d<200)
				{
					m=m.left(len-d);
					place -= d;
				}
				QString chunk_str;
				if(chunk==0)
					chunk_str="Chunks: "+QString::number(nb)+"\r\n";
				else if(chunk<nb)
					chunk_str="Chunk: "+QString::number(chunk)+"\r\n";
				else
				{
					kdDebug(14140) << k_funcinfo <<"The message is slit in more than initially estimated" <<endl;
				}
				result=sendCommand( "MSG", "A", true, (head+chunk_str+"\r\n"+m).utf8() );
				chunk++;
			}
			while(place < len_M) ;

			while(chunk<nb)
			{
				kdDebug(14140) << k_funcinfo <<"The message is plit in less than initially estimated.  Sending empty message to complete" <<endl;
				QString chunk_str="Chunk: "+QString::number(chunk);
				sendCommand( "MSG", "A", true, (head+chunk_str+"\r\n").utf8() );
				chunk++;
			}
			return result;
		}
		return -2;  //the message hasn't been sent.
	}
	return sendCommand( "MSG", "A", true, (head+"\r\n"+message).utf8() );
}

void MSNSwitchBoardSocket::slotSocketClosed( )
{
	for( QStringList::Iterator it = m_chatMembers.begin(); it != m_chatMembers.end(); ++it )
	{
		emit userLeft( (*it), i18n("connection closed"));
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

void MSNSwitchBoardSocket::requestDisplayPicture()
{
	MSNContact *c=static_cast<MSNContact*>(m_account->contacts()[m_msgHandle]);
	if(!c)
		return;

	if(!m_p2p)
	{
		m_p2p=new MSNP2PDisplatcher(this , "msnp2p protocol" );
		QObject::connect( this, SIGNAL( blockRead( const QByteArray & ) ),    m_p2p, SLOT(slotReadMessage( const QByteArray & ) ) );
		QObject::connect( m_p2p, SIGNAL( sendCommand( const QString &, const QString &, bool , const QByteArray & , bool ) )  ,
				this , SLOT(sendCommand( const QString &, const QString &, bool , const QByteArray & , bool )));
		QObject::connect( m_p2p, SIGNAL( fileReceived( KTempFile *, const QString& ) ) , this , SLOT(slotEmoticonReceived( KTempFile *, const QString& ) ) ) ;
	}


	m_p2p->requestDisplayPicture( m_myHandle, m_msgHandle, c->object());
}

void  MSNSwitchBoardSocket::slotEmoticonReceived( KTempFile *file, const QString &msnObj )
{
	kdDebug(14141) << k_funcinfo << msnObj << endl;

	if(m_emoticons.contains(msnObj))
	{ //it's an emoticon
		m_emoticons[msnObj].second=file;

		if( m_recvIcons > 0 )
			m_recvIcons--;
		kdDebug(14140) << k_funcinfo << "emoticons received queue is now: " << m_recvIcons << endl;

		if ( m_recvIcons <= 0 )
			cleanQueue();
	}
	else if(msnObj=="typewrite")
	{
		QString msg=i18n("<img src=\"%1\" alt=\"Typewrited message\" />" ).arg( file->name() );

	kdDebug(14140) << k_funcinfo << file->name()  <<endl;

		m_typewrited.append(file);
		m_typewrited.setAutoDelete(true);

		QPtrList<Kopete::Contact> others;
		others.append( m_account->myself() );

		QStringList::iterator it2;
		for( it2 = m_chatMembers.begin(); it2 != m_chatMembers.end(); ++it2 )
		{
			if( *it2 != m_msgHandle )
				others.append( m_account->contacts()[ *it2 ] );
		}

		if(!m_account->contacts()[m_msgHandle])
		{
			//this may happens if the contact has been deleted.
			kdDebug(14140) << k_funcinfo <<"WARNING: contact is null, adding it" <<endl;
			if( !m_chatMembers.contains( m_msgHandle ) )
				m_chatMembers.append( m_msgHandle );
			emit userJoined( m_msgHandle , m_msgHandle , false);
		}

		Kopete::Message kmsg( m_account->contacts()[ m_msgHandle ], others,
			msg, Kopete::Message::Inbound , Kopete::Message::RichText );

		emit msgReceived(  kmsg  );
	}
	else //if it is not an emoticon,
	{    // it's certenly the displaypicture.
		MSNContact *c=static_cast<MSNContact*>(m_account->contacts()[m_msgHandle]);
		if(c && c->object()==msnObj)
			c->setDisplayPicture(file);
		else
			delete file;
	}
}

void MSNSwitchBoardSocket::cleanQueue()
{
	kdDebug(14141) << k_funcinfo << m_msgQueue.count() << endl;

	QValueList<const Kopete::Message>::Iterator it_msg;
	for ( it_msg = m_msgQueue.begin(); it_msg != m_msgQueue.end(); ++it_msg )
	{
	 	Kopete::Message kmsg = (*it_msg);
	 	emit msgReceived( parseCustomEmoticons( kmsg ) );
	}
	m_msgQueue.clear();
}

Kopete::Message &MSNSwitchBoardSocket::parseCustomEmoticons(Kopete::Message &kmsg)
{
	QString message=kmsg.escapedBody();
	QMap<QString , QPair<QString , KTempFile*> >::Iterator it;
	for ( it = m_emoticons.begin(); it != m_emoticons.end(); ++it )
	{
		QString es=QStyleSheet::escape(it.data().first);
		KTempFile *f=it.data().second;
		if(message.contains(es) && f)
		{
			QString imgPath = f->name();
			QImage iconImage(imgPath);
			/* We don't use a comple algoritm (like the one in the #if)  because the msn client shows
		     * emoticons like that. So, in that case, we show like the MSN client */
			#if 0
			QString em = QRegExp::escape( es );
			message.replace( QRegExp(QString::fromLatin1( "(^|[\\W\\s]|%1)(%2)(?!\\w)" ).arg(em).arg(em)),
								QString::fromLatin1("\\1<img align=\"center\" width=\"") +
			#endif
			message.replace( es,
						QString::fromLatin1("<img align=\"center\" width=\"") +
						QString::number(iconImage.width()) +
						QString::fromLatin1("\" height=\"") +
						QString::number(iconImage.height()) +
						QString::fromLatin1("\" src=\"") + imgPath +
						QString::fromLatin1("\" title=\"") + es +
						QString::fromLatin1( "\"/>" ) );
			kmsg.setBody(message, Kopete::Message::RichText);
		}
	}
	return kmsg;
}

int MSNSwitchBoardSocket::sendWizz()
{
	QCString message = QString( "MIME-Version: 1.0\r\n"
			"Content-Type: text/x-msnmsgr-datacast\r\n"
			"\r\n"
			"ID: 1\r\n"
			"\r\n\r\n" ).utf8();

	QString args = "U";
	return sendCommand( "MSG", args, true, message );
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

