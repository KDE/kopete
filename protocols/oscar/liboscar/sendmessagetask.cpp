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
	if ( m_message.text().isEmpty() )
		return;
	
	//FIXME Right now we only send channel 1. We also need to
	//support channel 4 and 2
	FLAP f = { 0x02, client()->flapSequence(), 0 };
	SNAC s = { 0x0004, 0x0006, 0x0000, client()->snacSequence() };
	Buffer* b = new Buffer();
	
	DWORD cookie1 = KApplication::random();
	DWORD cookie2 = KApplication::random();
	
	b->addDWord( cookie1 );
	b->addDWord( cookie2 );
	
	b->addWord( 0x0001 ); //channel one only right now
	
	b->addByte( m_message.receiver().length() );
	b->addString( m_message.receiver().latin1(), m_message.receiver().length() );
	
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
	if ( codec->canEncode( m_message.text() ) )
	{
		kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Latin-1 encoding successful. Sending outgoing message as "
			<< "ISO-8859-1" << endl;
		tlv2buffer.addWord( m_message.text().length() + 4 ); // add TLV length
		tlv2buffer.addWord( 0x0000 );
		tlv2buffer.addWord( 0x0000 );
		tlv2buffer.addString( m_message.text().latin1(), m_message.text().length() );
	}
	else
	{
		kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Latin-1 encoding not successful. Sending outgoing message as "
			<< "UCS-2" << endl;
		
		int length = m_message.text().length() * 2;
		unsigned char* utfMessage = new unsigned char[length];
		for ( unsigned int l = 0; l < m_message.text().length(); l++ )
		{
			utfMessage[l * 2] = m_message.text().unicode()[l].row();
			utfMessage[( l * 2 ) + 1] = m_message.text().unicode()[l].cell();
		}
		
		tlv2buffer.addWord( length + 4 ); // add TLV length
		tlv2buffer.addWord( 0x0002 );
		tlv2buffer.addWord( 0x0000 );
		tlv2buffer.addString( utfMessage, length );
		
	}
	
	// Add the actual message TLV
	TLV tlv2( 0x0002, tlv2buffer.length(), tlv2buffer.buffer() );
	b->addTLV( tlv2 );

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
	
	if ( client()->isIcq() )
		b->addDWord( 0x00060000 ); //empty TLV 6 to store message on the server if not online

	Transfer* t = createTransfer( f, s, b );
	kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "SENDING: " << t->toString() << endl;
	send( t );
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
