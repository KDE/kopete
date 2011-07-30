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
#include "contactmanager.h"
#include "transfer.h"
#include <QList>

SSIListTask::SSIListTask( Task* parent ) : Task( parent )
{
	m_ssiManager = client()->ssiManager();
	QObject::connect( this, SIGNAL(newContact(OContact)), m_ssiManager, SLOT(newContact(OContact)) );
	QObject::connect( this, SIGNAL(newGroup(OContact)), m_ssiManager, SLOT(newGroup(OContact)) );
	QObject::connect( this, SIGNAL(newItem(OContact)), m_ssiManager, SLOT(newItem(OContact)) );
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
		SnacTransfer * st = static_cast<SnacTransfer*>( transfer );
		if ( st->snacSubtype() == 0x0006 )
		{
			setTransfer( transfer );
			handleContactListReply();
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
	checkContactTimestamp();
}

void SSIListTask::handleContactListReply()
{
	QList<TLV> tlvList;

	Buffer* buffer = transfer()->buffer();
	Oscar::BYTE protocolVersion = buffer->getByte();
	Oscar::WORD ssiItems = buffer->getWord();

	kDebug(OSCAR_RAW_DEBUG) << "SSI Protocol version: " << protocolVersion;
	kDebug(OSCAR_RAW_DEBUG) << "Number of items in this SSI packet: " << ssiItems;

	Oscar::WORD parsedItems;
	for ( parsedItems = 1; parsedItems <= ssiItems; ++parsedItems )
	{
		tlvList.clear();
		QString itemName = QString::fromUtf8( buffer->getBSTR() );
		Oscar::WORD groupId = buffer->getWord();
		Oscar::WORD itemId = buffer->getWord();
		Oscar::WORD itemType = buffer->getWord();
		Oscar::WORD tlvLength = buffer->getWord();
		for ( int i = 0; i < tlvLength; )
		{
			TLV t = buffer->getTLV();
			i += 4;
			i += t.length;
			tlvList.append( t );
		}
		
		if ( itemType == ROSTER_CONTACT )
			itemName = Oscar::normalize( itemName );
		
		OContact s( itemName, groupId, itemId, itemType, tlvList );
		
		kDebug(OSCAR_RAW_DEBUG) << "Got SSI Item: " << s.toString();
		if ( s.type() == ROSTER_GROUP )
			emit newGroup( s );
		
		if ( s.type() == ROSTER_CONTACT )
			emit newContact( s );
		
		if ( s.type() != ROSTER_CONTACT && s.type() != ROSTER_GROUP )
			emit newItem( s );
	}
	
	if ( buffer->bytesAvailable() > 0 )
	{
		client()->ssiManager()->setLastModificationTime( buffer->getDWord() );
		//check the snac flags for another packet
		SnacTransfer* st = dynamic_cast<SnacTransfer*>( transfer() );
		if ( st && st->snacFlags() == 0  )
		{
			kDebug(OSCAR_RAW_DEBUG) << "SSI List complete";
			client()->ssiManager()->setListComplete( true );
			setSuccess( 0, QString() );
		}
		else
			kDebug(OSCAR_RAW_DEBUG) << "Awaiting another SSI packet";
	}

}

void SSIListTask::handleSSIUpToDate()
{
	kDebug(OSCAR_RAW_DEBUG) << "Our SSI List is up to date";
	Buffer* buffer = transfer()->buffer();

	client()->ssiManager()->setLastModificationTime( buffer->getDWord() );
	Oscar::WORD ssiItems = buffer->getWord();
	kDebug(OSCAR_RAW_DEBUG) << "Number of items in SSI list: " << ssiItems;

	client()->ssiManager()->setListComplete( true );
	setSuccess( 0, QString() );
}

void SSIListTask::checkContactTimestamp()
{
	kDebug( OSCAR_RAW_DEBUG ) << "Checking the timestamp of the SSI list";
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
