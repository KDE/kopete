/*
	Kopete Oscar Protocol
	icquserinfo.h - ICQ User Info Data Types

	Copyright (c) 2004 Matt Rogers <mattr@kde.org>
	Copyright (c) 2006 Roman Jarosz <kedgedev@centrum.cz>

	Kopete (c) 2002-2006 by the Kopete developers <kopete-devel@kde.org>

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
		kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Parsing ICQ short user info packet" << endl;
		nickname = buffer->getLELNTS();
		firstName = buffer->getLELNTS();
		lastName = buffer->getLELNTS();
		email = buffer->getLELNTS();
		needsAuth = buffer->getByte();
		buffer->skipBytes( 1 ); //skip the unknown byte
		gender = buffer->getByte();
	}
	else
		kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Couldn't parse ICQ short user info packet" << endl;
}

ICQGeneralUserInfo::ICQGeneralUserInfo()
{
	uin.init( 0 );
	country.init( 0 );
	timezone.init( 0 );
	publishEmail.init( false );
	webAware.init( false );
	allowsDC.init( false );
}

void ICQGeneralUserInfo::fill( Buffer* buffer )
{
	if ( buffer->getByte() == 0x0A )
	{
		kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Parsing ICQ basic user info packet" << endl;
		nickName = buffer->getLELNTS();
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
		authorization = ( buffer->getByte() == 0x00 );
		webAware = ( buffer->getByte() == 0x01 );
		allowsDC = ( buffer->getByte() == 0x01 ); //taken from sim
		publishEmail = ( buffer->getByte() == 0x01 );
	}
	else
		kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Couldn't parse ICQ basic user info packet" << endl;
}

void ICQGeneralUserInfo::store( Buffer* buffer )
{
	if ( nickName.hasChanged() )
	{
		Buffer buf;
		buf.addLELNTS( nickName.get() );
		buffer->addLETLV( 0x0154, buf );
	}
	if ( firstName.hasChanged() )
	{
		Buffer buf;
		buf.addLELNTS( firstName.get() );
		buffer->addLETLV( 0x0140, buf );
	}
	if ( lastName.hasChanged() )
	{
		Buffer buf;
		buf.addLELNTS( lastName.get() );
		buffer->addLETLV( 0x014A, buf );
	}
	if ( email.hasChanged() || publishEmail.hasChanged() )
	{
		Buffer buf;
		buf.addLELNTS( email.get() );
		buf.addByte( ( publishEmail.get() ) ? 0x00 : 0x01 );
		buffer->addLETLV( 0x015E, buf );
	}
	if ( city.hasChanged() )
	{
		Buffer buf;
		buf.addLELNTS( city.get() );
		buffer->addLETLV( 0x0190, buf );
	}
	if ( state.hasChanged() )
	{
		Buffer buf;
		buf.addLELNTS( state.get() );
		buffer->addLETLV( 0x019A, buf );
	}
	if ( phoneNumber.hasChanged() )
	{
		Buffer buf;
		buf.addLELNTS( phoneNumber.get() );
		buffer->addLETLV( 0x0276, buf );
	}
	if ( faxNumber.hasChanged() )
	{
		Buffer buf;
		buf.addLELNTS( faxNumber.get() );
		buffer->addLETLV( 0x0280, buf );
	}
	if ( address.hasChanged() )
	{
		Buffer buf;
		buf.addLELNTS( address.get() );
		buffer->addLETLV( 0x0262, buf );
	}
	if ( cellNumber.hasChanged() )
	{
		Buffer buf;
		buf.addLELNTS( cellNumber.get() );
		buffer->addLETLV( 0x028A, buf );
	}
	if ( zip.hasChanged() )
	{
		Buffer buf;
		buf.addLELNTS( zip.get() );
		buffer->addLETLV( 0x026D, buf );
	}
	if ( country.hasChanged() )
	{
		buffer->addLETLV16( 0x01A4, country.get() );
	}
	if ( timezone.hasChanged() )
	{
		buffer->addLETLV8( 0x0316, timezone.get() );
	}
	if ( webAware.hasChanged() )
	{
		buffer->addLETLV8( 0x02F8, ( webAware.get() ) ? 0x01 : 0x00 );
	}
	if ( authorization.hasChanged() )
	{
		buffer->addLETLV8( 0x030C, ( authorization.get() ) ? 0x00 : 0x01 );
	}
}

ICQWorkUserInfo::ICQWorkUserInfo()
{
	country.init( 0 );
	occupation.init( 0 );
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
		kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Couldn't parse ICQ work user info packet" << endl;
}

void ICQWorkUserInfo::store( Buffer* buffer )
{
	if ( city.hasChanged() )
	{
		Buffer buf;
		buf.addLELNTS( city.get() );
		buffer->addLETLV( 0x029E, buf );
	}

	if ( state.hasChanged() )
	{
		Buffer buf;
		buf.addLELNTS( state.get() );
		buffer->addLETLV( 0x02A8, buf );
	}

	if ( phone.hasChanged() )
	{
		Buffer buf;
		buf.addLELNTS( phone.get() );
		buffer->addLETLV( 0x02C6, buf );
	}

	if ( fax.hasChanged() )
	{
		Buffer buf;
		buf.addLELNTS( fax.get() );
		buffer->addLETLV( 0x02D0, buf );
	}

	if ( address.hasChanged() )
	{
		Buffer buf;
		buf.addLELNTS( address.get() );
		buffer->addLETLV( 0x0294, buf );
	}

	if ( zip.hasChanged() )
	{
		Buffer buf;
		buf.addLELNTS( zip.get() );
		buffer->addLETLV( 0x02BD, buf );
	}

	if ( country.hasChanged() )
	{
		buffer->addLETLV16( 0x02B2, country.get() );
	}

	if ( company.hasChanged() )
	{
		Buffer buf;
		buf.addLELNTS( company.get() );
		buffer->addLETLV( 0x01AE, buf );
	}

	if ( department.hasChanged() )
	{
		Buffer buf;
		buf.addLELNTS( department.get() );
		buffer->addLETLV( 0x01B8, buf );
	}

	if ( position.hasChanged() )
	{
		Buffer buf;
		buf.addLELNTS( position.get() );
		buffer->addLETLV( 0x01C2, buf );
	}

	if ( occupation.hasChanged() )
	{
		buffer->addLETLV16( 0x01CC, occupation.get() );
	}

	if ( homepage.hasChanged() )
	{
		Buffer buf;
		buf.addLELNTS( homepage.get() );
		buffer->addLETLV( 0x02DA, buf );
	}
}

ICQMoreUserInfo::ICQMoreUserInfo()
{
	age.init( 0 );
	gender.init( 0 );
	birthdayYear.init( 0 );
	birthdayMonth.init( 0 );
	birthdayDay.init( 0 );
	lang1.init( 0 );
	lang2.init( 0 );
	lang3.init( 0 );
	ocountry.init( 0 );
	marital.init( 0 );
	sendInfo.init( false );
}

void ICQMoreUserInfo::fill( Buffer* buffer )
{
	if ( buffer->getByte() == 0x0A )
	{
		age = buffer->getLEWord();
		gender = buffer->getByte();
		homepage = buffer->getLELNTS();
		birthdayYear = buffer->getLEWord();
		birthdayMonth = buffer->getByte();
		birthdayDay = buffer->getByte();
		lang1 = buffer->getByte();
		lang2 = buffer->getByte();
		lang3 = buffer->getByte();
		buffer->getLEWord(); //emtpy field
		ocity = buffer->getLELNTS();
		ostate = buffer->getLELNTS();
		ocountry = buffer->getLEWord();
		marital = buffer->getByte();
		sendInfo = buffer->getByte();
	}
	else
		kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Couldn't parse ICQ work user info packet" << endl;
}

void ICQMoreUserInfo::store( Buffer* buffer )
{
	if ( age.hasChanged() )
	{
		buffer->addLETLV16( 0x0172, age.get() );
	}

	if ( gender.hasChanged() )
	{
		buffer->addLETLV8( 0x017C, gender.get() );
	}
	
	if ( homepage.hasChanged() )
	{
		Buffer buf;
		buf.addLELNTS( homepage.get() );
		buffer->addLETLV( 0x0213, buf );
	}

	if ( birthdayYear.hasChanged() || birthdayMonth.hasChanged() || birthdayDay.hasChanged() )
	{
		Buffer buf;
		buf.addLEWord( birthdayYear.get() );
		buf.addLEWord( birthdayMonth.get() );
		buf.addLEWord( birthdayDay.get() );
		buffer->addLETLV( 0x023A, buf );
	}

	if ( lang1.hasChanged() || lang2.hasChanged() || lang3.hasChanged() )
	{
		buffer->addLETLV16( 0x0186, lang1.get() );
		buffer->addLETLV16( 0x0186, lang2.get() );
		buffer->addLETLV16( 0x0186, lang3.get() );
	}

	if ( ocity.hasChanged() )
	{
		Buffer buf;
		buf.addLELNTS( ocity.get() );
		buffer->addLETLV( 0x0320, buf );
	}

	if ( ostate.hasChanged() )
	{
		Buffer buf;
		buf.addLELNTS( ostate.get() );
		buffer->addLETLV( 0x032A, buf );
	}

	if ( ocountry.hasChanged() )
	{
		buffer->addLETLV16( 0x0334, ocountry.get() );
	}

// 	if ( marital.hasChanged() )
// 	{
// 	}
	
	if ( sendInfo.hasChanged() )
	{
		buffer->addLETLV8( 0x0348, sendInfo.get() );
	}
}

ICQEmailInfo::ICQEmailInfo()
{
}

void ICQEmailInfo::fill( Buffer* buffer )
{
	if ( buffer->getByte() == 0x0A )
	{
		QList<EmailItem> emails;
		int numEmails = buffer->getByte();
		for ( int i = 0; i < numEmails; i++ )
		{
			EmailItem item;
			item.publish = ( buffer->getByte() == 0x00 );
			item.email = buffer->getLELNTS();
			emails.append( item );
		}
		emailList = emails;
	}
	else
		kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Couldn't parse ICQ email user info packet" << endl;
}

void ICQEmailInfo::store( Buffer* buffer )
{
	if ( emailList.hasChanged() )
	{
		int size = emailList.get().size();
		for ( int i = 0; i < size; i++ )
		{
			EmailItem item = emailList.get().at(i);

			Buffer buf;
			buf.addLELNTS( item.email );
			buf.addByte( ( item.publish ) ? 0x00 : 0x01 );
			buffer->addLETLV( 0x015E, buf );
		}
	}
}

ICQNotesInfo::ICQNotesInfo()
{
}

void ICQNotesInfo::fill( Buffer* buffer )
{
	if ( buffer->getByte() == 0x0A )
		notes = buffer->getLELNTS();
	else
		kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Couldn't parse ICQ notes user info packet" << endl;
}

void ICQNotesInfo::store( Buffer* buffer )
{
	if ( notes.hasChanged() )
	{
		Buffer buf;
		buf.addLELNTS( notes.get() );
		buffer->addLETLV( 0x0258, buf );
	}
}

ICQInterestInfo::ICQInterestInfo()
{
	for ( int i = 0; i < 4; i++ )
		topics[i].init( 0 );
}

void ICQInterestInfo::fill( Buffer* buffer )
{
	if ( buffer->getByte() == 0x0A )
	{
		int count=0; //valid interests
		int len= buffer->getByte();  //interests we get
		for ( int i = 0; i < len; i++ )
		{
			int t=buffer->getLEWord();
			QByteArray d = buffer->getLELNTS();
			if (count<4) { //i think this could not happen, i have never seen more
				topics[count]=t;
				descriptions[count]=d;
				count++;
			} else {
				kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "got more than four interest infos" << endl;
			}
		}
		if ( count < 4 )
		{
			for ( int i = count; i < 4; i++ )
			{
				topics[i] = 0;
				descriptions[i] = QByteArray();
			}
		}
		kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "LEN: "<< len << " COUNT: " << count<< endl;
	}
	else
		kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Couldn't parse ICQ interest user info packet" << endl;
}

void ICQInterestInfo::store( Buffer* buffer )
{
	bool hasChanged = false;
	for ( int i = 0; i < 4; i++ )
	{
		if ( topics[i].hasChanged() || descriptions[i].hasChanged() )
		{
			hasChanged = true;
			break;
		}
	}

	if ( hasChanged )
	{
		for ( int i = 0; i < 4; i++ )
		{
			if ( topics[i].get() != 0 )
			{
				Buffer buf;
				buf.addLEWord( topics[i].get() );
				buf.addLELNTS( descriptions[i].get() );
				buffer->addLETLV( 0x01EA, buf );
			}
		}
	}
}

ICQOrgAffInfo::ICQOrgAffInfo()
{
	org1Category.init( 0 );
	org2Category.init( 0 );
	org3Category.init( 0 );
	pastAff1Category.init( 0 );
	pastAff2Category.init( 0 );
	pastAff3Category.init( 0 );
}

void ICQOrgAffInfo::fill( Buffer* buffer )
{
	if ( buffer->getByte() == 0x0A )
	{
		if ( buffer->getByte() != 0x03 )
		{
			kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Couldn't parse ICQ affiliation info packet" << endl;
			return;
		}
		
		pastAff1Category = buffer->getLEWord();
		pastAff1Keyword = buffer->getLELNTS();
		pastAff2Category = buffer->getLEWord();
		pastAff2Keyword = buffer->getLELNTS();
		pastAff3Category = buffer->getLEWord();
		pastAff3Keyword = buffer->getLELNTS();
		
		if ( buffer->getByte() != 0x03 )
		{
			kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Couldn't parse ICQ organization info packet" << endl;
			return;
		}
		
		org1Category = buffer->getLEWord();
		org1Keyword = buffer->getLELNTS();
		org2Category = buffer->getLEWord();
		org2Keyword = buffer->getLELNTS();
		org3Category = buffer->getLEWord();
		org3Keyword = buffer->getLELNTS();
	}
	else
		kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Couldn't parse ICQ organization & affiliation info packet" << endl;

}

void ICQOrgAffInfo::store( Buffer* buffer )
{
	if ( org1Category.hasChanged() || org1Keyword.hasChanged() ||
	     org2Category.hasChanged() || org2Keyword.hasChanged() ||
	     org3Category.hasChanged() || org3Keyword.hasChanged() )
	{
		if ( org1Category.get() != 0 )
		{
			Buffer buf;
			buf.addLEWord( org1Category.get() );
			buf.addLELNTS( org1Keyword.get() );
			buffer->addLETLV( 0x01FE, buf );
		}

		if ( org2Category.get() != 0 )
		{
			Buffer buf;
			buf.addLEWord( org2Category.get() );
			buf.addLELNTS( org2Keyword.get() );
			buffer->addLETLV( 0x01FE, buf );
		}

		if ( org3Category.get() != 0 )
		{
			Buffer buf;
			buf.addLEWord( org3Category.get() );
			buf.addLELNTS( org3Keyword.get() );
			buffer->addLETLV( 0x01FE, buf );
		}
	}

	if ( pastAff1Category.hasChanged() || pastAff1Keyword.hasChanged() ||
	     pastAff2Category.hasChanged() || pastAff2Keyword.hasChanged() ||
	     pastAff3Category.hasChanged() || pastAff3Keyword.hasChanged() )
	{
		if ( pastAff1Category.get() != 0 )
		{
			Buffer buf;
			buf.addLEWord( pastAff1Category.get() );
			buf.addLELNTS( pastAff1Keyword.get() );
			buffer->addLETLV( 0x01D6, buf );
		}

		if ( pastAff2Category.get() != 0 )
		{
			Buffer buf;
			buf.addLEWord( pastAff2Category.get() );
			buf.addLELNTS( pastAff2Keyword.get() );
			buffer->addLETLV( 0x01D6, buf );
		}

		if ( pastAff3Category.get() != 0 )
		{
			Buffer buf;
			buf.addLEWord( pastAff3Category.get() );
			buf.addLELNTS( pastAff3Keyword.get() );
			buffer->addLETLV( 0x01D6, buf );
		}
	}
}

ICQSearchResult::ICQSearchResult()
{
	auth = false;
	online = false;
	gender = 'U';
}

void ICQSearchResult::fill( Buffer* buffer )
{
	buffer->getLEWord(); // data length ( eat it )

	uin = buffer->getLEDWord();
	kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Found UIN " << QString::number( uin ) << endl;
	
	nickName = buffer->getLELNTS();
	firstName = buffer->getLELNTS();
	lastName = buffer->getLELNTS();
	email = buffer->getLELNTS();

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
