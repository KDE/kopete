/*
   Kopete Oscar Protocol
   usersearchtask.cpp - Search for contacts

   Copyright (c) 2004 Gustavo Pichorim Boiko <gustavo.boiko@kdemail.net>

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

#include "usersearchtask.h"

#include "transfer.h"
#include "buffer.h"
#include "connection.h"

UserSearchTask::UserSearchTask( Task* parent )
 : ICQTask( parent )
{
}


UserSearchTask::~UserSearchTask()
{
}

void UserSearchTask::onGo()
{
}

bool UserSearchTask::forMe( const Transfer* t ) const
{
	const SnacTransfer* st = dynamic_cast<const SnacTransfer*>( t );
	
	if ( !st )
		return false;
	
	if ( st->snacService() != 0x0015 || st->snacSubtype() != 0x0003 )
		return false;
		
	Buffer buf( st->buffer()->buffer(), st->buffer()->length() );
	const_cast<UserSearchTask*>(this)->parseInitialData( buf );
	
	if ( requestType() == 0x07da && ( requestSubType() == 0x01a4 || requestSubType() == 0x01ae ) )
		return true;

	return false;
}

bool UserSearchTask::take( Transfer* t )
{
	if ( forMe( t ) )
	{
		setTransfer( t );
		
		Q_UINT16 seq = 0;
		SnacTransfer* st = dynamic_cast<SnacTransfer*>( t );
		if ( st )
			seq = st->snacRequest();
		
		TLV tlv1 = transfer()->buffer()->getTLV();
		Buffer* buffer = new Buffer( tlv1.data, tlv1.length );
		
		if ( seq == 0 )
			return false;
			
		ICQSearchResult result;
		buffer->getLEWord(); // data chunk size
		DWORD receiverUin = buffer->getLEDWord(); // target uin
		buffer->getLEWord(); // request type
		buffer->getLEWord(); // request sequence number: 0x0002
		buffer->getLEWord(); // request subtype
		
		buffer->getByte(); // Success byte: always 0x0a
		
		result.fill( buffer );
		m_results.append( result );
		
		emit foundUser( result );
		
		// Last user found reply
		if ( requestSubType() == 0x01ae )
		{
			int moreUsersCount = buffer->getLEDWord();
			emit searchFinished( moreUsersCount );
			setSuccess( 0, QString::null );
		}
	}
	return true;
}

void UserSearchTask::searchUserByUIN( const QString& uin )
{
	//create a new result list
	m_type = UINSearch;
	
	FLAP f = { 0x02, client()->flapSequence(), 0 };
	SNAC s = { 0x0015, 0x0002, 0x0000, client()->snacSequence() };
	
	setRequestType( 0x07D0 ); //meta-information request
	setRequestSubType( 0x0569 ); //subtype: META_SEARCH_BY_UIN
	setSequence( f.sequence );
	Buffer* tlvdata = new Buffer();
	tlvdata->addLEWord( 0x0136 ); //tlv of type 0x0136 with length 4. all little endian
	tlvdata->addLEWord( 0x0004 );
	tlvdata->addLEDWord( uin.toULong() );
	Buffer* buf = addInitialData( tlvdata );
	delete tlvdata;
	
	Transfer* t = createTransfer( f, s, buf );
	send( t );
}

void UserSearchTask::searchWhitePages( const ICQWPSearchInfo& info )
{
	Q_UINT16 seq = client()->flapSequence();
	m_type = WhitepageSearch;
	
	FLAP f = { 0x02, client()->flapSequence(), 0 };
	SNAC s = { 0x0015, 0x0002, 0x0000, client()->snacSequence() };
	
	setRequestType( 0x07D0 );
	setRequestSubType( 0x0533 );
	setSequence( f.sequence );
	Buffer* tlvData = new Buffer();
	tlvData->addLELNTS( info.firstName.latin1() );
	tlvData->addLELNTS( info.lastName.latin1() );
	tlvData->addLELNTS( info.nickName.latin1() );
	tlvData->addLELNTS( info.email.latin1() );
	tlvData->addLEWord( info.age );
	tlvData->addLEWord( info.age );
	tlvData->addByte( info.gender );
	tlvData->addByte( info.language );
	tlvData->addLELNTS( info.city.latin1() );
	tlvData->addLEWord( 0x0000 );
	tlvData->addLEWord( info.country );
	tlvData->addLEWord( 0x0000 ); //company
	tlvData->addLEWord( 0x0000 ); //department
	tlvData->addLEWord( 0x0000 ); //position
	tlvData->addLEWord( info.occupation );
	tlvData->addLEWord( 0x0000 ); //past category
	tlvData->addLEWord( 0x0000 ); //past keywords
	tlvData->addLEWord( 0x0000 ); //interests category
	tlvData->addLEWord( 0x0000 ); //interests keywords
	tlvData->addLEWord( 0x0000 ); //affiliations category
	tlvData->addLEWord( 0x0000 ); //affiliations keywords
	tlvData->addLEWord( 0x0000 ); //homepage category
	tlvData->addLEWord( 0x0000 ); //homepage keywords
	if ( info.onlineOnly )
		tlvData->addByte( 0x01 );
	else
		tlvData->addByte( 0x00 );
	
	Buffer* buf = addInitialData( tlvData );
	delete tlvData; //we're done with it
	
	Transfer* t = createTransfer( f, s, buf );
	send( t );
}


#include "usersearchtask.moc"

//kate: indent-mode csands; tab-width 4; space-indent off; replace-tabs off;
