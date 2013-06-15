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
#include "filetransfertask.h"

#include <qtextcodec.h>
#include <QList>
#include <QByteArray>
#include <QRegExp>
#include <kdebug.h>
#include "transfer.h"
#include "buffer.h"
#include "connection.h"
#include "oscarutils.h"
#include "userdetails.h"
#include "oscarmessageplugin.h"


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
		Oscar::WORD subtype = st->snacSubtype();
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
		kDebug(OSCAR_RAW_DEBUG) << "icbm cookie is " << m_icbmCookie.toHex();
		m_channel = b->getWord();
		kDebug(OSCAR_RAW_DEBUG) << "channel is " << m_channel;

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
				kWarning(OSCAR_RAW_DEBUG) << "A message was received on an unknown channel. Channel is " << m_channel;
				return false;
				break;
			}
		}
		else
		{
			m_fromUser = QString( b->getBUIN() );
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
	QList<TLV> messageTLVList = transfer()->buffer()->getTLVList();
	TLV t = Oscar::findTLV( messageTLVList, 0x0002 );
	if ( !t )
	{
		kWarning(OSCAR_RAW_DEBUG) << "Received a message packet with no message!";
		return;
	}
	Buffer messageBuffer( t.data );
	QList<TLV> innerTLVList = messageBuffer.getTLVList();
	QList<TLV>::iterator it = innerTLVList.begin(), listEnd = innerTLVList.end();
	for ( ; it != listEnd; ++it )
	{
		switch ( ( *it ).type )
		{
		case 0x0501:
			kDebug(OSCAR_RAW_DEBUG) << "Got features tlv. length: "
				<< ( *it ).length << " data: " << ( *it ).data << endl;
			break;
		case 0x0101:
		{
			Buffer message( ( *it ).data );
			m_charSet = message.getWord();
			m_subCharSet = message.getWord();
			kDebug(OSCAR_RAW_DEBUG) << "Message charset: " << m_charSet
				<< " message subcharset: " << m_subCharSet << endl;

			switch ( m_charSet )
			{
			case 0x0002:
				msg.setEncoding( Oscar::Message::UCS2 );
				break;
			case 0x0003:
			{
				msg.setEncoding( Oscar::Message::UserDefined );
				break;
			}
			default: // 0x0000 should be ASCII but some clients use different encoding.
				msg.setEncoding( Oscar::Message::UserDefined );
				break;
			}
			//message length is buffer length - length of ( charset + subcharset ) */
			int msgLength = ( *it ).length - 4;
			QByteArray msgArray( message.getBlock( msgLength ) );
			msg.setTextArray( msgArray );

			break;
		} //end case
		default:
			kDebug(OSCAR_RAW_DEBUG) << "Ignoring TLV of type " << ( *it ).type;
			break;
		} //end switch
	}

	TLV autoResponse = Oscar::findTLV( messageTLVList, 0x0004 );
	if ( autoResponse )
	{
		kDebug(OSCAR_RAW_DEBUG) << "auto response message";
		msg.addProperty( Oscar::Message::AutoResponse );
	}
	else
		msg.addProperty( Oscar::Message::Normal );

	TLV timestamp = Oscar::findTLV( messageTLVList, 0x0016 );
	if ( timestamp )
	{
		Buffer timestampBuffer( timestamp.data );
		msg.setTimestamp( QDateTime::fromTime_t( timestampBuffer.getDWord() ) );
	}
	else
		msg.setTimestamp( QDateTime::currentDateTime() );

	msg.setSender( m_fromUser );
	msg.setReceiver( client()->userId() );
	msg.setChannel( 0x01 );

	emit receivedMessage( msg );
}

