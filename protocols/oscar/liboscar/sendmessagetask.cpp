/*
   sendmessagetask.h  - Outgoing OSCAR Messaging Handler

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

#include "sendmessagetask.h"

#include <qtextcodec.h>
#include <kapplication.h>
#include <kdebug.h>
#include <krandom.h>
#include "connection.h"
#include "oscartypes.h"
#include "transfer.h"


SendMessageTask::SendMessageTask(Task* parent): Task(parent)
{
	m_autoResponse = false;
}


SendMessageTask::~SendMessageTask()
{
}

void SendMessageTask::setMessage( const Oscar::Message& msg )
{
	m_message = msg;
}

void SendMessageTask::setAutoResponse( bool autoResponse )
{
	m_autoResponse = autoResponse;
}

void SendMessageTask::onGo()
{
	if ( m_message.text().isEmpty() && m_message.type() == 1 ) // at least channel 2 needs to send empty messages
	{
		setError(-1, "No message to send");
		return;
	}

	const uint CHUNK_LENGTH = 450;
	uint msgPostion = 0;

	do
	{
		// Check Message to see what SNAC to use
		int snacSubfamily = 0x0006;
		if ( ( m_message.type() == 2 ) && m_message.hasProperty( Oscar::Message::AutoResponse ) )
		{ // an auto response is send for ack of channel 2 messages
			kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Sending SNAC 0x0B instead of 0x06 " << endl;
			snacSubfamily = 0x000B;
		}
		FLAP f = { 0x02, 0, 0 };
		SNAC s = { 0x0004, snacSubfamily, 0x0000, client()->snacSequence() };
		Buffer* b = new Buffer();

		if ( snacSubfamily == 0x0006 )
		{
			DWORD cookie1 = KRandom::random();
			DWORD cookie2 = KRandom::random();
			
			b->addDWord( cookie1 );
			b->addDWord( cookie2 );
		}
		else
		{
			b->addString( m_message.icbmCookie() ); // in automated response, we need the same cookie as in the request
		}
		
		b->addWord( m_message.type() );
		
		b->addByte( m_message.receiver().length() );
		b->addString( m_message.receiver().toLatin1(), m_message.receiver().length() );
		
		QString msgChunk = m_message.text().mid( msgPostion, CHUNK_LENGTH );
		// Try to split on space if needed
		if ( msgChunk.length() == CHUNK_LENGTH )
		{
			for ( int i = 0; i < 100; i++ )
			{
				if ( msgChunk[CHUNK_LENGTH - i].isSpace() )
				{
					msgChunk = msgChunk.left(CHUNK_LENGTH - i);
					msgPostion++;
					break;
				}
			}
		}
		msgPostion += msgChunk.length();


		if ( snacSubfamily == 0x0006 )
		{
			/* send a regular message */
			switch ( m_message.type() )
			{
			case 1:
				addChannel1Data( b, msgChunk );
				break;
			case 2:
				addChannel2Data( b, msgChunk );
				break;
			case 4:
				addChannel4Data( b, msgChunk );
				break;
			}

			// Add the TLV to indicate if this is an autoresponse: 0x00040000
			// Right now, only supported for the AIM client, I'm not sure about ICQ
			// For some reason you can't have both a 0x0004 and 0x0003 TLV in the same
			// SNAC, if you do the AIM server complains
			if ( !client()->isIcq() && (m_autoResponse == true) )
			{
				TLV tlv4( 0x0004, 0, NULL);
				b->addTLV( tlv4 );
			}
			else
			{
				b->addDWord( 0x00030000 ); //empty TLV 3 to get an ack from the server
			}
			
			if ( client()->isIcq() && ( ! m_message.hasProperty( Oscar::Message::StatusMessageRequest ) ) )
				b->addDWord( 0x00060000 ); //empty TLV 6 to store message on the server if not online
		}
		else
		{
			/* send an autoresponse */
			b->addWord( 0x0003 ); // reason code: 1: channel not supported; 2: busted payload; 3: channel specific;
			//TODO: i hardcoded it for now, since we don't suppoert error messages atm anyway
			addRendezvousMessageData( b, msgChunk );
		}
		

	
		Transfer* t = createTransfer( f, s, b );
		kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "SENDING: " << t->toString() << endl;
		send( t );
		
	} while ( msgPostion < m_message.text().length() );
	
	setSuccess(true);
}


void SendMessageTask::addBasicTLVs( Buffer* b )
{
	//TODO add stuff like user class, user status, online time, etc TLVS
}


