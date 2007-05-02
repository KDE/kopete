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

	if ( st->snacService() == 0x0004 )
	{
		WORD subtype = st->snacSubtype();
		switch ( subtype )
		{
		case 0x0007:
		case 0x000B:
			return true;
			break;
		default:
			return false;
			break;
		}
	}
	else
		return false;
}

bool MessageReceiverTask::take( Transfer* transfer )
{
	if ( forMe( transfer ) )
	{
		const SnacTransfer* st = dynamic_cast<const SnacTransfer*>( transfer );
		if ( !st )
			return false;
		m_currentSnacSubtype = st->snacSubtype();
		
		Buffer* b = transfer->buffer();
		m_icbmCookie = b->getBlock( 8 );
		kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "icbm cookie is " << m_icbmCookie << endl;
		m_channel = b->getWord();
		kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "channel is " << m_channel << endl;

		if ( m_currentSnacSubtype == 0x0007 )
		{
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
				kdWarning(OSCAR_RAW_DEBUG) << "A message was received on an unknown channel. Channel is " << m_channel << endl;
				return false;
				break;
			}
		}
		else
		{
			int screenNameLength = b->getByte();
			m_fromUser = QString( b->getBlock( screenNameLength ) );
			setTransfer( transfer );
			handleAutoResponse();
			setTransfer( 0 );
			return true;
		}
	}
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
			if ( m_charSet == 0x0002 )
				msg.setEncoding( Oscar::Message::UCS2 );
			else
				msg.setEncoding( Oscar::Message::UserDefined );

			//message length is buffer length - length of ( charset + subcharset ) */
			int msgLength = ( *it ).length - 4;
			QByteArray msgArray( message.getBlock( msgLength ) );
			msg.setTextArray( msgArray );

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
	kdDebug(14151) << k_funcinfo << "Received Type 2 message. Trying to handle it..." << endl;

	Oscar::Message msg;
	QValueList<TLV> messageTLVList = transfer()->buffer()->getTLVList();
	TLV t = Oscar::findTLV( messageTLVList, 0x0005 );
	if ( !t )
	{
		kdWarning(OSCAR_RAW_DEBUG) << k_funcinfo << "Received a channel 2 message packet with no message!" << endl;
		return;
	}
	Buffer messageBuffer( t.data );
	kdDebug(14151) << k_funcinfo << "Buffer length is " << messageBuffer.length() << endl;

	// request type
	int requestType = messageBuffer.getWord();
	kdDebug(14151) << k_funcinfo << "Request type (0 - request, 1 - cancel, 2 - accept): " << requestType << endl;

	// skip the message id cookie, already handled above
	messageBuffer.skipBytes( 8 );

	// next is capability identifier (GUID). skip for now
	messageBuffer.skipBytes( 16 );

	while( messageBuffer.length() > 0 )
	{
		TLV tlv = messageBuffer.getTLV();
		switch ( tlv.type )
		{
		case 0x0004:
			kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Got external ip: "
				<< tlv.length << " data: " << tlv.data << endl;
			break;
		case 0x0005:
			kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Got listening port: "
				<< tlv.length << " data: " << tlv.data << endl;
			break;
		case 0x000A:
			kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Got Acktype: " // 0x0001 normal message, 2 Abort Request, 3 Acknowledge request
				<< tlv.length << " data: " << tlv.data << endl;
			break;
		case 0x000B:
			kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Got unknown TLV 0x000B: "
				<< tlv.length << " data: " << tlv.data << endl;
			break;
		case 0x000F:
			kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Got unknown empty TLV 0x000F" << endl;
			break;
		case 0x2711:
		{
			kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Got a TLV 2711" << endl;
			Buffer tlv2711Buffer( tlv.data );
			parseRendezvousData( &tlv2711Buffer, &msg );
			if ( msg.messageType() == 0x1A )
			{
				kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Received plugin message" << endl;
				break;
			}

			switch ( requestType )
			{
			case 0x00: // some request
				emit receivedMessage( msg );
				break;
			case 0x01:
				kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Received Abort Mesage" << endl;
				break;
			case 0x02:
				kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Received OK Message" << endl;
				break;
			default:
			kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Received unknown request type: " << requestType << endl;
				break;
			}
			
			break;
		} //end case
		default:
			kdDebug(OSCAR_RAW_DEBUG) << "Ignoring TLV of type " << tlv.type << endl;
			break;
		} //end switch
	}//end while
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

	QCString msgText = tlv5buffer.getLNTS();
	int msgLength = msgText.size();
	if ( msgType == 0x0D || msgType == 0x0E )
	{
		for ( int i = 0; i < msgLength; i++ )
		{
			if ( msgText[i] == (char)0xFE )
				msgText[i] = 0x20;
		}
	}

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
	msg.setEncoding( Oscar::Message::UserDefined );
	msg.setTextArray( msgText );
	emit receivedMessage( msg );
}

