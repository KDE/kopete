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

#include <kapplication.h>
#include <kdebug.h>
#include <krandom.h>
#include "connection.h"
#include "oscartypes.h"
#include "transfer.h"


SendMessageTask::SendMessageTask(Task* parent): Task(parent)
{
	m_autoResponse = false;
	m_cookieCount = 0x7FFF;
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
	if ( m_message.textArray().isEmpty() && m_message.channel() == 1 )
	{ //channel 1 shouldn't be sending blanks
		setError(-1, "No message to send");
		return;
	}

	// Check Message to see what SNAC to use
	int snacSubfamily = 0x0006;
	if ( ( m_message.messageType() == 2 ) && m_message.hasProperty( Oscar::Message::AutoResponse ) )
	{ // an auto response is send for ack of chat messages
		kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Sending SNAC 0x0B instead of 0x06 " << endl;
		snacSubfamily = 0x000B;
	}
	FLAP f = { 0x02, 0, 0 };
	SNAC s = { 0x0004, snacSubfamily, 0x0000, client()->snacSequence() };
	Buffer* b = new Buffer();

	if ( snacSubfamily == 0x0006 && m_message.messageType() != 3 )
	{
		DWORD cookie1 = KRandom::random();
		DWORD cookie2 = KRandom::random();
		
		b->addDWord( cookie1 );
		b->addDWord( cookie2 );

		m_message.setIcbmCookie( b->buffer() ); //in case we need it later
	}
	else
	{ //file msgs and automated responses already have a cookie
		b->addString( m_message.icbmCookie() );
	}

	b->addWord( m_message.channel() );

	b->addByte( m_message.receiver().length() );
	b->addString( m_message.receiver().toLatin1(), m_message.receiver().length() );


	if ( snacSubfamily == 0x0006 )
	{
		/* send a regular message */
		switch ( m_message.channel() )
		{
		case 1: //plaintext
			addChannel1Data( b );
			break;
		case 2: //chatreq or filereq
			addChannel2Data( b );
			break;
		case 4: //url
			addChannel4Data( b );
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

		if ( client()->isIcq() && m_message.channel() != 2 && ! m_message.hasProperty( Oscar::Message::StatusMessageRequest ) )
			b->addDWord( 0x00060000 ); //empty TLV 6 to store message on the server if not online
	}
	else
	{
		/* send an autoresponse */
		b->addWord( 0x0003 ); // reason code: 1: channel not supported; 2: busted payload; 3: channel specific;
		//TODO: i hardcoded it for now, since we don't suppoert error messages atm anyway
		addRendezvousMessageData( b );
	}

	Transfer* t = createTransfer( f, s, b );
	kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "SENDING: " << t->toString() << endl;
	send( t );

	setSuccess(true);
}


void SendMessageTask::addBasicTLVs( Buffer* b )
{
	Q_UNUSED( b );
	//TODO add stuff like user class, user status, online time, etc TLVS
}


void SendMessageTask::addChannel1Data( Buffer* b )
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
	tlv2buffer.addWord( m_message.textArray().size() + 4 ); // add TLV length

	if ( m_message.encoding() == Oscar::Message::UserDefined )
	{
		kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Sending outgoing message in "
			<< "per-contact encoding" << endl;
		tlv2buffer.addWord( 0x0000 );
		tlv2buffer.addWord( 0x0000 );
	}
	else
	{
		kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Sending outgoing message as "
			<< "UCS-2" << endl;
		tlv2buffer.addWord( 0x0002 );
		tlv2buffer.addWord( 0x0000 );
	}
	tlv2buffer.addString( m_message.textArray() );

	b->addTLV( 0x0002, tlv2buffer.length(), tlv2buffer.buffer() );
}

