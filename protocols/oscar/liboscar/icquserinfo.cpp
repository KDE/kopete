/*
	Kopete Oscar Protocol
	icquserinfo.h - ICQ User Info Data Types
	
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

#include "icquserinfo.h"
#include "buffer.h"

#include <kdebug.h>

ICQShortInfo::ICQShortInfo()
{
	uin = 0;
	needsAuth = false;
	gender = 0;
}

void ICQShortInfo::fill( Buffer* buffer )
{
	if ( buffer->getByte() == 0x0A )
	{
		kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Parsing ICQ short user info packet" << endl;
		nickname = buffer->getLELNTS();
		firstName = buffer->getLELNTS();
		lastName = buffer->getLELNTS();
		email = buffer->getLELNTS();
		needsAuth = buffer->getByte();
		buffer->skipBytes( 1 ); //skip the unknown byte
		gender = buffer->getByte();
	}
	else
		kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Couldn't parse ICQ short user info packet" << endl;
}

ICQGeneralUserInfo::ICQGeneralUserInfo()
{
	uin = 0;
	country = 0;
	timezone = 0;
	publishEmail = false;
	webaware = false;
	allowsDC = false;
}

void ICQGeneralUserInfo::fill( Buffer* buffer )
{
	if ( buffer->getByte() == 0x0A )
	{
		kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Parsing ICQ basic user info packet" << endl;
		nickname = buffer->getLELNTS();
		firstName = buffer->getLELNTS();
		lastName = buffer->getLELNTS();
		email = buffer->getLELNTS();
		city = buffer->getLELNTS();
		state = buffer->getLELNTS();
		phoneNumber = buffer->getLELNTS();
		faxNumber = buffer->getLELNTS();
		address = buffer->getLELNTS();
		cellNumber = buffer->getLELNTS();
		zip = buffer->getLELNTS();
		country = buffer->getLEWord();
		timezone = buffer->getLEByte(); // UTC+(tzcode * 30min)
		webaware = ( buffer->getByte() == 0x01 );
		allowsDC = ( buffer->getByte() == 0x01 ); //taken from sim
		publishEmail = ( buffer->getByte() == 0x01 );
	}
	else
		kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Couldn't parse ICQ basic user info packet" << endl;
}

ICQWorkUserInfo::ICQWorkUserInfo()
{
	country = 0;
	occupation = 0;
}

void ICQWorkUserInfo::fill( Buffer* buffer )
{
	if ( buffer->getByte() == 0x0A )
	{
		city = buffer->getLELNTS();
		state = buffer->getLELNTS();
		phone = buffer->getLELNTS();
		fax = buffer->getLELNTS();
		address = buffer->getLELNTS();
		zip = buffer->getLELNTS();
		country = buffer->getLEWord();
		company = buffer->getLELNTS();
		department = buffer->getLELNTS();
		position = buffer->getLELNTS();
		occupation = buffer->getLEWord();
		homepage = buffer->getLELNTS();
	}
	else
		kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Couldn't parse ICQ work user info packet" << endl;
}

ICQMoreUserInfo::ICQMoreUserInfo()
{
	age = 0;
	gender = 0;
	lang1 = 0;
	lang2 = 0;
	lang3 = 0;
	ocountry = 0;
	marital = 0;
}

void ICQMoreUserInfo::fill( Buffer* buffer )
{
	if ( buffer->getByte() == 0x0A )
	{
		age = buffer->getLEWord();
		gender = buffer->getByte();
		homepage = buffer->getLELNTS();
		WORD year = buffer->getLEWord();
		BYTE month = buffer->getByte();
		BYTE day = buffer->getByte();
		
		// set birthday to NULL if at least one of the values in the buffer is 0
		if ( year == 0 || month == 0 || day == 0 )
			birthday = QDate();
		else
			birthday = QDate( year, month, day );
		
		lang1 = buffer->getByte();
		lang2 = buffer->getByte();
		lang3 = buffer->getByte();
		buffer->getLEWord(); //emtpy field
		ocity = buffer->getLELNTS();
		ostate = buffer->getLELNTS();
		ocountry = buffer->getLEWord();
		marital = buffer->getLEWord();
	}
	else
		kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Couldn't parse ICQ work user info packet" << endl;
}

ICQEmailInfo::ICQEmailInfo()
{
}

void ICQEmailInfo::fill( Buffer* buffer )
{
	if ( buffer->getByte() == 0x0A )
	{
		int numEmails = buffer->getByte();
		QString email;
		for ( int i = 0; i < numEmails; i++ )
		{
			if ( buffer->getByte() == 0x00 )
				email = buffer->getLELNTS();
		}
	}
	else
		kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Coudln't parse ICQ email user info packet" << endl;
}

ICQInterestInfo::ICQInterestInfo()
{
	count=0;
}

void ICQInterestInfo::fill( Buffer* buffer )
{
	if ( buffer->getByte() == 0x0A )
	{
		count=0; //valid interests
		int len= buffer->getByte();  //interests we get
		for ( int i = 0; i < len; i++ )
		{
			int t=buffer->getLEWord();
			QCString d = buffer->getLELNTS();
			if (t>0) { //there is some topic
				if (count<4) { //i think this could not happen, i have never seen more
					topics[count]=t;
					descriptions[count]=d;
					kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "got topic: "<<topics[count]<<" desc: " << topics[count] << endl;
					count++;
				} else {
					kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "got more than four interest infos" << endl;
				}
			}
		}
		kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "LEN: "<< len << " COUNT: " << count<< endl;
	}
	else
		kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Coudln't parse ICQ interest user info packet" << endl;
}

ICQSearchResult::ICQSearchResult()
{
	auth = false;
	online = false;
	gender = 'U';
}

void ICQSearchResult::fill( Buffer* buffer )
{
	WORD datalength = buffer->getLEWord(); // data length
	WORD len = 0;
	uin = buffer->getLEDWord();
	kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Found UIN " << QString::number( uin ) << endl;
	len = buffer->getLEWord();
	if ( len > 0 )
		nickName = QCString( buffer->getBlock( len ) );
	
	len = buffer->getLEWord();
	if ( len > 0 )
		firstName = QCString( buffer->getBlock( len ) );
	
	len = buffer->getLEWord();
	if ( len > 0 )
		lastName = QCString( buffer->getBlock( len ) );
	
	len = buffer->getLEWord();
	if ( len > 0 )
		email = QCString( buffer->getBlock( len ) );
	
	auth = ( buffer->getByte() != 0x01 );
	online = ( buffer->getLEWord() == 0x0001 );
	switch ( buffer->getByte() )
	{
	case 0x00:
		gender = 'M';
		break;
	case 0x01:
		gender = 'F';
		break;
	default:
		gender = 'U';
		break;
	}
	age = buffer->getLEWord();
}

ICQWPSearchInfo::ICQWPSearchInfo()
{
	age = 0;
	gender = 0;
	language = 0;
	country = 0;
	occupation = 0;
	onlineOnly = false;
}

	

//kate: space-indent off; tab-width 4; replace-tabs off; indent-mode csands;
