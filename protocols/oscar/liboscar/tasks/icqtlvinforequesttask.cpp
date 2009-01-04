/*
  Kopete Oscar Protocol
  icqtlvinforequesttask.cpp - SNAC 0x15 parsing for full user info (TLV based)

  Copyright (c) 2007 Roman Jarosz <kedgedev@centrum.cz>

  Kopete (c) 2007 by the Kopete developers <kopete-devel@kde.org>

  *************************************************************************
  *                                                                       *
  * This library is free software; you can redistribute it and/or         *
  * modify it under the terms of the GNU Lesser General Public            *
  * License as published by the Free Software Foundation; either          *
  * version 2 of the License, or (at your option) any later version.      *
  *                                                                       *
  *************************************************************************
*/

#include "icqtlvinforequesttask.h"

#include <kdebug.h>

#include "connection.h"
#include "transfer.h"
#include "buffer.h"

ICQTlvInfoRequestTask::ICQTlvInfoRequestTask( Task* parent ) : ICQTask( parent )
{
	m_type = Short;
}

ICQTlvInfoRequestTask::~ICQTlvInfoRequestTask()
{
}

bool ICQTlvInfoRequestTask::forMe( const Transfer* transfer ) const
{
	const SnacTransfer * st = dynamic_cast<const SnacTransfer*>( transfer );

	if ( !st )
		return false;

	if ( st->snacService() != 0x0015 || st->snacSubtype() != 0x0003 || !m_contactSequenceMap.contains( st->snacRequest() ) )
		return false;

	Buffer buf( *( st->buffer() ) );
	const_cast<ICQTlvInfoRequestTask*>( this )->parseInitialData( buf );

	if ( requestType() == 0x07DA && requestSubType() == 0x0FB4 )
		return true;

	return false;
}

bool ICQTlvInfoRequestTask::take( Transfer* transfer )
{
	if ( forMe( transfer ) )
	{
		const SnacTransfer* st = dynamic_cast<const SnacTransfer*>( transfer );
		if ( !st )
			return false;
		
		setTransfer( transfer );
		TLV tlv1 = transfer->buffer()->getTLV();
		Buffer buffer( tlv1.data, tlv1.length );

		//FIXME this is silly. parseInitialData should take care of this for me.
		buffer.skipBytes( 12 );

		if ( buffer.getByte() == 0x0A )
		{
			kDebug(OSCAR_RAW_DEBUG) << "Received user info";
			parse( st->snacRequest(), buffer.getLEBlock() );
			setSuccess( 0, QString() );
		}
		else
		{
			kDebug(OSCAR_RAW_DEBUG) << "Couldn't receive user info!!!";
			setError( 0, QString() );
		}

		setTransfer( 0 );
		return true;
	}
	return false;
}

void ICQTlvInfoRequestTask::onGo()
{
	kDebug(OSCAR_RAW_DEBUG) << "Requsting full TLV user info for: " << m_userToRequestFor;

	setSequence( client()->snacSequence() );
	setRequestType( 0x07D0 );
	setRequestSubType( 0x0FA0 );

	Buffer b;

	b.startBlock( Buffer::BWord, Buffer::LittleEndian );
	// Magic numbers
	b.addDWord( 0x05b90002 );
	b.addDWord( 0x80000000 );
	b.addDWord( 0x00000006 );
	b.addDWord( 0x00010002 );
	b.addDWord( 0x00020000 );
	b.addDWord( 0x04e20000 );
	b.addWord( 0x0002 );
	b.addWord( m_type );
	b.addDWord( 0x00000001 );

	b.startBlock( Buffer::BWord );
	b.addTLV( 0x003C, m_metaInfoId );
	b.addTLV( 0x0032, m_userToRequestFor.toLatin1() );
	b.endBlock();

	b.endBlock();

	Buffer *sendBuf = addInitialData( &b );

	Oscar::DWORD seq = client()->snacSequence();
	m_contactSequenceMap[seq] = m_userToRequestFor;

	FLAP f = { 0x02, 0, 0 };
	SNAC s = { 0x0015, 0x0002, 0, seq };
	Transfer* t = createTransfer( f, s, sendBuf );
	send( t );
}

ICQFullInfo ICQTlvInfoRequestTask::fullInfoFor( const QString& contact )
{
	ICQFullInfo info = m_fullInfoMap.value( contact );
	m_fullInfoMap.remove( contact );
	return info;
}

void ICQTlvInfoRequestTask::parse( Oscar::DWORD seq, const QByteArray &data )
{
	Buffer buf( data );

	buf.skipBytes( 45 );

	QString contact = m_contactSequenceMap[seq];

	ICQFullInfo info;
	info.setSequenceNumber( seq );
	info.fill( &buf );
	m_fullInfoMap[contact] = info;

	emit receivedInfoFor( contact );
	m_contactSequenceMap.remove( seq );
}

#include "icqtlvinforequesttask.moc"
