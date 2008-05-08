/*
    gwfield.cpp - Fields used for Request/Response data in GroupWise
    Copyright (c) 2004      SUSE Linux AG	 	 http://www.suse.com
    
    Based on Testbed    
    Copyright (c) 2003      by Will Stephenson		 <will@stevello.free-online.co.uk>
    
    Kopete    (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include <q3cstring.h>

#include "gwerror.h"

#ifdef LIBGW_USE_KDEBUG
  #include <kdebug.h>
#endif

#include "gwfield.h"
#include <iostream>

using namespace Field;
using namespace std;

QLatin1String Field::NM_A_IP_ADDRESS				("nnmIPAddress");
QLatin1String Field::NM_A_PORT					("nnmPort");
QLatin1String Field::NM_A_FA_FOLDER				("NM_A_FA_FOLDER");
QLatin1String Field::NM_A_FA_CONTACT				("NM_A_FA_CONTACT");
QLatin1String Field::NM_A_FA_CONVERSATION		("NM_A_FA_CONVERSATION");
QLatin1String Field::NM_A_FA_MESSAGE				("NM_A_FA_MESSAGE");
QLatin1String Field::NM_A_FA_CONTACT_LIST		("NM_A_FA_CONTACT_LIST");
QLatin1String Field::NM_A_FA_RESULTS				("NM_A_FA_RESULTS");
QLatin1String Field::NM_A_FA_INFO_DISPLAY_ARRAY	("NM_A_FA_INFO_DISPLAY_ARRAY");
QLatin1String Field::NM_A_FA_USER_DETAILS		("NM_A_FA_USER_DETAILS");
QLatin1String Field::NM_A_SZ_OBJECT_ID			("NM_A_SZ_OBJECT_ID");
QLatin1String Field::NM_A_SZ_PARENT_ID			("NM_A_SZ_PARENT_ID");
QLatin1String Field::NM_A_SZ_SEQUENCE_NUMBER		("NM_A_SZ_SEQUENCE_NUMBER");
QLatin1String Field::NM_A_SZ_TYPE				("NM_A_SZ_TYPE");
QLatin1String Field::NM_A_SZ_STATUS				("NM_A_SZ_STATUS");
QLatin1String Field::NM_A_SZ_STATUS_TEXT			("NM_A_SZ_STATUS_TEXT");
QLatin1String Field::NM_A_SZ_DN					("NM_A_SZ_DN");
QLatin1String Field::NM_A_SZ_DISPLAY_NAME		("NM_A_SZ_DISPLAY_NAME");
QLatin1String Field::NM_A_SZ_USERID				("NM_A_SZ_USERID");
QLatin1String Field::NM_A_SZ_CREDENTIALS			("NM_A_SZ_CREDENTIALS");
QLatin1String Field::NM_A_SZ_MESSAGE_BODY		("NM_A_SZ_MESSAGE_BODY");
QLatin1String Field::NM_A_SZ_MESSAGE_TEXT		("NM_A_SZ_MESSAGE_TEXT");
QLatin1String Field::NM_A_UD_MESSAGE_TYPE		("NM_A_UD_MESSAGE_TYPE");
QLatin1String Field::NM_A_FA_PARTICIPANTS		("NM_A_FA_PARTICIPANTS");
QLatin1String Field::NM_A_FA_INVITES				("NM_A_FA_INVITES");
QLatin1String Field::NM_A_FA_EVENT				("NM_A_FA_EVENT");
QLatin1String Field::NM_A_UD_COUNT				("NM_A_UD_COUNT");
QLatin1String Field::NM_A_UD_DATE				("NM_A_UD_DATE");
QLatin1String Field::NM_A_UD_EVENT				("NM_A_UD_EVENT");
QLatin1String Field::NM_A_B_NO_CONTACTS			("NM_A_B_NO_CONTACTS");
QLatin1String Field::NM_A_B_NO_CUSTOMS			("NM_A_B_NO_CUSTOMS");
QLatin1String Field::NM_A_B_NO_PRIVACY			("NM_A_B_NO_PRIVACY");
QLatin1String Field::NM_A_B_ONLY_MODIFIED		("NM_A_B_ONLY_MODIFIED");
QLatin1String Field::NM_A_UW_STATUS				("NM_A_UW_STATUS");
QLatin1String Field::NM_A_UD_OBJECT_ID			("NM_A_UD_OBJECT_ID");
QLatin1String Field::NM_A_SZ_TRANSACTION_ID		("NM_A_SZ_TRANSACTION_ID");
QLatin1String Field::NM_A_SZ_RESULT_CODE			("NM_A_SZ_RESULT_CODE");
QLatin1String Field::NM_A_UD_BUILD				("NM_A_UD_BUILD");
QLatin1String Field::NM_A_SZ_AUTH_ATTRIBUTE		("NM_A_SZ_AUTH_ATTRIBUTE");
QLatin1String Field::NM_A_UD_KEEPALIVE			("NM_A_UD_KEEPALIVE");
QLatin1String Field::NM_A_SZ_USER_AGENT			("NM_A_SZ_USER_AGENT");
QLatin1String Field::NM_A_BLOCKING				("nnmBlocking");
QLatin1String Field::NM_A_BLOCKING_DENY_LIST		("nnmBlockingDenyList");
QLatin1String Field::NM_A_BLOCKING_ALLOW_LIST	("nnmBlockingAllowList");
QLatin1String Field::NM_A_SZ_BLOCKING_ALLOW_ITEM	("NM_A_SZ_BLOCKING_ALLOW_ITEM");
QLatin1String Field::NM_A_SZ_BLOCKING_DENY_ITEM	("NM_A_SZ_BLOCKING_DENY_ITEM");
QLatin1String Field::NM_A_LOCKED_ATTR_LIST		("nnmLockedAttrList");
QLatin1String Field::NM_A_SZ_DEPARTMENT			("OU");
QLatin1String Field::NM_A_SZ_TITLE				("Title");
// GW7
QLatin1String Field::NM_A_FA_CUSTOM_STATUSES		("NM_A_FA_CUSTOM_STATUSES");
QLatin1String Field::NM_A_FA_STATUS				("NM_A_FA_STATUS");
QLatin1String Field::NM_A_UD_QUERY_COUNT			("NM_A_UD_QUERY_COUNT");
QLatin1String Field::NM_A_FA_CHAT				("NM_A_FA_CHAT");
QLatin1String Field::NM_A_DISPLAY_NAME			("nnmDisplayName");
QLatin1String Field::NM_A_CHAT_OWNER_DN			("nnmChatOwnerDN");
QLatin1String Field::NM_A_UD_PARTICIPANTS		("NM_A_UD_PARTICIPANTS");
QLatin1String Field::NM_A_DESCRIPTION			("nnmDescription");
QLatin1String Field::NM_A_DISCLAIMER				("nnmDisclaimer");
QLatin1String Field::NM_A_QUERY					("nnmQuery");
QLatin1String Field::NM_A_ARCHIVE				("nnmArchive");
QLatin1String Field::NM_A_MAX_USERS				("nnmMaxUsers");
QLatin1String Field::NM_A_SZ_TOPIC				("NM_A_SZ_TOPIC");
QLatin1String Field::NM_A_FA_CHAT_ACL			("NM_A_FA_CHAT_ACL");
QLatin1String Field::NM_A_FA_CHAT_ACL_ENTRY		("NM_A_FA_CHAT_ACL_ENTRY");
QLatin1String Field::NM_A_SZ_ACCESS_FLAGS		("NM_A_SZ_ACCESS_FLAGS");
QLatin1String Field::NM_A_CHAT_CREATOR_DN		("nnmCreatorDN");
QLatin1String Field::NM_A_CREATION_TIME			("nnmCreationTime");
QLatin1String Field::NM_A_UD_CHAT_RIGHTS			("NM_A_UD_CHAT_RIGHTS");

QLatin1String Field::KOPETE_NM_USER_DETAILS_CN("CN");
QLatin1String Field::KOPETE_NM_USER_DETAILS_GIVEN_NAME("Given Name");
QLatin1String Field::KOPETE_NM_USER_DETAILS_SURNAME("Surname");
QLatin1String Field::KOPETE_NM_USER_DETAILS_ARCHIVE_FLAG("nnmArchive");
QLatin1String Field::KOPETE_NM_USER_DETAILS_FULL_NAME("Full Name");


QLatin1String Field::NM_FIELD_TRUE				("1");
QLatin1String Field::NM_FIELD_FALSE				("0");

/* === FieldList ==================================================== */
FieldList::~FieldList()
{
}

