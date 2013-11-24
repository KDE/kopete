/*
    Kopete Groupwise Protocol
    getdetailstask.cpp - fetch a contact's details from the server

    Copyright (c) 2006      Novell, Inc	 	 	 http://www.opensuse.org
    Copyright (c) 2004      SUSE Linux AG	 	 http://www.suse.com
    
    Based on Iris, Copyright (C) 2003  Justin Karneges <justin@affinix.com>

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

#include "getdetailstask.h"

#include "client.h"
#include "response.h"
#include "userdetailsmanager.h"

using namespace GroupWise;

GetDetailsTask::GetDetailsTask( Task * parent )
 : RequestTask( parent )
{
}


GetDetailsTask::~GetDetailsTask()
{
}

void GetDetailsTask::userDNs( const QStringList & userDNs )
{
	Field::FieldList lst;
	for ( QStringList::ConstIterator it = userDNs.begin(); it != userDNs.end(); ++it )
	{
		lst.append( new Field::SingleField( Field::NM_A_SZ_DN, 0, NMFIELD_TYPE_UTF8, *it ) );
	}
	createTransfer( "getdetails", lst );
}

bool GetDetailsTask::take( Transfer * transfer )
{
	if ( !forMe( transfer ) )
		return false;
	Response * response = dynamic_cast<Response *>( transfer );
	if ( !response )
		return false;
	
	Field::FieldList detailsFields = response->fields();
	// parse received details and signal like billio
	Field::MultiField * container = 0;
	Field::FieldListIterator end = detailsFields.end();
	for ( Field::FieldListIterator it = detailsFields.find( Field::NM_A_FA_RESULTS );
		  it != end;
		  it = detailsFields.find( ++it, Field::NM_A_FA_RESULTS ) )
	{
		container = static_cast<Field::MultiField *>( *it );
		ContactDetails cd = extractUserDetails( container );
		emit gotContactUserDetails( cd );
	}
	
	return true;
}

ContactDetails GetDetailsTask::extractUserDetails(Field::MultiField * details )
{
	ContactDetails cd;
	cd.status = GroupWise::Invalid;
	cd.archive = false;
	Field::FieldList fields = details->fields();
	// TODO: not sure what this means, ask Mike
	Field::SingleField * sf;
	if ( ( sf = fields.findSingleField ( Field::NM_A_SZ_AUTH_ATTRIBUTE ) ) )
		cd.authAttribute = sf->value().toString();
	if ( ( sf = fields.findSingleField ( Field::NM_A_SZ_DN ) ) )
		cd.dn =sf->value().toString().toLower(); // HACK: lowercased DN
	if ( ( sf = fields.findSingleField ( Field::KOPETE_NM_USER_DETAILS_CN ) ) )
		cd.cn = sf->value().toString();
	if ( ( sf = fields.findSingleField ( Field::KOPETE_NM_USER_DETAILS_GIVEN_NAME ) ) )
		cd.givenName = sf->value().toString();
	if ( ( sf = fields.findSingleField ( Field::KOPETE_NM_USER_DETAILS_SURNAME ) ) )
		cd.surname = sf->value().toString();
	if ( ( sf = fields.findSingleField ( Field::KOPETE_NM_USER_DETAILS_ARCHIVE_FLAG ) ) )
		cd.archive = ( sf->value().toInt() == 1 );
	if ( ( sf = fields.findSingleField ( Field::KOPETE_NM_USER_DETAILS_FULL_NAME ) ) )
		cd.fullName = sf->value().toString();
	if ( ( sf = fields.findSingleField ( Field::NM_A_SZ_STATUS ) ) )
		cd.status = sf->value().toInt();
	if ( ( sf = fields.findSingleField ( Field::NM_A_SZ_MESSAGE_BODY ) ) )
		cd.awayMessage = sf->value().toString();
	Field::MultiField * mf;
	//TODO: use Multi
	QMap< QString, QVariant > propMap;
	if ( ( mf = fields.findMultiField ( Field::NM_A_FA_INFO_DISPLAY_ARRAY ) ) )
	{
		Field::FieldList fl = mf->fields();
		const Field::FieldListIterator end = fl.end();
		for ( Field::FieldListIterator it = fl.begin(); it != end; ++it )
		{
			Field::SingleField * propField = dynamic_cast<Field::SingleField *>( *it );
			if ( propField ) {
				QString propName = propField->tag();
				QString propValue = propField->value().toString();
				propMap.insert( propName, propValue );
			} else {
				Field::MultiField * mf2;
				if ( ( mf2 = dynamic_cast<Field::MultiField *>( *it ) ) ) {
					Field::FieldList fl2 = mf2->fields();
					const Field::FieldListIterator end = fl2.end();
					for ( Field::FieldListIterator it2 = fl2.begin(); it2 != end; ++it2 )
					{
						propField = dynamic_cast<Field::SingleField *>( *it2 );
						if ( propField ) {
							QString propName = propField->tag();
							QString propValue = propField->value().toString();
							propMap.insert( propName, propValue );
						}
					}
				}
			}
		}
	}
	if ( !propMap.empty() )
	{
		cd.properties = propMap;
	}
	return cd;
}
#include "getdetailstask.moc"
