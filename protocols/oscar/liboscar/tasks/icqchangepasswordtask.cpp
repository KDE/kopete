/*
  Kopete Oscar Protocol
  icqchangepasswordtask.cpp - SNAC 0x15 change password

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

#include "icqchangepasswordtask.h"

#include <kdebug.h>

#include "connection.h"
#include "transfer.h"
#include "buffer.h"

ICQChangePasswordTask::ICQChangePasswordTask( Task* parent ) : ICQTask( parent )
{
	m_goSequence = 0;
}

ICQChangePasswordTask::~ICQChangePasswordTask()
{
}

bool ICQChangePasswordTask::forMe( const Transfer* transfer ) const
{
	const SnacTransfer * st = dynamic_cast<const SnacTransfer*>( transfer );

	if ( !st )
		return false;

	if ( st->snacService() != 0x0015 || st->snacSubtype() != 0x0003 )
		return false;

	Buffer buf( *( st->buffer() ) );
	const_cast<ICQChangePasswordTask*>( this )->parseInitialData( buf );

	if ( requestType() == 0x07DA && requestSubType() == 0x00AA
	     && m_goSequence == sequence() )
		return true;

	return false;
}

bool ICQChangePasswordTask::take( Transfer* transfer )
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
			kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Password changed successfully." << endl;
			setSuccess( 0, QString() );
		}
		else
		{
			kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Error changing password!!!" << endl;
			setError( 0, QString() );
		}

		setTransfer( 0 );
		return true;
	}
	return false;
}

void ICQChangePasswordTask::onGo()
{
	kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Changing password." << endl;

	if ( m_password.length() < 6 || m_password.length() > 8 )
	{
		kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Wrong password length." << endl;
		setError( 0, QString() );
		return;
	}

	m_goSequence = client()->snacSequence();
	setSequence( m_goSequence );
	setRequestType( 0x07D0 );
	setRequestSubType( 0x042E );

	Buffer b;
	b.addLELNTS( m_password.toLatin1() );

	Buffer *sendBuf = addInitialData( &b );
	FLAP f = { 0x02, 0, 0 };
	SNAC s = { 0x0015, 0x0002, 0, client()->snacSequence() };
	Transfer* t = createTransfer( f, s, sendBuf );
	send( t );
}

void ICQChangePasswordTask::setPassword( const QString& password )
{
	m_password = password;
}

QString ICQChangePasswordTask::password() const
{
	return m_password;
}
