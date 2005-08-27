/*
   messagereceivertask.cpp  - Incoming OSCAR Messaging Handler

   Copyright (c) 2004 by Matt Rogers <mattr@kde.org>
   Kopete    (c) 2002-2004 by the Kopete developers  <kopete-devel@kde.org>

   *************************************************************************
   *                                                                       *
   * This program is free software; you can redistribute it and/or modify  *
   * it under the terms of the GNU General Public License as published by  *
   * the Free Software Foundation; either version 2 of the License, or     *
   * (at your option) any later version.                                   *
   *                                                                       *
   *************************************************************************
*/

#include "messagereceivertask.h"

#include <qtextcodec.h>
//Added by qt3to4:
#include <Q3ValueList>
#include <Q3CString>
#include <kdebug.h>
#include "transfer.h"
#include "buffer.h"
#include "connection.h"

#include "oscarutils.h"
#include "userdetails.h"


MessageReceiverTask::MessageReceiverTask( Task* parent ) : Task( parent )
{
}


MessageReceiverTask::~MessageReceiverTask()
{
}


bool MessageReceiverTask::forMe( const Transfer* transfer ) const
{
	const SnacTransfer* st = dynamic_cast<const SnacTransfer*>( transfer );
	if ( !st )
		return false;
	
	if ( st->snacService() == 0x0004 && st->snacSubtype() == 0x0007 )
		return true;
	else
		return false;
}

bool MessageReceiverTask::take( Transfer* transfer )
{
	if ( forMe( transfer ) )
	{
		Buffer* b = transfer->buffer();
		m_icbmCookie = b->getBlock( 8 );
		kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "icbm cookie is " << m_icbmCookie << endl;
		m_channel = b->getWord();
		kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "channel is " << m_channel << endl;
		
		UserDetails ud;
		ud.fill( b );
		m_fromUser = ud.userId();
		
		switch( m_channel )
		{
		case 0x0001:
			setTransfer( transfer );
			handleType1Message();
			setTransfer( 0 );
			return true;
			break;
		case 0x0002:
			setTransfer( transfer );
			handleType2Message();
			setTransfer( 0 );
			return true;
			break;
		case 0x0004:
			setTransfer( transfer );
			handleType4Message();
			setTransfer( 0 );
			return true;
			break;
		default:
			kdWarning(OSCAR_RAW_DEBUG) << "A message was received on an unknown channel. channel is " << m_channel << endl;
			return false;
			break;
		};
	}
	else
		return false;
}

