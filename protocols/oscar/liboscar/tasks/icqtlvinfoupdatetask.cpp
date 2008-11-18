/*
  Kopete Oscar Protocol
  icqtlvinfoupdatetask.cpp - SNAC 0x15 update user info (TLV based)

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

#include "icqtlvinfoupdatetask.h"

#include <kdebug.h>

#include "connection.h"
#include "transfer.h"
#include "buffer.h"

ICQTlvInfoUpdateTask::ICQTlvInfoUpdateTask( Task* parent ) : ICQTask( parent )
{
	m_goSequence = 0;
}

ICQTlvInfoUpdateTask::~ICQTlvInfoUpdateTask()
{
}

bool ICQTlvInfoUpdateTask::forMe( const Transfer* transfer ) const
{
	const SnacTransfer * st = dynamic_cast<const SnacTransfer*>( transfer );

	if ( !st )
		return false;

	if ( st->snacService() != 0x0015 || st->snacSubtype() != 0x0003 || st->snacRequest() != m_goSequence )
		return false;

	Buffer buf( *( st->buffer() ) );
	const_cast<ICQTlvInfoUpdateTask*>( this )->parseInitialData( buf );

	if ( requestType() == 0x07DA && requestSubType() == 0x0FDC )
		return true;

	return false;
}

bool ICQTlvInfoUpdateTask::take( Transfer* transfer )
{
	if ( forMe( transfer ) )
	{
		setTransfer( transfer );
		TLV tlv1 = transfer->buffer()->getTLV();
		Buffer buffer( tlv1.data, tlv1.length );

		//FIXME this is silly. parseInitialData should take care of this for me.
		buffer.skipBytes( 12 );

		if ( buffer.getByte() == 0x0A )
		{
			kDebug(OSCAR_RAW_DEBUG) << "User info was saved.";
			setSuccess( 0, QString() );
		}
		else
		{
			kDebug(OSCAR_RAW_DEBUG) << "Error saving user info!!!";
			setError( 0, QString() );
		}

		setTransfer( 0 );
		return true;
	}
	return false;
}

void ICQTlvInfoUpdateTask::onGo()
{
	kDebug(OSCAR_RAW_DEBUG) << "Updating user info.";

	setSequence( client()->snacSequence() );
	setRequestType( 0x07D0 );
	setRequestSubType( 0x0FD2 );

	Buffer b;

	b.startBlock( Buffer::BWord, Buffer::LittleEndian );
	// Magic numbers
	b.addDWord( 0x05b90003 );
	b.addDWord( 0x80000000 );
	b.addDWord( 0x00000006 );
	b.addDWord( 0x00010002 );
	b.addDWord( 0x00020000 );
	b.addDWord( 0x04e20000 );
	b.addDWord( 0x00020003 );

	m_info.store( &b );

	b.endBlock();

	m_goSequence = client()->snacSequence();

	Buffer *sendBuf = addInitialData( &b );
	FLAP f = { 0x02, 0, 0 };
	SNAC s = { 0x0015, 0x0002, 0, m_goSequence };
	Transfer* t = createTransfer( f, s, sendBuf );
	send( t );
}

void ICQTlvInfoUpdateTask::setInfo( const ICQFullInfo& info )
{
	m_info = info;
}
