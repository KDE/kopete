/*
    Kopete Groupwise Protocol
    getdetailstask.cpp - fetch a contact's details from the server

    Copyright (c) 2004      SUSE Linux AG	 	 http://www.suse.com
    
    Based on Iris, Copyright (C) 2003  Justin Karneges

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

#include "client.h"
#include "response.h"
#include "userdetailsmanager.h"

#include "getdetailstask.h"

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
		lst.append( new Field::SingleField( NM_A_SZ_DN, 0, NMFIELD_TYPE_UTF8, *it ) );
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
	for ( Field::FieldListIterator it = detailsFields.find( NM_A_FA_RESULTS );
		  it != end;
		  it = detailsFields.find( ++it, NM_A_FA_RESULTS ) )
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
	if ( ( sf = fields.findSingleField ( NM_A_SZ_AUTH_ATTRIBUTE ) ) )
		cd.authAttribute = sf->value().toString();
	if ( ( sf = fields.findSingleField ( NM_A_SZ_DN ) ) )
		cd.dn =sf->value().toString().lower(); // HACK: lowercased DN
	if ( ( sf = fields.findSingleField ( "CN" ) ) )
		cd.cn = sf->value().toString();
	if ( ( sf = fields.findSingleField ( "Given Name" ) ) )
		cd.givenName = sf->value().toString();
	if ( ( sf = fields.findSingleField ( "Surname" ) ) )
		cd.surname = sf->value().toString();
	if ( ( sf = fields.findSingleField ( "nnmArchive" ) ) )
		cd.archive = ( sf->value().toInt() == 1 );
	if ( ( sf = fields.findSingleField ( "Full Name" ) ) )
		cd.fullName = sf->value().toString();
	if ( ( sf = fields.findSingleField ( NM_A_SZ_STATUS ) ) )
		cd.status = sf->value().toInt();
	if ( ( sf = fields.findSingleField ( NM_A_SZ_MESSAGE_BODY ) ) )
		cd.awayMessage = sf->value().toString();
	Field::MultiField * mf;
	QMap< QString, QString > propMap;
	if ( ( mf = fields.findMultiField ( NM_A_FA_INFO_DISPLAY_ARRAY ) ) )
	{
		Field::FieldList fl = mf->fields();
		const Field::FieldListIterator end = fl.end();
		for ( Field::FieldListIterator it = fl.begin(); it != end; ++it )
		{
			Field::SingleField * propField = static_cast<Field::SingleField *>( *it );
			QString propName = propField->tag();
			QString propValue = propField->value().toString();
			propMap.insert( propName, propValue );
		}
	}
	if ( !propMap.empty() )
	{
		cd.properties = propMap;
	}
	return cd;
}
#include "getdetailstask.moc"