void MessageReceiverTask::handleType2Message()
{
	kDebug(14151) << "Received Type 2 message. Trying to handle it...";

	Oscar::Message msg;
	msg.setSender( m_fromUser );
	msg.setReceiver( client()->userId() );
	QList<TLV> messageTLVList = transfer()->buffer()->getTLVList();
	TLV t = Oscar::findTLV( messageTLVList, 0x0005 );
	if ( !t )
	{
		kWarning(OSCAR_RAW_DEBUG) << "Received a channel 2 message packet with no message!";
		return;
	}
	Buffer messageBuffer( t.data );
	kDebug(14151) << "Buffer length is " << messageBuffer.length();

	// request type
	int requestType = messageBuffer.getWord();
	kDebug(14151) << "Request type (0 - request, 1 - cancel, 2 - accept): " << requestType;
	msg.setRequestType( requestType );

	// skip the message id cookie, already handled above
	messageBuffer.skipBytes( 8 );

	// next is capability identifier (GUID). this tells us what we're dealing with.
	Oscar::Guid g = messageBuffer.getGuid();
	if ( g == oscar_caps[CAP_SENDFILE] )
	{
		kDebug(14151) << "**************this is a filetransfer message************";
		emit fileMessage( requestType, m_fromUser, m_icbmCookie, messageBuffer);
		return;
	}

	if ( g == oscar_caps[CAP_CHAT] )
	{
		kDebug(14151) << "**************this is a chat message************";
		msg.setMessageType( Oscar::MessageType::Chat );
	}

	while( messageBuffer.bytesAvailable() > 0 )
	{
		TLV tlv = messageBuffer.getTLV();
		switch ( tlv.type )
		{
		case 0x0004:
			kDebug(OSCAR_RAW_DEBUG) << "Got external ip: "
				<< tlv.length << " data: " << tlv.data << endl;
			break;
		case 0x0005:
			kDebug(OSCAR_RAW_DEBUG) << "Got listening port: "
				<< tlv.length << " data: " << tlv.data << endl;
			break;
		case 0x000A:
			kDebug(OSCAR_RAW_DEBUG) << "Got request #: "
				<< tlv.length << " data: " << tlv.data << endl;
			break;
		case 0x000B:
			kDebug(OSCAR_RAW_DEBUG) << "Got unknown TLV 0x000B: "
				<< tlv.length << " data: " << tlv.data << endl;
			break;
		case 0x000C:
			kDebug(OSCAR_RAW_DEBUG) << "Got chat invitation message: "
				<< tlv.length << " data: " << tlv.data << endl;
			msg.setTextArray(tlv.data);
			break;
		case 0x000F:
			kDebug(OSCAR_RAW_DEBUG) << "Got unknown empty TLV 0x000F";
			break;
		case 0x2711:
		{
			kDebug(OSCAR_RAW_DEBUG) << "Got a TLV 2711";
			Buffer tlv2711Buffer( tlv.data );
			parseRendezvousData( &tlv2711Buffer, &msg );
			if(msg.messageType() == Oscar::MessageType::Chat)
				emit chatroomMessage( msg, m_icbmCookie );
			else
			{
				switch ( requestType )
				{
				case 0x00: // some request
					emit receivedMessage( msg );
					break;
				case 0x01:
					kDebug(OSCAR_RAW_DEBUG) << "Received Abort Message";
					break;
				case 0x02:
					kDebug(OSCAR_RAW_DEBUG) << "Received OK Message";
					break;
				default:
				kDebug(OSCAR_RAW_DEBUG) << "Received unknown request type: " << requestType;
					break;
				}
			}
			break;
		} //end case
		default:
			kDebug(OSCAR_RAW_DEBUG) << "Ignoring TLV of type " << tlv.type;
			break;
		} //end switch
	}//end while
}

void MessageReceiverTask::handleType4Message()
{
	TLV tlv5 = transfer()->buffer()->getTLV();
	kDebug(14151) << "The first TLV is of type " << tlv5.type;
	if (tlv5.type != 0x0005)
	{
		kDebug(OSCAR_RAW_DEBUG) << "Aborting because first TLV != TLV(5)";
		return;
	}

	Buffer tlv5buffer(tlv5.data, tlv5.length);

	Oscar::DWORD uin = tlv5buffer.getLEDWord(); // little endian for no sane reason!
	if ( QString::number(uin) != m_fromUser )
		kWarning(14151) << "message uin does not match uin found in packet header!";

	Oscar::BYTE msgType = tlv5buffer.getByte();
	Oscar::BYTE msgFlags = tlv5buffer.getByte();

	kDebug(14151) << "Received server message. type = " << msgType
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

	QByteArray msgText = tlv5buffer.getLELNTS();
	if ( msgType == 0x0D || msgType == 0x0E || msgType == 0x04 )
		msgText.replace( 0xFE, 0x20 );

	switch ( msgFlags )
	{
	case 0x03:
		msg.addProperty( Oscar::Message::AutoResponse );
		break;
	case 0x01:
		msg.addProperty( Oscar::Message::Normal );
		break;
	default:
		kDebug(OSCAR_RAW_DEBUG) << "Not handling message flag " << msgFlags;
		break;
	}

	msg.setChannel( 0x04 );
	msg.setTimestamp( QDateTime::currentDateTime() );
	msg.setSender( msgSender );
	msg.setReceiver( client()->userId() );
	msg.setEncoding( Oscar::Message::UserDefined );
	msg.setTextArray( msgText );
	emit receivedMessage( msg );
}

void MessageReceiverTask::handleAutoResponse()
{
	kDebug(14151) << "Received auto response. Trying to handle it...";

	Oscar::Message msg;
	msg.addProperty( Oscar::Message::AutoResponse );
	Buffer* b = transfer()->buffer();

	// reason code
	int reasonCode = b->getWord();
	kDebug(14151) << "Reason code (1 - channel not supported, 2 - busted payload, 3 - channel specific data): " << reasonCode;

	// TODO: remove this hack somehow
	// Check if it's for FileTransferTask ( FileTransfer auto response has diffrent structure )
	const QList<FileTransferTask*> p = parent()->findChildren<FileTransferTask*>();
	foreach( FileTransferTask *t, p)
	{
		if ( t->takeAutoResponse( reasonCode, m_icbmCookie, b ) )
			return;
	}
	
	parseRendezvousData( b, &msg );
	emit receivedMessage( msg );
}

