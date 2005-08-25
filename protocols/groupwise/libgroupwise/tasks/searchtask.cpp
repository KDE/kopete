/*
    Kopete Groupwise Protocol
    searchtask.cpp - high level search for users on the server - spawns PollSearchResultsTasks

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

#include <qdatetime.h>
#include <qtimer.h>
//Added by qt3to4:
#include <Q3ValueList>

#include "client.h"
#include "gwerror.h"
#include "gwfield.h"
#include "response.h"

#include "pollsearchresultstask.h"

#include "searchtask.h"

// the delay we allow the server to initially do the search
#define GW_POLL_INITIAL_DELAY 1000
// the maximum number of times to poll the server
#define GW_POLL_MAXIMUM 5
// the frequency between subsequent polls
#define GW_POLL_FREQUENCY_MS 8000

using namespace GroupWise;

SearchTask::SearchTask(Task* parent): RequestTask(parent), m_polls( 0 )
{
}


SearchTask::~SearchTask()
{
}

void SearchTask::search( const Q3ValueList<UserSearchQueryTerm> & query )
{
	m_queryHandle = QString::number( QDateTime::currentDateTime().toTime_t () );
	Field::FieldList lst;
	if ( query.isEmpty() )
	{
		setError( 1, "no query terms" );
		return;
	}
	// object Id identifies the search for later reference
	lst.append( new Field::SingleField( NM_A_SZ_OBJECT_ID, 0, NMFIELD_TYPE_UTF8, m_queryHandle ) );
	Q3ValueList<UserSearchQueryTerm>::ConstIterator it = query.begin();
	const Q3ValueList<UserSearchQueryTerm>::ConstIterator end = query.end();
	for ( ; it != end; ++it )
	{
		Field::SingleField * fld =  new Field::SingleField( (*it).field.ascii(), (*it).operation, 0, NMFIELD_TYPE_UTF8, (*it).argument );
		lst.append( fld );
	}
	//lst.append( new Field::SingleField( "Given Name", 0, NMFIELD_TYPE_UTF8, [ NMFIELD_METHOD_EQUAL | NMFIELD_METHOD_MATCHBEGIN | NMFIELD_METHOD_MATCHEND | NMFIELD_METHOD_SEARCH ], searchTerm );
	// Or "Surname", NM_A_SZ_USERID, NM_A_SZ_TITLE, NM_A_SZ_DEPARTMENT in other fields
	
	createTransfer( "createsearch", lst );
}

bool SearchTask::take( Transfer * transfer )
{
	if ( !forMe( transfer ) )
		return false;
	Response * response = dynamic_cast<Response *>( transfer );
	if ( !response )
		return false;
	if ( response->resultCode() )
	{
		kdDebug( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << "got return code in response << " << response->resultCode() << endl;
		setError( response->resultCode() );
		return true;
	}
	// now start the results poll timer
	QTimer::singleShot( GW_POLL_INITIAL_DELAY, this, SLOT( slotPollForResults() ) );
	return true;
}

void SearchTask::slotPollForResults()
{
	//create a PollSearchResultsTask
	PollSearchResultsTask * psrt = new PollSearchResultsTask( client()->rootTask() );
	psrt->poll( m_queryHandle );
	connect( psrt, SIGNAL( finished() ), SLOT( slotGotPollResults() ) );
	psrt->go( true );
}

void SearchTask::slotGotPollResults()
{
	PollSearchResultsTask * psrt = (PollSearchResultsTask *)sender();
	kdDebug( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << "status code is " << psrt->queryStatus() << endl;
	m_polls++;
	switch ( psrt->queryStatus() )
	{
		case PollSearchResultsTask::Pending:
		case PollSearchResultsTask::InProgess:
			if ( m_polls < GW_POLL_MAXIMUM ) // restart timer
				QTimer::singleShot( GW_POLL_FREQUENCY_MS, this, SLOT( slotPollForResults() ) );
			else
				setSuccess( psrt->statusCode() );
			break;
		case PollSearchResultsTask::Completed: 
			m_results = psrt->results();
			setSuccess();
			break;
		case PollSearchResultsTask::Cancelled:
			setError(psrt->statusCode() );
			break;
		case PollSearchResultsTask::Error:
			setError( psrt->statusCode() );
			break;
		case PollSearchResultsTask::TimeOut:
			setError( psrt->statusCode() );
			break;
	}
}

Q3ValueList< GroupWise::ContactDetails > SearchTask::results()
{
	return m_results;
}

#include "searchtask.moc"
