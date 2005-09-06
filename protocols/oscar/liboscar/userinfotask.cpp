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
		if ( m_contactSequenceMap.find( st->snacRequest() ) == m_contactSequenceMap.end() )
			return false;
		else
		{
			//kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Found sequence. taking packet" << endl;
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
		Q_UINT16 seq = 0;
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
			
			QValueList<TLV> list = b->getTLVList();
			QValueList<TLV>::iterator it = list.begin();
			QString profile;
			QString away;
			for ( ; ( *it ); ++it )
			{
				switch( ( *it ).type )
				{
				case 0x0001: //profile text encoding
					kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "text encoding is " << QString( ( *it ).data )<< endl;
					break;
				case 0x0002: //profile text
					kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "The profile is '" << QString( ( *it ).data ) << "'" << endl;
					profile = QString( ( *it ).data ); // aim always seems to use us-ascii encoding
					emit receivedProfile( m_contactSequenceMap[seq], profile );
					break;
				case 0x0003: //away message encoding
					kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Away message encoding is " << QString( ( *it ).data ) << endl;
					break;
				case 0x0004: //away message
					kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Away message is '" << QString( ( *it ).data ) << "'" << endl;
					away = QString( (*it ).data ); // aim always seems to use us-ascii encoding
					emit receivedAwayMessage( m_contactSequenceMap[seq], away );
					break;
				case 0x0005: //capabilities
					break;
				default: //unknown
					kdDebug(14151) << k_funcinfo << "Unknown user info type " << ( *it ).type << endl;
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
		kdDebug( OSCAR_RAW_DEBUG ) << k_funcinfo << "Info requested for empty contact!" << endl;
		return;
	}
	
	FLAP f = { 0x02, 0, 0 };
	SNAC s = { 0x0002, 0x0005, 0, m_seq };
	Buffer* buffer = new Buffer();
	
	buffer->addWord( m_typesSequenceMap[m_seq] );
	buffer->addBUIN( m_contactSequenceMap[m_seq].local8Bit() );
	
	Transfer* t = createTransfer( f, s, buffer );
	send( t );
}

void UserInfoTask::requestInfoFor( const QString& contact, unsigned int types )
{
	Q_UINT16 seq = client()->snacSequence();
	kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "setting sequence " << seq << " for contact " << contact << endl;
	m_contactSequenceMap[seq] = contact;
	m_typesSequenceMap[seq] = types;
	m_seq = seq;
	onGo();
}

UserDetails UserInfoTask::getInfoFor( Q_UINT16 sequence ) const
{
	return m_sequenceInfoMap[sequence];
}



//kate: indent-mode csands; tab-width 4;


#include "userinfotask.moc"
