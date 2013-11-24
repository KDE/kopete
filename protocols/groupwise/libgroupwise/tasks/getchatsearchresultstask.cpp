/*
    Kopete Groupwise Protocol
    getchatsearchresultstask.cpp - Poll the server to see if it has processed our search yet.

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

#include "getchatsearchresultstask.h"

#include <kdebug.h>

#include "gwfield.h"
#include "response.h"

#include "logintask.h"

using namespace GroupWise;

GetChatSearchResultsTask::GetChatSearchResultsTask(Task* parent): RequestTask(parent)
{
}


GetChatSearchResultsTask::~GetChatSearchResultsTask()
{
}

void GetChatSearchResultsTask::poll( int queryHandle )
{
	Field::FieldList lst;
	lst.append( new Field::SingleField( Field::NM_A_UD_OBJECT_ID, 0, NMFIELD_TYPE_UDWORD, queryHandle ) );
	lst.append( new Field::SingleField( Field::NM_A_UD_QUERY_COUNT, 0, NMFIELD_TYPE_UDWORD, 10 ) );
	createTransfer( "getchatsearchresults", lst );
}

bool GetChatSearchResultsTask::take( Transfer * transfer )
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
	Field::SingleField * sf = responseFields.findSingleField( Field::NM_A_UW_STATUS );
	m_queryStatus = (SearchResultCode)sf->value().toInt();
	
	Field::MultiField * resultsArray = responseFields.findMultiField( Field::NM_A_FA_RESULTS );
	if ( !resultsArray )
	{
		setError( Protocol );
		return true;
	}
	Field::FieldList matches = resultsArray->fields();
	const Field::FieldListIterator end = matches.end();
	for ( Field::FieldListIterator it = matches.find( Field::NM_A_FA_CHAT );
			it != end;
			it = matches.find( ++it, Field::NM_A_FA_CHAT ) )
	{
		Field::MultiField * mf = static_cast<Field::MultiField *>( *it );
		Field::FieldList chat = mf->fields();
		GroupWise::ChatroomSearchResult cd = extractChatDetails( chat );
		m_results.append( cd );
	}
	
	if ( m_queryStatus != DataRetrieved )
		setError( m_queryStatus );
	else
	{
//		kDebug () << " we won!";
		setSuccess( m_queryStatus );
	}
	return true;
}

QList< GroupWise::ChatroomSearchResult > GetChatSearchResultsTask::results()
{
	return m_results;
}

int GetChatSearchResultsTask::queryStatus()
{
	return m_queryStatus;
}

GroupWise::ChatroomSearchResult GetChatSearchResultsTask::extractChatDetails( Field::FieldList & fields )
{
	ChatroomSearchResult csr;
	csr.participants = 0;
	// read the supplied fields, set metadata and status.
	Field::SingleField * sf;
	if ( ( sf = fields.findSingleField ( Field::NM_A_DISPLAY_NAME ) ) )
		csr.name = sf->value().toString();
	if ( ( sf = fields.findSingleField ( Field::NM_A_CHAT_OWNER_DN ) ) )
		csr.ownerDN = sf->value().toString().toLower(); // HACK: lowercased DN
	if ( ( sf = fields.findSingleField ( Field::NM_A_UD_PARTICIPANTS ) ) )
		csr.participants = sf->value().toInt();
	
//	kDebug() << csr.name << ", " << csr.ownerDN << ", " << csr.participants;
	return csr;
}

#include "getchatsearchresultstask.moc"
