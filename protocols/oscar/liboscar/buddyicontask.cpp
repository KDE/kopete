// buddyicontask.cpp

// Copyright (C)  2005  Matt Rogers <mattr@kde.org>

// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.

// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
// 02111-1307  USA

#include "buddyicontask.h"

#include <qstring.h>
#include <kdebug.h>
#include "buffer.h"
#include "connection.h"
#include "transfer.h"
#include "oscarutils.h"

BuddyIconTask::BuddyIconTask( Task* parent )
	:Task( parent )
{

}

void BuddyIconTask::requestIconFor( const QString& user )
{
	m_user = user;
}

void BuddyIconTask::setHash( const QByteArray& md5Hash )
{
	m_hash = md5Hash;
}

void BuddyIconTask::onGo()
{
	if ( m_user.isEmpty() || m_hash.count() == 0 )
		return;

	if ( !client()->isIcq() )
		sendAIMBuddyIconRequest();
}

bool BuddyIconTask::forMe( const Transfer* transfer )
{
	const SnacTransfer* st = dynamic_cast<const SnacTransfer*>( transfer );
	if ( st )
		return false;

	if ( st->snacService() == 0x0010 )
	{
		QString user;
		switch( st->snacSubtype() )
		{
		case 0x0005:
		{
			user = const_cast<Buffer*>( st->buffer() )->peekBSTR();
			if ( Oscar::normalize( user ) != Oscar::normalize( m_user ) )
				return false;
			else
				return true;
		}
		break;
		case 0x0007:
		{
			user = const_cast<Buffer*>( st->buffer() )->peekBUIN();
			if ( Oscar::normalize( user ) != Oscar::normalize( m_user ) )
				return false;
			else
				return true;
		}
		break;
		default:
			return false;
			break;
		}
	}

	return false;
}

bool BuddyIconTask::take( Transfer* transfer )
{
	if ( !forMe( transfer ) )
		return false;

	SnacTransfer* st = dynamic_cast<SnacTransfer*>( transfer );
	if ( !st )
		return false;

	setTransfer( transfer );
	if ( st->snacSubtype() == 0x0005 )
		handleAIMBuddyIconResponse();
// else
// 		handleICQBuddyIconResponse();

	return true;
}

void BuddyIconTask::sendAIMBuddyIconRequest()
{
	FLAP f = { 0x02, 0, client()->flapSequence() };
	SNAC s = { 0x0010, 0x0004, 0x0000, client()->snacSequence() };
	Buffer* b = new Buffer;

	b->addBSTR( m_user.latin1() ); //TODO check encoding
	b->addByte( 0x01 );
	b->addWord( 0x0001 );
	b->addByte( 0x01 );
	b->addByte( 0x10 ); //MD5 Hash Size
	b->addString( m_hash, 0x10 ); //MD5 Hash
	Transfer* t = createTransfer( f, s, b );
	send( t );
	delete( t );
}

void BuddyIconTask::handleAIMBuddyIconResponse()
{
	Buffer* b = transfer()->buffer();
}


