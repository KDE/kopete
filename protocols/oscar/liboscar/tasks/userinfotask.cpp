/*
Kopete Oscar Protocol
userinfotask.h - Handle sending and receiving info requests for users

Copyright (c) 2004-2005 Matt Rogers <mattr@kde.org>

Kopete (c) 2002-2005 by the Kopete developers <kopete-devel@kde.org>

*************************************************************************
*                                                                       *
* This library is free software; you can redistribute it and/or         *
* modify it under the terms of the GNU Lesser General Public            *
* License as published by the Free Software Foundation; either          *
* version 2 of the License, or (at your option) any later version.      *
*                                                                       *
*************************************************************************
*/
#include "userinfotask.h"

#include <kdebug.h>

#include "buffer.h"
#include "connection.h"
#include "transfer.h"
#include "userdetails.h"


UserInfoTask::UserInfoTask( Task* parent )
: Task( parent )
{
}


UserInfoTask::~UserInfoTask()
{
}

bool UserInfoTask::forMe( const Transfer * transfer ) const
{
	const SnacTransfer* st = dynamic_cast<const SnacTransfer*>( transfer );
	if ( !st )
		return false;

	if ( st->snacService() == 0x0002 && st->snacSubtype() == 0x0006 )
	{
		if ( !m_contactSequenceMap.contains( st->snacRequest() ) )
			return false;
		else
		{
			//kDebug(OSCAR_RAW_DEBUG) << "Found sequence. taking packet";
			return true;
		}
	}
	else
		return false;
}

bool UserInfoTask::take( Transfer * transfer )
{
	if ( forMe( transfer ) )
	{
		setTransfer( transfer );
		Oscar::DWORD seq = 0;
		SnacTransfer* st = dynamic_cast<SnacTransfer*>( transfer );
		if ( st )
			seq = st->snacRequest();

		if ( seq != 0 )
		{
			//AFAIK location info packets always have user info
			Buffer* b = transfer->buffer();
			UserDetails ud;
			ud.fill( b );
			m_sequenceInfoMap[seq] = ud;
			emit gotInfo( seq );

			QList<TLV> list = b->getTLVList();
			QList<TLV>::iterator it = list.begin(), itEnd = list.end();
			QString profile;
			QString away;
			for ( ; it != itEnd; ++it )
			{
				switch( ( *it ).type )
				{
				case 0x0001: //profile text encoding
					kDebug(OSCAR_RAW_DEBUG) << "text encoding is " << QString( ( *it ).data );
					break;
				case 0x0002: //profile text
					kDebug(OSCAR_RAW_DEBUG) << "The profile is '" << QString( ( *it ).data ) << "'";
					profile = QString( ( *it ).data ); // aim always seems to use us-ascii encoding
					emit receivedProfile( m_contactSequenceMap[seq], profile );
					break;
				case 0x0003: //away message encoding
					kDebug(OSCAR_RAW_DEBUG) << "Away message encoding is " << QString( ( *it ).data );
					break;
				case 0x0004: //away message
					kDebug(OSCAR_RAW_DEBUG) << "Away message is '" << QString( ( *it ).data ) << "'";
					away = QString( (*it ).data ); // aim always seems to use us-ascii encoding
					emit receivedAwayMessage( m_contactSequenceMap[seq], away );
					break;
				case 0x0005: //capabilities
					break;
				default: //unknown
					kDebug(14151) << "Unknown user info type " << ( *it ).type;
					break;
				};
			}
			list.clear();
		}
		setTransfer( 0 );
		return true;
	}
	return false;
}

void UserInfoTask::onGo()
{
	if ( m_contactSequenceMap[m_seq].isEmpty() )
	{
		kDebug( OSCAR_RAW_DEBUG ) << "Info requested for empty contact!";
		return;
	}

	FLAP f = { 0x02, 0, 0 };
	SNAC s = { 0x0002, 0x0005, 0, m_seq };
	Buffer* buffer = new Buffer();

	buffer->addWord( m_typesSequenceMap[m_seq] );
	buffer->addBUIN( m_contactSequenceMap[m_seq].toLocal8Bit() );

	Transfer* t = createTransfer( f, s, buffer );
	send( t );
}

void UserInfoTask::requestInfoFor( const QString& contact, unsigned int types )
{
	Oscar::DWORD seq = client()->snacSequence();
	kDebug(OSCAR_RAW_DEBUG) << "setting sequence " << seq << " for contact " << contact;
	m_contactSequenceMap[seq] = contact;
	m_typesSequenceMap[seq] = types;
	m_seq = seq;
	onGo();
}

UserDetails UserInfoTask::getInfoFor( Oscar::DWORD sequence ) const
{
	return m_sequenceInfoMap[sequence];
}



//kate: indent-mode csands; tab-width 4;


#include "userinfotask.moc"