void MessageReceiverTask::handleAutoResponse()
{
	kdDebug(14151) << k_funcinfo << "Received auto response. Trying to handle it..." << endl;
	
	Oscar::Message msg;
	msg.addProperty( Oscar::Message::AutoResponse );
	Buffer* b = transfer()->buffer();

	// reason code
	int reasonCode = b->getWord();
	kdDebug(14151) << k_funcinfo << "Reason code (1 - channel not supported, 2 - busted payload, 3 - channel specific data): " << reasonCode << endl;
	
	parseRendezvousData( b, &msg );
	emit receivedMessage( msg );
}

void MessageReceiverTask::parseRendezvousData( Buffer* b, Oscar::Message* msg )
{
	int length1 =  b->getLEWord();
	if ( length1 != 0x001B )
	{	// all real messages (actually their header) seem to have length 0x1B
		kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Weired Message length. Bailing out!" << endl;
		return;
	}
	
	int protocolVersion = b->getLEWord(); // the extended data protocol version, there are quite a few...
	
	// plugin (for file transfer & stuff, all zeros for regular message
	b->skipBytes( 16 );
	// unknown
	b->skipBytes( 2 );
	// client capablities
	b->skipBytes( 4 );
	// unknown
	b->skipBytes( 1 );
	
	// (down)counter: basically just some number, ICQ counts down, miranda up, doesnt matter.
	// BUT: when sending auto response on channel 2, like with the icbm cookie, we need to send the same value!
	int channel2Counter = b->getLEWord();
	
	// the next one is length (of a counter + following all-zero field), but also seems to indicate what type of message this is
	int length2 = b->getLEWord();
	
	// the only length usable ATM is 0x000E, which is a message
	switch( length2 )
	{
	case 0x000E:
	{
		int cookie = b->getLEWord();
		for ( int i = 0; i < 12; i++ )
		{	// 12 bytes all zeros
			b->getByte();
		}

		// now starts the real message
		// TODO if type is PLAIN, there is (might be?) an additional TLV with color and font information at the end...

		uint messageType = b->getByte();
		int flags = b->getByte();
		int status = b->getLEWord(); 	// don't know what status this is or what to use it for
		int priority = b->getLEWord(); 	// don't know what that's good for either

		kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Message type is: " << messageType << endl;
		
		QCString msgText( b->getLELNTS() );
		Oscar::Message::Encoding encoding = Oscar::Message::UserDefined;
		int fgcolor = 0x00000000;
		int bgcolor = 0x00ffffff;

		// Don't parse plugin message
		if ( b->length() >= 8 && messageType != 0x1A )
		{
			fgcolor = b->getLEDWord();
			bgcolor = b->getLEDWord();

			while ( b->length() >= 4 )
			{
				int capLength = b->getLEDWord();
				if ( b->length() < capLength )
					break;

				QByteArray cap( b->getBlock( capLength ) );
				if ( qstrncmp ( cap.data(), "{0946134E-4C7F-11D1-8222-444553540000}", capLength ) == 0 )
					encoding = Oscar::Message::UTF8;
			}
		}

		msg->setEncoding( encoding );
		msg->setTextArray( msgText );
		
		if ( ( messageType & 0xF0 ) == 0xE0 ) // check higher byte for value E -> status message request
			msg->addProperty( Oscar::Message::StatusMessageRequest );
		else
			msg->addProperty( Oscar::Message::Request );
		
		msg->setSender( m_fromUser );
		msg->setReceiver( client()->userId() );
		msg->setTimestamp( QDateTime::currentDateTime() );
		msg->setType( 0x02 );
		msg->setIcbmCookie( m_icbmCookie );
		msg->setProtocolVersion( protocolVersion );
		msg->setChannel2Counter( channel2Counter );
		msg->setMessageType( messageType );
		
		break;
	}
	default:
		kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Got unknown message with length2 " << length2 << endl;
	}
}

QTextCodec* MessageReceiverTask::guessCodec( const QCString& string )
{
	Q_UNUSED( string );
	return 0;
}

#include "messagereceivertask.moc"
//kate: indent-mode csands;