void MessageReceiverTask::parseRendezvousData( Buffer* b, Oscar::Message* msg )
{
	// Do chat stuff
	if ( msg->messageType() == Oscar::MessageType::Chat )
	{
		// unknown, maybe it is always 00 04 23 21
		b->skipBytes( 4 );

		QString joinString( b->getBlock( b->length() - 4 ) );

		QRegExp rx( "aol://2719:(\\d+)-(\\d+)-(\\w+)" );
		if ( rx.exactMatch( joinString ) )
		{
			bool okay = true;
			Oscar::WORD num = rx.cap( 1 ).toUShort( &okay, 10 );
			Oscar::WORD exchange = rx.cap( 2 ).toUShort( &okay, 10 );
			QString chatroom = rx.cap( 3 );

			if ( num != 10 )
				kDebug() << "Warning: Expecting 10 but got " << num;

			msg->setExchange( exchange );
			msg->setChatRoom( chatroom );
			return;
		}

		kDebug() << "Error: Join string '" << joinString << "' did not match the regex.";

		return;
	}

	int length1 =  b->getLEWord();
	if ( length1 != 0x001B )
	{	// all real messages (actually their header) seem to have length 0x1B
		kDebug(OSCAR_RAW_DEBUG) << "Weired Message length. Bailing out!";
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

	// (down)counter: basically just some number, ICQ counts down, miranda up, doesn't matter.
	// BUT: when sending auto response on channel 2, like with the icbm cookie, we need to send the same value!
	int channel2Counter = b->getLEWord();

	// the next one is length (of a counter + following all-zero field), but also seems to indicate what type of message this is
	int length2 = b->getLEWord();

	// the only length usable ATM is 0x000E, which is a message
	switch( length2 )
	{
	case 0x000E:
	{
		b->skipBytes(14); //cookie is the first 4 bytes. next 12 is apparently all zeros

		// now starts the real message
		// TODO if type is PLAIN, there is (might be?) an additional TLV with color and font information at the end...

		uint messageType = b->getByte();

		/*
		int flags = b->getByte();
		int status = b->getLEWord(); 	// don't know what status this is or what to use it for
		int priority = b->getLEWord(); 	// don't know what that's good for either
		*/
		b->skipBytes(5); //same as above

		kDebug(OSCAR_RAW_DEBUG) << "Message type is: " << messageType;

		QByteArray msgText( b->getLEBlock() );
		if (!msgText.isEmpty() && msgText.at(msgText.length() - 1) == '\0')
			msgText.chop(1);
		
		Oscar::Message::Encoding encoding = Oscar::Message::UserDefined;

		if ( messageType == Oscar::MessageType::Plugin )
		{
			Oscar::MessagePlugin *plugin = new MessagePlugin();

			Buffer* headerBuffer = new Buffer( b->getLEBlock() );

			Oscar::Guid pluginGuid = headerBuffer->getGuid();
			plugin->setType( pluginGuid );
			plugin->setSubTypeId( headerBuffer->getLEWord() );
			plugin->setSubTypeText( headerBuffer->getLEDBlock() );

			// We don't parse unknown bytes
			// ICQ5 has 15 bytes
			// ICQ6 has 17 bytes
			delete headerBuffer;

			if ( b->bytesAvailable() >= 4 )
				plugin->setData( b->getLEDBlock() );

			msg->setPlugin( plugin );
		}
		else
		{
			int fgcolor = 0x00000000;
			int bgcolor = 0x00ffffff;

			if ( b->bytesAvailable() >= 8 )
			{
				fgcolor = b->getLEDWord();
				bgcolor = b->getLEDWord();

				while ( b->bytesAvailable() >= 4 )
				{
					int capLength = b->getLEDWord();
					if ( b->bytesAvailable() < capLength )
						break;

					QByteArray cap( b->getBlock( capLength ) );
					if ( qstrncmp ( cap.data(), "{0946134E-4C7F-11D1-8222-444553540000}", capLength ) == 0 )
						encoding = Oscar::Message::UTF8;
				}
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
		msg->setChannel( 0x02 );
		msg->setIcbmCookie( m_icbmCookie );
		msg->setProtocolVersion( protocolVersion );
		msg->setChannel2Counter( channel2Counter );
		msg->setMessageType( messageType );

		break;
	}
	default:
		kDebug(OSCAR_RAW_DEBUG) << "Got unknown message with length2 " << length2;
	}
}

QTextCodec* MessageReceiverTask::guessCodec( const QByteArray& string )
{
	Q_UNUSED( string );
	return 0;
}

#include "messagereceivertask.moc"
//kate: indent-mode csands;