void SendMessageTask::addChannel2Data( Buffer* b )
{
	kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Trying to send channel 2 message!" << endl;

	Buffer tlv5buffer;
	
	tlv5buffer.addWord( m_message.reqType() ); // 0 = request; 1 = cancel; 2 = accept

	// message id cookie. needs to be the same one as above
	tlv5buffer.addString( m_message.icbmCookie() );
	
	//cap used to identify the message type
	tlv5buffer.addGuid( oscar_caps[ m_message.messageType() == 3 ? CAP_SENDFILE : CAP_ICQSERVERRELAY ] );

	if( m_message.reqType() == 0 )
	{ //requests need more data

		// add TLV 0A: request # TODO: might be >1 if not direct
		tlv5buffer.addWord( 0x000A ); // TLV Type
		tlv5buffer.addWord( 0x0002 ); // TLV Length
		tlv5buffer.addWord( 0x0001 ); // TLV Data

		// add TLV 0F: unknown but always there
		tlv5buffer.addWord( 0x000F );
		tlv5buffer.addWord( 0x0000 );

		//ft needs at least internal-ip, port, and port check
		//might need proxy-ip later.
		if ( int p = m_message.port() )
		{
			//hardcoding my own ip is bad, bad, BAD!
			char v2[] = { 70, 68, 183, 66 };
			tlv5buffer.addTLV( 0x3, 4, v2 ); //our ip FIXME
			//our port
			tlv5buffer.addWord( 5 );
			tlv5buffer.addWord( 2 );
			tlv5buffer.addWord( p ); //FIXME: check endianness
			//port check
			tlv5buffer.addWord( 0x17 );
			tlv5buffer.addWord( 2 );
			tlv5buffer.addWord( ~ p ); //FIXME: check endianness

		}


		/* now comes the important TLV 0x2711 */
		Buffer tlv2711;
		if ( m_message.messageType() == 3 ) //TODO: reduce the amount of magic #s
		{ //filetransfer
			tlv2711.addWord( 1 ); //multiple file flag (we only support 1 right now)
			tlv2711.addWord( 1 ); //file count
			tlv2711.addDWord( m_message.fileSize() ); //FIXME: verify that it's nonzero
			tlv2711.addString( m_message.fileName().toLatin1() );
			tlv2711.addByte( 0 ); //make sure the name's null-terminated
		}
		else //chat
			addRendezvousMessageData( &tlv2711 );
		tlv5buffer.addTLV( 0x2711, tlv2711.length(), tlv2711.buffer() );
	}

	b->addTLV( 0x0005, tlv5buffer.length(), tlv5buffer.buffer() );
}

void SendMessageTask::addChannel4Data( Buffer* b )
{
	Q_UNUSED( b );
	//Q_UNUSED( message );
	//TODO
}

void SendMessageTask::addRendezvousMessageData( Buffer* b )
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
	int channel2Counter;
	if ( m_message.hasProperty( Oscar::Message::AutoResponse ) )
		channel2Counter = m_message.channel2Counter();
	else
		channel2Counter = (m_cookieCount--) & 0x7FFF;
	
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
	
	int messageFlags = 0x00; // Normal
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
	

	b->addLEWord( m_message.textArray().size() + 1 ); // length of string + zero termination
	b->addString( m_message.textArray() ); // string itself
	b->addByte( 0x00 ); // zero termination
	b->addLEDWord( 0x00000000 ); // foreground
	b->addLEDWord( 0x00FFFFFF ); // foreground

	if ( m_message.encoding() == Oscar::Message::UTF8 )
	{
		b->addLEDWord( 38 );
		b->addString( "{0946134E-4C7F-11D1-8222-444553540000}", 38 );
	}
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
		//kDebug(14151) << k_funcinfo << "Going to encode as US-ASCII" << endl;
		// We are forcing kopete to send messages using ISO-8859-1
		// It's a hack and should be reimplemented in a better way
		charset=0x0003;
		codec=QTextCodec::codecForMib(4);
		//kDebug(14151) << k_funcinfo << "Now trying ISO-8859-1" << endl;
	}
	else
	{
		codec=0L; // we failed encoding it as US-ASCII
		//kDebug(14151) << k_funcinfo << "Cannot encode as US-ASCII" << endl;
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
	kDebug(14151) << k_funcinfo <<
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
	kDebug(14151) << k_funcinfo << "Outgoing message encoded as 'UTF-16BE'" << endl;
	tlv2.addString(utfMessage, length); // the actual message
	delete [] utfMessage;
}
else
{
	kDebug(14151) << k_funcinfo << "Outgoing message encoded as '" << codec->name() << "'" << endl;
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
