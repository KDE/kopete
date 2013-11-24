/*
    Kopete Groupwise Protocol
    pollsearchresultstask.cpp - Poll the server to see if it has processed our search yet.

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

#include "pollsearchresultstask.h"
#include "gwfield.h"
#include "response.h"

#include "logintask.h"

#include <QtCore/QList>

using namespace GroupWise;

PollSearchResultsTask::PollSearchResultsTask(Task* parent): RequestTask(parent)
{
}


PollSearchResultsTask::~PollSearchResultsTask()
{
}

void PollSearchResultsTask::poll( const QString & queryHandle )
{
	Field::FieldList lst;
	lst.append( new Field::SingleField( Field::NM_A_SZ_OBJECT_ID, 0, NMFIELD_TYPE_UTF8, queryHandle ) );
	createTransfer( "getresults", lst );
}

bool PollSearchResultsTask::take( Transfer * transfer )
{
	if ( !forMe( transfer ) )
		return false;
	Response * response = dynamic_cast<Response *>( transfer );
	if ( !response )
		return false;
	if ( response->resultCode() )
	{
		setError( response->resultCode() );
		return true;
	}
	
	// look for the status code
	Field::FieldList responseFields = response->fields();
	Field::SingleField * sf = responseFields.findSingleField( Field::NM_A_SZ_STATUS );
	m_queryStatus = sf->value().toInt();
	
	Field::MultiField * resultsArray = responseFields.findMultiField( Field::NM_A_FA_RESULTS );
	if ( !resultsArray )
	{
		setError( Protocol );
		return true;
	}
	Field::FieldList matches = resultsArray->fields();
	const Field::FieldListIterator end = matches.end();
	for ( Field::FieldListIterator it = matches.find( Field::NM_A_FA_CONTACT );
		  it != end;
		  it = matches.find( ++it, Field::NM_A_FA_CONTACT ) )
	{
		Field::MultiField * mf = static_cast<Field::MultiField *>( *it );
		Field::FieldList contact = mf->fields();
		GroupWise::ContactDetails cd = extractUserDetails( contact );
		m_results.append( cd );
	}
	
	// first field: Field::NM_A_SZ_STATUS contains 
	#define SEARCH_PENDING 0
	#define SEARCH_INPROGRESS 1
	#define SEARCH_COMPLETED 2
	#define SEARCH_TIMEOUT 3
	#define SEARCH_CANCELLED 4
	#define SEARCH_ERROR 5
	// set a status code if needed
	// followed by Field::NM_A_FA_RESULTS, looks like a getdetails
	// add an accessor to get at the results list of ContactItems, probably
	
	if ( m_queryStatus != 2 )
		setError( m_queryStatus );
	else
		setSuccess( m_queryStatus );
	return true;
}

QList< GroupWise::ContactDetails > PollSearchResultsTask::results()
{
	return m_results;
}

int PollSearchResultsTask::queryStatus()
{
	return m_queryStatus;
}

GroupWise::ContactDetails PollSearchResultsTask::extractUserDetails( Field::FieldList & fields )
{
	ContactDetails cd;
	cd.status = GroupWise::Invalid;
	cd.archive = false;
	// read the supplied fields, set metadata and status.
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
	if ( ( sf = fields.findSingleField ( Field::KOPETE_NM_USER_DETAILS_FULL_NAME ) ) )
		cd.fullName = sf->value().toString();
	if ( ( sf = fields.findSingleField ( Field::KOPETE_NM_USER_DETAILS_ARCHIVE_FLAG ) ) )
		cd.archive = ( sf->value().toInt() == 1 );
	if ( ( sf = fields.findSingleField ( Field::NM_A_SZ_STATUS ) ) )
		cd.status = sf->value().toInt();
	if ( ( sf = fields.findSingleField ( Field::NM_A_SZ_MESSAGE_BODY ) ) )
		cd.awayMessage = sf->value().toString();
	Field::MultiField * mf;
	QMap< QString, QVariant > propMap;
	if ( ( mf = fields.findMultiField ( Field::NM_A_FA_INFO_DISPLAY_ARRAY ) ) )
	{
		Field::FieldList fl = mf->fields();
		const Field::FieldListIterator end = fl.end();
		for ( Field::FieldListIterator it = fl.begin(); it != end; ++it )
		{
			// assumes each property only present once
			// check in logintask.cpp and if it's a multi field,
			// get the value of this instance, check if it's already in the property map and append if found.
			Field::SingleField * propField = dynamic_cast<Field::SingleField *>( *it );
			if ( propField )
			{
				QString propName = propField->tag();
				QString propValue = propField->value().toString();
				propMap.insert( propName, propValue );
			}
			else
			{
				Field::MultiField * propList = dynamic_cast<Field::MultiField*>( *it );
				if ( propList )
				{
					QString parentName = propList->tag();
					Field::FieldList propFields = propList->fields();
					const Field::FieldListIterator end = propFields.end();
					for ( Field::FieldListIterator it = propFields.begin(); it != end; ++it )
					{
						propField = dynamic_cast<Field::SingleField *>( *it );
						if ( propField )
						{
							QString propValue = propField->value().toString();
							QString contents = propMap[ propField->tag() ].toString();
							if ( !contents.isEmpty() )
								contents.append( ", " );
							contents.append( propField->value().toString());
							propMap.insert( propField->tag(), contents );
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

#include "pollsearchresultstask.moc"