void SendMessageTask::addChannel1Data( Buffer* b, const QString& message )
{
	Buffer tlv2buffer;

	//Send features TLV using data from gaim. Features are different
	//depending on whether we're ICQ or AIM
	if ( client()->isIcq() )
	{
		tlv2buffer.addDWord( 0x05010002 ); //TLV 0x0501, length 2
		tlv2buffer.addWord( 0x0106 ); //TLV 0x0501 data
	}
	else
	{
		tlv2buffer.addDWord( 0x05010004 ); //TLV 0x0501, length 4
		tlv2buffer.addDWord( 0x01010102 ); //TLV 0x0501 data.
	}
	//we only send one message part. There's only one client that actually uses
	//them and it's quite old and infrequently used
	tlv2buffer.addWord( 0x0101 ); //add TLV(0x0101) also known as TLV(257)
	/* If we can encode in Latin1, do that, otherwise send Unicode */
	QTextCodec* codec = QTextCodec::codecForMib( 4 ); //4 is the MIBEnum for ISO-8859-1
	if ( codec->canEncode( message ) )
	{
		kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Latin-1 encoding successful. Sending outgoing message as "
			<< "ISO-8859-1" << endl;
		tlv2buffer.addWord( message.length() + 4 ); // add TLV length
		tlv2buffer.addWord( 0x0000 );
		tlv2buffer.addWord( 0x0000 );
		tlv2buffer.addString( message.toLatin1(), message.length() );
	}
	else
	{
		kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Latin-1 encoding not successful. Sending outgoing message as "
			<< "UCS-2" << endl;
		int length = message.length() * 2;
		unsigned char* utfMessage = new unsigned char[length];
		for ( unsigned int l = 0; l < message.length(); l++ )
		{
			utfMessage[l * 2] = message.unicode()[l].row();
			utfMessage[( l * 2 ) + 1] = message.unicode()[l].cell();
		}

		tlv2buffer.addWord( length + 4 ); // add TLV length
		tlv2buffer.addWord( 0x0002 );
		tlv2buffer.addWord( 0x0000 );
		tlv2buffer.addString( utfMessage, length );

		delete [] utfMessage;
	}

	TLV tlv2( 0x0002, tlv2buffer.length(), tlv2buffer.buffer() );
	b->addTLV( tlv2 );
}

void SendMessageTask::addChannel2Data( Buffer* b, const QString& message )
{
	kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Trying to send type 2 message!" << endl;

	Buffer tlv5buffer;
	
	tlv5buffer.addWord( 0 ); // 0 = request; other possibilities: 1 = cancel; 2 = accept;
	//TODO: i hardcoded it for now, don't yet what to use the other stuff for

	// message id cookie. needs to be the same one as above, thus copy first eight bytes of buffer
	Buffer* tmp = new Buffer(b->buffer(), 8);
	tlv5buffer.addString( tmp->buffer(), 8 );
	delete tmp;
	
	/* send our client capability. oscardocs say this one means we support type 2 messages,
	   ethereal say it means we support server relay. however, it's what most clients send,
	   even official ones...
	*/

	// too lazy to think about byte order :)
	tlv5buffer.addByte( 0x09 );
	tlv5buffer.addByte( 0x46 );
	tlv5buffer.addByte( 0x13 );
	tlv5buffer.addByte( 0x49 );
	tlv5buffer.addByte( 0x4C );
	tlv5buffer.addByte( 0x7F );
	tlv5buffer.addByte( 0x11 );
	tlv5buffer.addByte( 0xD1 );
	tlv5buffer.addByte( 0x82 );
	tlv5buffer.addByte( 0x22 );
	tlv5buffer.addByte( 0x44 );
	tlv5buffer.addByte( 0x45 );
	tlv5buffer.addByte( 0x53 );
	tlv5buffer.addByte( 0x54 );
	tlv5buffer.addByte( 0x00 );
	tlv5buffer.addByte( 0x00 );
	
	// These are optional, would probably be a god ide to start using them, though

	// add TLV 03: internal ip
// 	tlv5buffer.addWord( 0x0003 ); // TLV Type
// 	tlv5buffer.addWord( 0x0004 ); // TLV Length
// 	tlv5buffer.addDWord( 0x00000000 ); // TLV Data: Internal IP

	// add TLV 05: listening port
// 	tlv5buffer.addWord( 0x0005 ); // TLV Type
// 	tlv5buffer.addWord( 0x0002 ); // TLV Length
// 	tlv5buffer.addWord( 0x0000 ); // TLV Data: listening port

	// add TLV 0A: unknown
	tlv5buffer.addWord( 0x000A ); // TLV Type
	tlv5buffer.addWord( 0x0002 ); // TLV Length
	tlv5buffer.addWord( 0x0001 ); // TLV Data: unknown, usually 1

	// add TLV 0B: unknown
// 	tlv5buffer.addWord( 0x000B ); // TLV Type
// 	tlv5buffer.addWord( 0x0002 ); // TLV Length
// 	tlv5buffer.addWord( 0x0000 ); // TLV Data: unknown

	// add TLV 0F: unknown
	tlv5buffer.addWord( 0x000F ); // TLV Type
	tlv5buffer.addWord( 0x0000 ); // TLV Length
	// TLV Data: empty
	
		
	
	/* now comes the important TLV 0x271 */
	
	int tlv2711DataLength = 2+2+16+2+4+1+2 + 2+2+4+4+4 + 2+4+2+message.length()+1 /*+ 4+4+4+16+1*/;
	
	tlv5buffer.addWord( 0x2711 ); // start extended data TLV
	tlv5buffer.addWord( tlv2711DataLength ); // the calculated length
	//TODO: might change when changing message encoding to utf?
	
	addRendezvousMessageData( &tlv5buffer, message );
	TLV tlv5( 0x0005, tlv5buffer.length(), tlv5buffer.buffer() );
	b->addTLV( tlv5 );
}

