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
		setTransfer( transfer );
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
			handleType1Message();
			return true;
			break;
		case 0x0002:
			handleType2Message();
			return true;
			break;
		case 0x0004:
			handleType4Message();
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
	QValueList<TLV> messageTLVList = transfer()->buffer()->getTLVList();
	TLV t = Oscar::findTLV( messageTLVList, 0x0002 );
	if ( !t )
	{
		kdWarning(OSCAR_RAW_DEBUG) << k_funcinfo << "Received a message packet with no message!" << endl;
		return;
	}
	Buffer messageBuffer( t.data );
	QValueList<TLV> innerTLVList = messageBuffer.getTLVList();
	QValueList<TLV>::iterator it = innerTLVList.begin(), listEnd = innerTLVList.end();
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

QTextCodec* MessageReceiverTask::guessCodec( const QCString& string )
{
	Q_UNUSED( string );
	return 0;
}

#include "messagereceivertask.moc"
//kate: indent-mode csands;