void MessageReceiverTask::handleType1Message()
{
	Oscar::Message msg;
	Q3ValueList<TLV> messageTLVList = transfer()->buffer()->getTLVList();
	TLV t = Oscar::findTLV( messageTLVList, 0x0002 );
	if ( !t )
	{
		kdWarning(OSCAR_RAW_DEBUG) << k_funcinfo << "Received a message packet with no message!" << endl;
		return;
	}
	Buffer messageBuffer( t.data );
	Q3ValueList<TLV> innerTLVList = messageBuffer.getTLVList();
	Q3ValueList<TLV>::iterator it = innerTLVList.begin(), listEnd = innerTLVList.end();
	for ( ; (*it); ++it )
	{
		switch ( ( *it ).type )
		{
		case 0x0501:
			kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Got features tlv. length: " 
				<< ( *it ).length << " data: " << ( *it ).data << endl;
			break;
		case 0x0101:
		{ 
			Buffer message( ( *it ).data );
			m_charSet = message.getWord();
			m_subCharSet = message.getWord();
			kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Message charset: " << m_charSet 
				<< " message subcharset: " << m_subCharSet << endl;
			if ( m_charSet == 0x0000 )
			{ //we can just decode from the raw QByteArray because ascii is 7 bit
				msg.addProperty( Oscar::Message::Latin );
				msg.setText( QString( message.getBlock( ( *it ).length - 4 ) ) );
				kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "message is: " << msg.text() << endl;
			}
			else if ( m_charSet == 0x0002 )
			{
				msg.addProperty( Oscar::Message::UCS2 );
				int messageLength = ( ( *it ).length - 4 ) / 2;
				kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "message length: " << messageLength << endl;
				msg.setText( QString::fromUcs2( message.getWordBlock( messageLength ) ) );
				kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "message is: " << msg.text() << endl;
			}
			else
			{
				msg.addProperty( Oscar::Message::UTF8 );
				kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Attempting to decode message with QChar array" << endl;
				int messageLength = ( ( *it ).length - 4 );
				QChar* testString = new QChar[messageLength];
				for ( int i = 0; i < messageLength; i++ )
					testString[i] = message.getByte();
				
				msg.setText( QString( testString, messageLength ) );
				kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "message is: " << msg.text() << endl;
			}
			break;
		} //end case
		default:
			kdDebug(OSCAR_RAW_DEBUG) << "Ignoring TLV of type " << ( *it ).type << endl;
			break;
		} //end switch
	}
	
	TLV autoResponse = Oscar::findTLV( messageTLVList, 0x0004 );
	if ( autoResponse )
	{
		kdDebug(OSCAR_RAW_DEBUG) << "auto response message" << endl;
		msg.addProperty( Oscar::Message::AutoResponse );
	}
	else
		msg.addProperty( Oscar::Message::Normal );
	
	msg.setSender( m_fromUser );
	msg.setReceiver( client()->userId() );
	msg.setTimestamp( QDateTime::currentDateTime() );
	msg.setType( 0x01 );
	
	emit receivedMessage( msg );
}

void MessageReceiverTask::handleType2Message()
{
	kdDebug(14151) << k_funcinfo << "We don't _really_ support type2 messages yet..." << endl;

	Oscar::Message msg;
	Q3ValueList<TLV> messageTLVList = transfer()->buffer()->getTLVList();
	TLV t = Oscar::findTLV( messageTLVList, 0x0005 );
	if ( !t )
	{
		kdWarning(OSCAR_RAW_DEBUG) << k_funcinfo << "Received a channel 2 message packet with no message!" << endl;
		return;
	}
	Buffer messageBuffer( t.data );
	Q3ValueList<TLV> innerTLVList = messageBuffer.getTLVList();
	Q3ValueList<TLV>::iterator it = innerTLVList.begin(), listEnd = innerTLVList.end();
	for ( ; (*it); ++it )
	{
		switch ( ( *it ).type )
		{
		case 0x0004:
			kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Got external ip: "
				<< ( *it ).length << " data: " << ( *it ).data << endl;
			break;
		case 0x0005:
			kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Got listening port: "
				<< ( *it ).length << " data: " << ( *it ).data << endl;
			break;
		case 0x000A:
			kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Got Acktype: " // 0x0001 normal message, 2 Abort Request, 3 Acknowledge request
				<< ( *it ).length << " data: " << ( *it ).data << endl;
			break;
		case 0x000B:
			kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Got unknown TLV 0x000B: "
				<< ( *it ).length << " data: " << ( *it ).data << endl;
			break;
		case 0x000F:
			kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Got unknown empty TLV 0x000F" << endl;
			break;
		case 0x2711:
		{
			Buffer tlv2711Buffer( ( *it ).data );

			int length1 =  tlv2711Buffer.getLEWord();
			if ( length1 != 0x001B )
			{	// all real messages (actually their header) have length 0x1B
				kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Weired Message length. Bailing out!" << endl;
				return;
			}
			
			int protocolVersion = tlv2711Buffer.getLEWord(); // dunno what to do with it...
			
			for ( int i = 0; i < 25; i++ )
			{	// 25 bytes of unneeded stuff
				tlv2711Buffer.getByte();
			}

			// the next one is length (of a counter + following all-zero field), but also indicates what type of message this is
			int length2 = tlv2711Buffer.getLEWord();

			// the only length usable ATM is 0x000E, which is a message
			switch( length2 )
			{
			case 0x000E:
			{
				int cookie = tlv2711Buffer.getLEWord();
				for ( int i = 0; i < 12; i++ )
				{	// 12 bytes all zeros
					tlv2711Buffer.getByte();
				}

				// now starts the real message
				msg.setType( tlv2711Buffer.getByte() );
				// TODO if type is PLAIN, there is an additional TLV with color and font information at the end...
				
				int flag = tlv2711Buffer.getByte();
				if ( flag == 0x03 ) // 0x03 = FLAG_AUTORESPONSE
				{
					msg.addProperty( Oscar::Message::AutoResponse );
				}
				else
				{
					msg.addProperty( Oscar::Message::Normal ); // copied from above, but: is this necessary?? Normal = 0
				}

				int status = tlv2711Buffer.getLEWord(); // don't know what status this is or what to use it for
				int priority = tlv2711Buffer.getLEWord(); // don't know what that's good for either
				QString messageText = tlv2711Buffer.getLELNTS();
				msg.setText( messageText );
				msg.setSender( m_fromUser );
				msg.setReceiver( client()->userId() );
				msg.setTimestamp( QDateTime::currentDateTime() );

				
				emit receivedMessage( msg );
			}
			default:
				kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Got unknown message with length2 " << length2 << endl;
			}


			break;
		} //end case
		default:
			kdDebug(OSCAR_RAW_DEBUG) << "Ignoring TLV of type " << ( *it ).type << endl;
			break;
		} //end switch
	}
}