void SendMessageTask::addChannel4Data( Buffer* b, const QString& message )
{
	//TODO
}

void SendMessageTask::addRendezvousMessageData( Buffer* b, const QString& message )
{
	// first data segment
	b->addLEWord( 0x001B ); // length of this data segment, always 27

	// protocol version
	// miranda,licq use 8, gaim,icq5 use 9, icq2003b uses 10.
	// 9 seems to make things a litle difficult, 10 seems a little more like 8, but still more difficult
	b->addLEWord( 0x0008 ); // so stick with 8 for now :)
	
	for ( int i = 0; i < 16; i++)
	{
		b->addByte( 0x00 ); // pluginID or all zeros (see oscar docs)
	}
	
	b->addWord( 0x0000 ); // unknown
	b->addLEDWord( 0x00000003 ); // FIXME client capabilities: not sure, but should be ICQ Server Relay
	b->addByte( 0x0000 ); // unknown

	// channel 2 counter: in auto response, use original message value. s/t else otherwise (most anythig will work)
	int channel2Counter = 0xBEEF; // just some number for now
	if ( m_message.hasProperty( Oscar::Message::AutoResponse ) )
		channel2Counter = m_message.channel2Counter();
	
	b->addLEWord( channel2Counter ); // channel 2 counter
	
	// second data segment
	b->addLEWord( 0x000E ); // length of this data segment, always 14
	b->addLEWord( channel2Counter ); // channel 2 counter

	for ( int i = 0; i < 12; i++)
	{
		b->addByte( 0x00 ); // unknown, usually all zeros
	}
	
	// actual message data segment

	// Message type
	if ( m_message.messageType() == 0x00 )
		b->addByte( 0x01 );
	else
		b->addByte( m_message.messageType() );
	
	int messageFlags = 0x01; // Normal
	if ( m_message.hasProperty( Oscar::Message::StatusMessageRequest ) )
		messageFlags = 0x03; // Auto message. required for both requesting and sending status messages
	else if ( m_message.hasProperty( Oscar::Message::AutoResponse ) )
		messageFlags = 0x00; // A regular type 2 msg ack requires 0x00 here...
	b->addByte( messageFlags );
	
	// status code, priority:
	// common (ICQ) practice seems to be: both 1 when requesting away message, both 0 otherwise
	// miranda sends 256/0 in away message request. it works, but i don't see the point...
	// other then that, don't yet really know what they are for.
	if ( m_message.hasProperty( Oscar::Message::StatusMessageRequest ) && ( ! m_message.hasProperty( Oscar::Message::AutoResponse ) ) )
	{
		b->addLEWord( 0x0001 ); // status (?)
		b->addLEWord( 0x0001 ); // priority (?)
	}
	else
	{
		b->addLEWord( 0x0000 ); // status (?)
		b->addLEWord( 0x0000 ); // priority (?)
	}
	

	//! UTF in away messages doesnt work. using latin1 for now
	// need to append message itself now. i am not sure about how encoding is handled, so i use utf for now
// 	int length = msgChunk.length() * 2;
// 	unsigned char* utfMessage = new unsigned char[length];
// 	for ( unsigned int l = 0; l < msgChunk.length(); l++ )
// 	{
// 		utfMessage[l*2] = msgChunk.unicode()[l].row();
// 		utfMessage[(l*2)+1] = msgChunk.unicode()[l].cell();
// 	}
// 	b->addLEWord( length + 1 ); // length of string + zero termination
// 	b->addString( utfMessage, length ); // string itself
	
	b->addLEWord( message.length() + 1 ); // length of string + zero termination
	b->addString( message.toLatin1(), message.length() ); // string itself
	b->addByte( 0x00 ); // zero termination
}



