/*
    Kopete Groupwise Protocol
    searchchattask.cpp - high level search for users on the server - spawns PollSearchResultsTasks

    Copyright (c) 2005      SUSE Linux Products GmbH	 	 http://www.suse.com
    
    Based on Iris, Copyright (C) 2003  Justin Karneges <justin@affinix.com>

    Kopete (c) 2002-2005 by the Kopete developers <kopete-devel@kde.org>
 
    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "searchchattask.h"

#include <qdatetime.h>
#include <qtimer.h>

#include "client.h"
#include "gwerror.h"
#include "gwfield.h"
#include "response.h"

#include "getchatsearchresultstask.h"


// the delay we allow the server to initially do the search
#define GW_POLL_INITIAL_DELAY 1000
// the maximum number of times to poll the server
#define GW_POLL_MAXIMUM 5
// the frequency between subsequent polls
#define GW_POLL_FREQUENCY_MS 8000

using namespace GroupWise;

SearchChatTask::SearchChatTask(Task* parent): RequestTask(parent), m_polls( 0 )
{
}


SearchChatTask::~SearchChatTask()
{
}

void SearchChatTask::search( SearchType type )
{
	Field::FieldList lst;
	// object Id identifies the search for later reference
	lst.append( new Field::SingleField( Field::NM_A_B_ONLY_MODIFIED, 0, NMFIELD_TYPE_BOOL, ( type == FetchAll ? 0 : 1 ) ) );
	createTransfer( "chatsearch", lst );
}

bool SearchChatTask::take( Transfer * transfer )
{
	if ( !forMe( transfer ) )
		return false;
	Response * response = dynamic_cast<Response *>( transfer );
	if ( !response )
		return false;
	if ( response->resultCode() )
	{
//		kDebug() << "got return code in response << " << response->resultCode();
		setError( response->resultCode() );
		return true;
	}
	Field::FieldList responseFields = response->fields();
	Field::SingleField * sf = responseFields.findSingleField( Field::NM_A_UD_OBJECT_ID );
	m_objectId = sf->value().toInt();

	// now start the results poll timer
	QTimer::singleShot( GW_POLL_INITIAL_DELAY, this, SLOT(slotPollForResults()) );
	return true;
}

void SearchChatTask::slotPollForResults()
{
	//create a PollSearchResultsTask
	GetChatSearchResultsTask * gcsrt = new GetChatSearchResultsTask( client()->rootTask() );
	gcsrt->poll( m_objectId );
	connect( gcsrt, SIGNAL(finished()), SLOT(slotGotPollResults()) );
	gcsrt->go( true );
}

void SearchChatTask::slotGotPollResults()
{
	GetChatSearchResultsTask * gcsrt = (GetChatSearchResultsTask *)sender();
//	kDebug() << "status code is " << gcsrt->queryStatus();
	m_polls++;
	switch ( gcsrt->queryStatus() )
	{
		case GetChatSearchResultsTask::GettingData:
			if ( m_polls < GW_POLL_MAXIMUM ) // restart timer
				QTimer::singleShot( GW_POLL_FREQUENCY_MS, this, SLOT(slotPollForResults()) );
			else
				setSuccess( gcsrt->statusCode() );
			break;
		case GetChatSearchResultsTask::DataRetrieved:
			// got some results, there may be more.
			m_results += gcsrt->results();
            QTimer::singleShot( 0, this, SLOT(slotPollForResults()) );
			break;
		case GetChatSearchResultsTask::Completed:
			m_results += gcsrt->results();
			setSuccess();
			break;
		case GetChatSearchResultsTask::Cancelled:
			setError(gcsrt->statusCode() );
			break;
		case GetChatSearchResultsTask::Error:
			setError( gcsrt->statusCode() );
			break;
	}
}

QList< GroupWise::ChatroomSearchResult > SearchChatTask::results()
{
	return m_results;
}

#include "searchchattask.moc"
