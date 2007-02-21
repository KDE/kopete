/*
  Kopete Oscar Protocol
  ssilisttask.cpp - handles all operations dealing with the whole SSI list

  Copyright (c) 2004 Matt Rogers <mattr@kde.org>

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
#include "ssilisttask.h"

#include <kdebug.h>
#include "connection.h"
#include "oscarutils.h"
#include "ssimanager.h"
#include "transfer.h"

SSIListTask::SSIListTask( Task* parent ) : Task( parent )
{
	m_ssiManager = client()->ssiManager();
	QObject::connect( this, SIGNAL( newContact( const Oscar::SSI& ) ), m_ssiManager, SLOT( newContact( const Oscar::SSI& ) ) );
	QObject::connect( this, SIGNAL( newGroup( const Oscar::SSI& ) ), m_ssiManager, SLOT( newGroup( const Oscar::SSI& ) ) );
	QObject::connect( this, SIGNAL( newItem( const Oscar::SSI& ) ), m_ssiManager, SLOT( newItem( const Oscar::SSI& ) ) );
}


SSIListTask::~SSIListTask()
{}

bool SSIListTask::forMe( const Transfer* transfer ) const
{
	const SnacTransfer * st = dynamic_cast<const SnacTransfer*>( transfer );
	if ( !st )
		return false;

	if ( st->snacService() == 0x0013 )
	{
		switch ( st->snacSubtype() )
		{
		case 0x0006:
		case 0x000F:
			return true;
		default:
			return false;
		};
	}

	return false;
}

bool SSIListTask::take( Transfer* transfer )
{
	if ( forMe( transfer ) )
	{
		SnacTransfer * st = dynamic_cast<SnacTransfer*>( transfer );
		if ( st->snacSubtype() == 0x0006 )
		{
			setTransfer( transfer );
			handleSSIListReply();
			setTransfer( 0 );
			return true;
		}
		else if ( st->snacSubtype() == 0x000F )
		{
			setTransfer( transfer );
			handleSSIUpToDate();
			setTransfer( 0 );
			return true;
		}
	}

	return false;
}

void SSIListTask::onGo()
{
	checkSSITimestamp();
}

void SSIListTask::handleSSIListReply()
{
	QValueList<TLV> tlvList;

	Buffer* buffer = transfer()->buffer();
	kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "SSI Protocol version: " << buffer->getByte() << endl;
	WORD ssiItems = buffer->getWord();
	kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Number of items in this SSI packet: " << ssiItems << endl;
	WORD parsedItems;
	for ( parsedItems = 1; parsedItems <= ssiItems; ++parsedItems )
	{
		tlvList.clear();
		WORD strlength = buffer->getWord();
		QString itemName = QString::fromUtf8( buffer->getBlock( strlength ), strlength );
		WORD groupId = buffer->getWord();
		WORD itemId = buffer->getWord();
		WORD itemType = buffer->getWord();
		WORD tlvLength = buffer->getWord();
		for ( int i = 0; i < tlvLength; )
		{
			TLV t = buffer->getTLV();
			i += 4;
			i += t.length;
			tlvList.append( t );
		}
		
		if ( itemType == ROSTER_CONTACT )
			itemName = Oscar::normalize( itemName );
		
		Oscar::SSI s( itemName, groupId, itemId, itemType, tlvList );
		
		kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Got SSI Item: " << s.toString() << endl;
		if ( s.type() == ROSTER_GROUP )
			emit newGroup( s );
		
		if ( s.type() == ROSTER_CONTACT )
			emit newContact( s );
		
		if ( s.type() != ROSTER_CONTACT && s.type() != ROSTER_GROUP )
			emit newItem( s );
	}
	
	if ( buffer->length() > 0 )
	{
		client()->ssiManager()->setLastModificationTime( buffer->getDWord() );
		//check the snac flags for another packet
		SnacTransfer* st = dynamic_cast<SnacTransfer*>( transfer() );
		if ( st && st->snacFlags() == 0  )
		{
			kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "SSI List complete" << endl;
			client()->ssiManager()->setListComplete( true );
			setSuccess( 0, QString::null );
		}
		else
			kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Awaiting another SSI packet" << endl;
	}

}

void SSIListTask::handleSSIUpToDate()
{
	kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Our SSI List is up to date" << endl;
	Buffer* buffer = transfer()->buffer();

	client()->ssiManager()->setLastModificationTime( buffer->getDWord() );
	WORD ssiItems = buffer->getWord();
	kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Number of items in SSI list: " << ssiItems << endl;

	client()->ssiManager()->setListComplete( true );
	setSuccess( 0, QString::null );
}

void SSIListTask::checkSSITimestamp()
{
	kdDebug( OSCAR_RAW_DEBUG ) << k_funcinfo << "Checking the timestamp of the SSI list" << endl;
	FLAP f = { 0x02, 0, 0 };
	SNAC s = { 0x0013, 0x0005, 0x0000, client()->snacSequence() };
	Buffer* buffer = new Buffer();
	buffer->addDWord( client()->ssiManager()->lastModificationTime() );
	buffer->addDWord( client()->ssiManager()->numberOfItems() );
	Transfer* t = createTransfer( f, s, buffer );
	send( t );
}

#include "ssilisttask.moc"

// kate: tab-width 4; indent-mode csands;
