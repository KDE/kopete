/*
  Kopete Oscar Protocol
  icquserinfoupdatetask.cpp - SNAC 0x15 update user info

  Copyright (c) 2006 Roman Jarosz <kedgedev@centrum.cz>

  Kopete (c) 2006 by the Kopete developers <kopete-devel@kde.org>

  *************************************************************************
  *                                                                       *
  * This library is free software; you can redistribute it and/or         *
  * modify it under the terms of the GNU Lesser General Public            *
  * License as published by the Free Software Foundation; either          *
  * version 2 of the License, or (at your option) any later version.      *
  *                                                                       *
  *************************************************************************
*/

#include "icquserinfoupdatetask.h"

#include <kdebug.h>

#include "connection.h"
#include "transfer.h"
#include "buffer.h"

ICQUserInfoUpdateTask::ICQUserInfoUpdateTask( Task* parent ) : ICQTask( parent )
{
	m_goSequence = 0;
}

ICQUserInfoUpdateTask::~ICQUserInfoUpdateTask()
{
	qDeleteAll( m_infoList );
}

bool ICQUserInfoUpdateTask::forMe( const Transfer* transfer ) const
{
	const SnacTransfer * st = dynamic_cast<const SnacTransfer*>( transfer );

	if ( !st )
		return false;

	if ( st->snacService() != 0x0015 || st->snacSubtype() != 0x0003 || st->snacRequest() != m_goSequence )
		return false;

	Buffer buf( *( st->buffer() ) );
	const_cast<ICQUserInfoUpdateTask*>( this )->parseInitialData( buf );

	if ( requestType() == 0x07DA && requestSubType() == 0x0C3F )
		return true;

	return false;
}

bool ICQUserInfoUpdateTask::take( Transfer* transfer )
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
			kDebug(OSCAR_RAW_DEBUG) << "Own user info was saved.";
			setSuccess( 0, QString() );
		}
		else
		{
			kDebug(OSCAR_RAW_DEBUG) << "Error saving own user info!!!";
			setError( 0, QString() );
		}

		setTransfer( 0 );
		return true;
	}
	return false;
}

void ICQUserInfoUpdateTask::onGo()
{
	kDebug(OSCAR_RAW_DEBUG) << "Saving own user info.";

	setSequence( client()->snacSequence() );
	setRequestType( 0x07D0 );
	setRequestSubType( 0x0C3A );

	Buffer b;
	for ( int i = 0; i < m_infoList.size(); ++i )
		m_infoList.at(i)->store( &b );

	if ( b.length() == 0 )
	{
		setSuccess( 0, QString() );
		return;
	}

	m_goSequence = client()->snacSequence();

	Buffer *sendBuf = addInitialData( &b );
	FLAP f = { 0x02, 0, 0 };
	SNAC s = { 0x0015, 0x0002, 0, m_goSequence };
	Transfer* t = createTransfer( f, s, sendBuf );
	send( t );
}

void ICQUserInfoUpdateTask::setInfo( const QList<ICQInfoBase*>& infoList )
{
	m_infoList = infoList;
}