/* Old oscarsocket code, which is here for reference in case this doesn't work
QTextCodec *codec = 0L;
WORD charset = 0x0000; // default to ascii
WORD charsubset = 0x0000;
int length = message.length();
unsigned char *utfMessage = 0L;

codec=QTextCodec::codecForMib(3); // US-ASCII

if(codec)
{
	if(codec->canEncode(message)) // this returns true for some accented western european chars but kopete can't decode on receipt
	{
		//kdDebug(14151) << k_funcinfo << "Going to encode as US-ASCII" << endl;
		// We are forcing kopete to send messages using ISO-8859-1
		// It's a hack and should be reimplemented in a better way
		charset=0x0003;
		codec=QTextCodec::codecForMib(4);
		//kdDebug(14151) << k_funcinfo << "Now trying ISO-8859-1" << endl;
	}
	else
	{
		codec=0L; // we failed encoding it as US-ASCII
		//kdDebug(14151) << k_funcinfo << "Cannot encode as US-ASCII" << endl;
	}
}

// if we couldn't encode it as ascii, and either the client says it can do UTF8, or we have no
// contact specific encoding set, might as well send it as UTF-16BE as as ISO-8859-1
if ( !codec && ( contact->hasCap(CAP_UTF8) || !contact->encoding() ) )
{
	// use UTF is peer supports it and encoding as US-ASCII failed
	length=message.length()*2;
	utfMessage=new unsigned char[length];
	for(unsigned int l=0; l<message.length(); l++)
	{
		utfMessage[l*2]=message.unicode()[l].row();
		utfMessage[(l*2)+1]=message.unicode()[l].cell();
	}
	charset=0x0002; // send UTF-16BE
}

// no codec and no charset and per-contact encoding set
if(!codec && charset != 0x0002 && contact->encoding() != 0)
{
	codec=QTextCodec::codecForMib(contact->encoding());
	if(codec)
		charset=0x0003; //send as ISO-8859-1
}

if(!codec && charset != 0x0002) // it's neither unicode nor did we find a codec so far!
{
	kdDebug(14151) << k_funcinfo <<
		"Couldn't find suitable encoding for outgoing message, " <<
		"encoding using ISO-8859-1, prepare for receiver getting unreadable text :)" << endl;
	charset=0x0003;
	codec=QTextCodec::codecForMib(4); // ISO-8859-1
}

tlv2.addWord(0x0101); //add TLV(0x0101) also known as TLV(257)
tlv2.addWord(length + 0x04); // add TLV length
tlv2.addWord(charset); // normal char set
tlv2.addWord(charsubset); // normal char set

if(utfMessage)
{
	kdDebug(14151) << k_funcinfo << "Outgoing message encoded as 'UTF-16BE'" << endl;
	tlv2.addString(utfMessage, length); // the actual message
	delete [] utfMessage;
}
else
{
	kdDebug(14151) << k_funcinfo << "Outgoing message encoded as '" << codec->name() << "'" << endl;
	QCString outgoingMessage=codec->fromUnicode(message);
	tlv2.addString(outgoingMessage, length); // the actual message
}
// ====================================================================================

outbuf.addTLV(0x0002, tlv2.length(), tlv2.buffer());

if(isAuto) // No clue about this stuff, probably AIM-specific [mETz]
{
	outbuf.addWord(0x0004);
	outbuf.addWord(0x0000);
}

if(mIsICQ)
{
	//outbuf.addWord(0x0003); // TLV.Type(0x03) - request an ack from server
	//outbuf.addWord(0x0000);

	outbuf.addWord(0x0006); // TLV.Type(0x06) - store message if recipient offline
	outbuf.addWord(0x0000);
}

sendBuf(outbuf,0x02);
*/




//kate: tab-width 4; indent-mode csands;