FieldListIterator FieldList::find( QLatin1String tag )
{
	FieldListIterator it = begin();
	return find( it, tag );
}

FieldListIterator FieldList::find( FieldListIterator &it, QLatin1String tag )
{
	FieldListIterator theEnd = end();
	//qDebug() << "FieldList::find() looking for " << tag << endl;
	for ( ; it != theEnd; ++it )
	{
		//qDebug() << " - on " << (*it)->tag() << endl;
		if ( (*it)->tag() == tag )
			break;
	}
	return it;
}

int FieldList::findIndex( QLatin1String tag )
{
	FieldListIterator it = begin();
	FieldListIterator theEnd = end();
	int index = 0;
	for ( ; it != theEnd; ++it, ++index )
		if ( (*it)->tag() == tag )
			return index;
			
	return -1;
}

void FieldList::dump( bool recursive, int offset )
{
	const FieldListIterator myEnd = end();
	if ( !offset )
//		kDebug() << ( recursive ? ", recursively" : ", non-recursive" );
	for( FieldListIterator it = begin(); it != myEnd; ++it )
	{
		QString s;
		s.fill(' ', offset*2 );
		s.append( (*it)->tag() );
		SingleField * sf;
		if ( ( sf = dynamic_cast<SingleField*>( *it ) ) )
		{
			s.append( " :" );
			s.append( sf->value().toString() );
		}
//		kDebug() << s;
		if ( recursive )
		{
			MultiField * mf;
			if ( ( mf = dynamic_cast<MultiField*>( *it ) ) )
				mf->fields().dump( recursive, offset+1 );
		}
	}
}

