/*
   Kopete Oscar Protocol
   icqtask.cpp - SNAC 0x15 parsing 

   Copyright (c) 2004 Gustavo Pichorim Boiko <gustavo.boiko@kdemail.net>

   Kopete (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>

   *************************************************************************
   *                                                                       *
   * This library is free software; you can redistribute it and/or         *
   * modify it under the terms of the GNU Lesser General Public            *
   * License as published by the Free Software Foundation; either          *
   * version 2 of the License, or (at your option) any later version.      *
   *                                                                       *
   *************************************************************************
*/

#include "icqtask.h"
#include "buffer.h"
#include "connection.h"

#include <qstring.h>

#include <kdebug.h>

ICQTask::ICQTask( Task * parent )
	: Task( parent )
{
	m_icquin = client()->userId().toULong();
	m_sequence = 0;
	m_requestType = 0xFFFF;
	m_requestSubType = 0xFFFF;
}

ICQTask::~ ICQTask()
{
}

void ICQTask::onGo()
{
}

bool ICQTask::forMe( const Transfer *t ) const
{
	Q_UNUSED( t );
	return false;
}

bool ICQTask::take( Transfer* t )
{
	Q_UNUSED( t );
	return false;
}

void ICQTask::parseInitialData( Buffer buf )
{
	int tlvLength = 0;

	TLV tlv1 = buf.getTLV();
	Buffer tlv1Buffer(tlv1.data, tlv1.length);
	tlvLength = tlv1Buffer.getLEWord(); //FIXME handle the data chunk size
	m_icquin = tlv1Buffer.getLEDWord(); //nice ICQ UIN
	m_requestType = tlv1Buffer.getLEWord(); //request type
	m_sequence = tlv1Buffer.getLEWord();
	if ( m_requestType == 0x07DA ) //there's an extra data subtype
		m_requestSubType = tlv1Buffer.getLEWord();
	else
		m_requestSubType = 0xFFFF;

/*kDebug(OSCAR_RAW_DEBUG) << "uin: " << m_icquin << " sequence: " << sequence
		<<" request type: 0x" << QString::number( m_requestType, 16 )
		<< " request sub type: 0x" << QString::number( m_requestSubType, 16 ) << endl;*/
}

Buffer* ICQTask::addInitialData( Buffer* buf ) const
{
	if ( m_requestType == 0xFFFF )
	{ //something very wrong here
		return 0;
	}
	
	Buffer* tlvData = new Buffer();
	tlvData->addLEDWord( m_icquin ); // UIN
	tlvData->addLEWord( m_requestType ); // request type
	tlvData->addLEWord( m_sequence );
	
	if ( m_requestSubType != 0xFFFF )
		tlvData->addLEWord( m_requestSubType );
	
	/*kDebug(OSCAR_RAW_DEBUG) << "uin: " << m_icquin << " sequence: " << sequence
		<<" request type: 0x" << QString::number( m_requestType, 16 )
		<< " request sub type: 0x" << QString::number( m_requestSubType, 16 ) << endl; */
	if ( buf != 0 )
		tlvData->addString( buf->buffer() );
	
	Buffer* newBuffer = new Buffer();
	//add TLV 1
	newBuffer->addWord( 0x0001 ); //TLV 1
	newBuffer->addWord( tlvData->length() + 2 ); //TLV length
	newBuffer->addLEWord( tlvData->length() ); // data chunk size
	newBuffer->addString( tlvData->buffer() );
	
	delete tlvData;

	return newBuffer;
}

Oscar::DWORD ICQTask::uin() const
{
	return m_icquin;
}

void ICQTask::setUin( Oscar::DWORD uin )
{
	m_icquin = uin;
}

Oscar::WORD ICQTask::sequence() const
{
	return m_sequence;
}

void ICQTask::setSequence( Oscar::WORD sequence )
{
	m_sequence = sequence;
}

Oscar::DWORD ICQTask::requestType() const
{
	return m_requestType;
}

void ICQTask::setRequestType( Oscar::WORD type )
{
	m_requestType = type;
}

Oscar::DWORD ICQTask::requestSubType() const
{
	return m_requestSubType;
}

void ICQTask::setRequestSubType( Oscar::WORD subType )
{
	m_requestSubType = subType;
}

#include "icqtask.moc"

//kate: space-indent off; tab-width 4; replace-tabs off; indent-mode csands;
