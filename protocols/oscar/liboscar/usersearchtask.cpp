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
		
		quint16 seq = 0;
		SnacTransfer* st = dynamic_cast<SnacTransfer*>( t );
		if ( st )
			seq = st->snacRequest();
		
		TLV tlv1 = transfer()->buffer()->getTLV();
		Buffer* buffer = new Buffer( tlv1.data, tlv1.length );
		
		if ( seq == 0 )
		{
			setTransfer( 0 );
			return false;
		}
			
		ICQSearchResult result;
		buffer->getLEWord(); // data chunk size
		/*DWORD receiverUin =*/ buffer->getLEDWord(); // target uin
		buffer->getLEWord(); // request type
		buffer->getLEWord(); // request sequence number: 0x0002
		buffer->getLEWord(); // request subtype
		
		BYTE success = buffer->getByte(); // Success byte: always 0x0a
		
		if ( ( success == 0x32 ) || ( success == 0x14 ) || ( success == 0x1E ) )
			result.uin = 1;
		else
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
		setTransfer( 0 );
	}
	return true;
}

void UserSearchTask::searchUserByUIN( const QString& uin )
{
	//create a new result list
	m_type = UINSearch;
	
	FLAP f = { 0x02, 0, 0 };
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
	m_type = WhitepageSearch;
	
	FLAP f = { 0x02, 0, 0 };
	SNAC s = { 0x0015, 0x0002, 0x0000, client()->snacSequence() };
	
	setRequestType( 0x07D0 );
	setRequestSubType( 0x0533 );
	setSequence( f.sequence );
	Buffer* tlvData = new Buffer();
	/*
		search.addLEWord(0x0533); // subtype: 1331
	
	//LNTS FIRST
	search.addLEWord(first.length());
	if(first.length()>0)
		search.addLEString(first.toLatin1(), first.length());
	
	// LNTS LAST
	search.addLEWord(last.length());
	if(last.length()>0)
		search.addLEString(last.toLatin1(), last.length());
	
	// LNTS NICK
	search.addLEWord(nick.length());
	if(nick.length()>0)
		search.addLEString(nick.toLatin1(), nick.length());
	
	// LNTS EMAIL
	search.addLEWord(mail.length());
	if(mail.length()>0)
		search.addLEString(mail.toLatin1(), mail.length());
	
	// WORD.L MINAGE
	search.addLEWord(minage);
	
	// WORD.L MAXAGE
	search.addLEWord(maxage);
	
	// BYTE xx SEX 1=fem, 2=mal, 0=dontcare
	if (sex==1)
		search.addLEByte(0x01);
	else if(sex==2)
		search.addLEByte(0x02);
	else
		search.addLEByte(0x00);
	
	// BYTE xx LANGUAGE
	search.addLEByte(lang);
	
	// LNTS CITY
	search.addLEWord(city.length());
	if(city.length()>0)
		search.addLEString(city.toLatin1(), city.length());
	
	// LNTS STATE
	search.addLEWord(state.length());
	if(state.length()>0)
		search.addLEString(state.toLatin1(), state.length());
	
	// WORD.L xx xx COUNTRY
	search.addLEWord(country);
	
	// LNTS COMPANY
	search.addLEWord(company.length());
	if(company.length()>0)
		search.addLEString(company.toLatin1(), company.length());
	
	// LNTS DEPARTMENT
	search.addLEWord(department.length());
	if(department.length()>0)
		search.addLEString(department.toLatin1(), department.length());
	
	// LNTS POSITION
	search.addLEWord(position.length());
	if(position.length()>0)
		search.addLEString(position.toLatin1(), position.length());
	
	// BYTE xx OCCUPATION
	search.addLEByte(occupation);
	
	//WORD.L xx xx PAST
	search.addLEWord(0x0000);
	
	//LNTS PASTDESC - The past description to search for.
	search.addLEWord(0x0000);
	
	// WORD.L xx xx INTERESTS - The interests category to search for.
	search.addLEWord(0x0000);
	
	// LNTS INTERDESC - The interests description to search for.
	search.addLEWord(0x0000);
	
	// WORD.L xx xx AFFILIATION - The affiliation to search for.
	search.addLEWord(0x0000);
	
	// LNTS AFFIDESC - The affiliation description to search for.
	search.addLEWord(0x0000);
	
	// WORD.L xx xx HOMEPAGE - The home page category to search for.
	search.addLEWord(0x0000);
	
	// LNTS HOMEDESC - The home page description to search for.
	search.addLEWord(0x0000);
	
	// BYTE xx ONLINE 1=online onliners, 0=dontcare
	if(onlineOnly)
		search.addLEByte(0x01);
	else
		search.addLEByte(0x00);
	*/
	if ( !info.firstName.isEmpty() )
	{
		tlvData->addLEWord( info.firstName.length() );
		tlvData->addLEString( info.firstName.toLatin1(), info.firstName.length() );
	}
	else
		tlvData->addLEWord( 0x0000 );
	
	if ( !info.lastName.isEmpty() )
	{
		tlvData->addLEWord( info.lastName.length() );
		tlvData->addLEString( info.lastName.toLatin1(), info.lastName.length() );
	}
	else
		tlvData->addLEWord( 0x0000 );
	
	if ( !info.nickName.isEmpty() )
	{
		tlvData->addLEWord( info.nickName.length() );
		tlvData->addLEString( info.nickName.toLatin1(), info.nickName.length() );
	}
	else
		tlvData->addLEWord( 0x0000 );
	
	if ( !info.email.isEmpty() )
	{
		tlvData->addLEWord( info.email.length() );
		tlvData->addLEString( info.email.toLatin1(), info.email.length() );
	}
	else
		tlvData->addLEWord( 0x0000 );
	
	tlvData->addLEWord( info.age );
	tlvData->addLEWord( info.age );
	tlvData->addByte( info.gender );
	tlvData->addByte( info.language );
	
	if ( !info.city.isEmpty() )
	{
		tlvData->addLEWord( info.city.length() );
		tlvData->addLEString( info.city.toLatin1(), info.city.length() );
	}
	else
		tlvData->addLEWord( 0x0000 );

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
