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
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
// 02110-1301  USA

#include "buddyicontask.h"

#include <qstring.h>
#include <kdebug.h>
#include "buffer.h"
#include "connection.h"
#include "transfer.h"
#include "oscarutils.h"
#include <typeinfo>

BuddyIconTask::BuddyIconTask( Task* parent )
	:Task( parent )
{
	m_seq = 0;
	m_refNum = -1;
	m_iconLength = 0;
	m_iconType = 0x0001;
	m_hashType = 0;
}

void BuddyIconTask::uploadIcon( Oscar::WORD length, const QByteArray& data )
{
	m_iconLength = length;
	m_icon = data;
	m_action = Send;
}

void BuddyIconTask::requestIconFor( const QString& user )
{
	m_user = user;
	m_action = Receive;
}

void BuddyIconTask::setHash( const QByteArray& md5Hash )
{
	m_hash = md5Hash;
}

void BuddyIconTask::setIconType( Oscar::WORD iconType )
{
	m_iconType = iconType;
}

void BuddyIconTask::setHashType( Oscar::BYTE type )
{
	m_hashType = type;
}

void BuddyIconTask::onGo()
{
	if ( m_action == Send && m_icon.count() == 0 )
		return;

	if ( m_action == Receive && ( m_user.isEmpty() || m_hash.count() == 0 ) )
		return;

	if ( m_action == Receive )
	{
		if ( client()->isIcq() )
			sendICQBuddyIconRequest();
		else
			sendAIMBuddyIconRequest();
	}
	else
		sendIcon();
}

bool BuddyIconTask::forMe( const Transfer* transfer ) const
{
	const SnacTransfer* st = dynamic_cast<const SnacTransfer*>( transfer );
	if ( !st )
		return false;

	if ( st->snacRequest() != m_seq )
	{
		kDebug(OSCAR_RAW_DEBUG) << "sequences don't match";
		return false;
	}

	if ( st->snacService() == 0x0010 )
	{
		switch( st->snacSubtype() )
		{
		case 0x0003:
		case 0x0005:
		case 0x0007:
			return true;
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
	if ( st->snacSubtype() == 0x0003 )
		handleUploadResponse();
	else if ( st->snacSubtype() == 0x0005 )
		handleAIMBuddyIconResponse();
	else
		handleICQBuddyIconResponse();

	setSuccess( 0, QString() );
	setTransfer( 0 );
	return true;
}

void BuddyIconTask::sendIcon()
{
	kDebug(OSCAR_RAW_DEBUG) << "icon length: " << m_iconLength;
	FLAP f = { 0x02, 0, 0 };
	m_seq = client()->snacSequence();
	SNAC s = { 0x0010, 0x0002, 0x0000, m_seq };
	Buffer* b = new Buffer;
	b->addWord( m_iconType ); //gaim hard codes it, so will we
	b->addWord( m_iconLength );
	b->addString( m_icon );
	Transfer* t = createTransfer( f, s, b );
	send( t );
}

void BuddyIconTask::handleUploadResponse()
{
	kDebug(OSCAR_RAW_DEBUG) << "server acked icon upload";
	Buffer* b = transfer()->buffer();
	b->skipBytes( 4 );
	Oscar::BYTE iconHashSize = b->getByte();
	QByteArray hash( b->getBlock( iconHashSize ) );
	//check the hash
	kDebug(OSCAR_RAW_DEBUG) << "hash " << hash.toHex();
	setSuccess( 0, QString() );
}


void BuddyIconTask::sendAIMBuddyIconRequest()
{
	kDebug(OSCAR_RAW_DEBUG) << "requesting buddy icon for " << m_user;
	FLAP f = { 0x02, 0, 0 };
	m_seq = client()->snacSequence();
	SNAC s = { 0x0010, 0x0004, 0x0000, m_seq };
	Buffer* b = new Buffer;

	b->addBUIN( m_user.toLatin1() ); //TODO: check encoding
	b->addByte( 0x01 );
	b->addWord( m_iconType );
	b->addByte( m_hashType );
	b->addByte( m_hash.size() ); //MD5 Hash Size
	b->addString( m_hash ); //MD5 Hash
	Transfer* t = createTransfer( f, s, b );
	send( t );
}

void BuddyIconTask::handleAIMBuddyIconResponse()
{
	Buffer* b = transfer()->buffer();
	QString user = b->getBUIN();
	kDebug(OSCAR_RAW_DEBUG) << "Receiving buddy icon for " << user;
	b->skipBytes(2); //unknown field. not used
	Oscar::BYTE iconType = b->getByte();
	Q_UNUSED( iconType );
	Oscar::BYTE hashSize = b->getByte();
	QByteArray iconHash( b->getBlock(hashSize) );
	Oscar::WORD iconSize = b->getWord();
	QByteArray icon( b->getBlock(iconSize) );
	emit haveIcon( user, icon );
	setSuccess( 0, QString() );
}

void BuddyIconTask::sendICQBuddyIconRequest()
{
	kDebug(OSCAR_RAW_DEBUG) << "requesting buddy icon for " << m_user;
	FLAP f = { 0x02, 0, 0 };
	m_seq = client()->snacSequence();
	SNAC s = { 0x0010, 0x0006, 0x0000, m_seq };
	Buffer* b = new Buffer;

	b->addBUIN( m_user.toLatin1() ); //TODO: check encoding
	b->addByte( 0x01 );
	b->addWord( m_iconType );
	b->addByte( m_hashType );
	b->addByte( m_hash.size() ); //MD5 Hash Size
	b->addString( m_hash ); //MD5 Hash
	Transfer* t = createTransfer( f, s, b );
	send( t );
}

void BuddyIconTask::handleICQBuddyIconResponse()
{
	Buffer* b = transfer()->buffer();
	QString user = b->getBUIN();
	kDebug(OSCAR_RAW_DEBUG) << "Receiving buddy icon for " << user;
	
	b->skipBytes(2); //not used
	Oscar::BYTE iconType = b->getByte();
	Q_UNUSED( iconType );
	
	Oscar::BYTE hashSize = b->getByte();
	QByteArray iconHash( b->getBlock(hashSize) );
	
	b->skipBytes(1); //not used
	b->skipBytes(2); //not used
	Oscar::BYTE iconType2 = b->getByte();
	Q_UNUSED( iconType2 );
	
	Oscar::BYTE hashSize2 = b->getByte();
	QByteArray iconHash2( b->getBlock(hashSize2) );
	
	Oscar::WORD iconSize = b->getWord();
	QByteArray icon( b->getBlock(iconSize) );
	
	emit haveIcon( user, icon );
	setSuccess( 0, QString() );
}

#include "buddyicontask.moc"