void FieldList::purge()
{
	Field::FieldListIterator it = begin();
	Field::FieldListIterator theEnd = end();
	int index = 0;
	for ( ; it != theEnd; ++it, ++index )
		delete *it;
}

// THIS IS AN ATTEMPT TO HIDE THE POLYMORPHISM INSIDE THE LIST
// HOWEVER IT FAILS BECAUSE WE NEED BOTH THE ITERATOR AND THE CASTED Single|MultiField it points to

SingleField * FieldList::findSingleField( QLatin1String tag )
{
	FieldListIterator it = begin();
	return findSingleField( it, tag );
}

SingleField * FieldList::findSingleField( FieldListIterator &it, QLatin1String tag )
{
	FieldListIterator found = find( it, tag );
	if ( found == end() )
		return 0;
	else
		return dynamic_cast<SingleField *>( *found );
}

MultiField * FieldList::findMultiField( QLatin1String tag )
{
	FieldListIterator it = begin();
	return findMultiField( it, tag );
}

MultiField * FieldList::findMultiField( FieldListIterator &it, QLatin1String tag )
{
	FieldListIterator found = find( it, tag );
	if ( found == end() )
		return 0;
	else
	return dynamic_cast<MultiField *>( *found );
}


/* === FieldBase ========================================================= */

FieldBase::FieldBase( QLatin1String tag, quint8 method, quint8 flags, quint8 type )
: m_tag( tag ), m_method( method ), m_flags( flags ), m_type( type )
{

}

QLatin1String FieldBase::tag() const
{
	return m_tag;
}

quint8 FieldBase::method() const
{
	return m_method;
}

quint8 FieldBase::flags() const
{
	return m_flags;
}

quint8 FieldBase::type() const
{
	return m_type;
}

void FieldBase::setFlags( const quint8 flags )
{
	m_flags = flags;
}

/* === SingleField ========================================================= */

SingleField::SingleField( QLatin1String tag, quint8 method, quint8 flags, quint8 type, QVariant value )
: FieldBase( tag, method, flags, type ), m_value( value )
{
}

SingleField::SingleField( QLatin1String tag, quint8 flags, quint8 type, QVariant value )
: FieldBase( tag, NMFIELD_METHOD_VALID, flags, type ), m_value( value )
{
}

SingleField::~SingleField()
{
}

void SingleField::setValue( const QVariant v )
{
	m_value = v;
}

QVariant SingleField::value() const
{
	return m_value;
}

/* === MultiField ========================================================= */

MultiField::MultiField( QLatin1String tag, quint8 method, quint8 flags, quint8 type, FieldList fields )
: FieldBase( tag, method, flags, type ), m_fields( fields )
{
}

MultiField::MultiField( QLatin1String tag, quint8 method, quint8 flags, quint8 type )
: FieldBase( tag, method, flags, type )
{
}

MultiField::~MultiField()
{
	m_fields.purge();
}

FieldList MultiField::fields() const
{
	return m_fields;
}

void MultiField::setFields( FieldList fields )
{
	m_fields = fields;
}