void MessageReceiverTask::handleType4Message()
{
	TLV tlv5 = transfer()->buffer()->getTLV();
	kdDebug(14151) << k_funcinfo << "The first TLV is of type " << tlv5.type << endl;
	if (tlv5.type != 0x0005)
	{
		kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Aborting because first TLV != TLV(5)" << endl;
		return;
	}
	
	Buffer tlv5buffer(tlv5.data, tlv5.length);
	
	DWORD uin = tlv5buffer.getLEDWord(); // little endian for no sane reason!
	if ( QString::number(uin) != m_fromUser )
		kdWarning(14151) << k_funcinfo << "message uin does not match uin found in packet header!" << endl;

	BYTE msgType = tlv5buffer.getByte();
	BYTE msgFlags = tlv5buffer.getByte();
	
	kdDebug(14151) << k_funcinfo << "Received server message. type = " << msgType
		<< ", flags = " << msgFlags << endl;
		
	//handle the special user types
	Oscar::Message msg;
	QString msgSender;
	switch ( msgType )
	{
	case 0x0D:
		msgSender = "ICQ Web Express";
		msg.addProperty( Oscar::Message::WWP );
		break;
	case 0x0E:
		msgSender = "ICQ Email Express";
		msg.addProperty( Oscar::Message::EMail );
		break;
	default:
		msgSender = m_fromUser;
		break;
	};
	
	QByteArray msgText = tlv5buffer.getLNTS();
	int msgLength = msgText.size();
	if ( msgType == 0x0D || msgType == 0x0E )
	{
		for ( int i = 0; i < msgLength; i++ )
		{
			if ( msgText[i] == (char)0xFE )
				msgText[i] = 0x20;
		}
	}
	
	msg.addProperty( Oscar::Message::Latin );
	switch ( msgFlags )
	{
	case 0x03:
		msg.addProperty( Oscar::Message::AutoResponse );
		break;
	case 0x01:
		msg.addProperty( Oscar::Message::Normal );
		break;
	default:
		kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Not handling message flag " << msgFlags << endl;
		break;
	}
	
	msg.setType( 0x04 );
	msg.setTimestamp( QDateTime::currentDateTime() );
	msg.setSender( msgSender );
	msg.setReceiver( client()->userId() );
	msg.setText( QString(msgText) );
	emit receivedMessage( msg );
}

QTextCodec* MessageReceiverTask::guessCodec( const Q3CString& string )
{
	Q_UNUSED( string );
	return 0;
}

#include "messagereceivertask.moc"
//kate: indent-mode csands;
