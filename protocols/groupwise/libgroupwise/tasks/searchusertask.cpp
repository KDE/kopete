/*
    Kopete Groupwise Protocol
    searchusertask.cpp - high level search for users on the server - spawns PollSearchResultsTasks

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

#include "searchusertask.h"

#include <qdatetime.h>
#include <qtimer.h>

#include "client.h"
#include "gwerror.h"
#include "gwfield.h"
#include "response.h"

#include "pollsearchresultstask.h"

// the delay we allow the server to initially do the search
#define GW_POLL_INITIAL_DELAY 1000
// the maximum number of times to poll the server
#define GW_POLL_MAXIMUM 5
// the frequency between subsequent polls
#define GW_POLL_FREQUENCY_MS 8000

using namespace GroupWise;

SearchUserTask::SearchUserTask(Task* parent): RequestTask(parent), m_polls( 0 )
{
}


SearchUserTask::~SearchUserTask()
{
}

void SearchUserTask::search( const QList<UserSearchQueryTerm> & query )
{
	m_queryHandle = QString::number( QDateTime::currentDateTime().toTime_t () );
	Field::FieldList lst;
	if ( query.isEmpty() )
	{
		setError( 1, "no query terms" );
		return;
	}
	// object Id identifies the search for later reference
	lst.append( new Field::SingleField( Field::NM_A_SZ_OBJECT_ID, 0, NMFIELD_TYPE_UTF8, m_queryHandle ) );
	QList<UserSearchQueryTerm>::ConstIterator it = query.begin();
	const QList<UserSearchQueryTerm>::ConstIterator end = query.end();
	for ( ; it != end; ++it )
	{
		Field::SingleField * fld =  new Field::SingleField( (*it).field, (*it).operation, 0, NMFIELD_TYPE_UTF8, (*it).argument );
		lst.append( fld );
	}
	//lst.append( new Field::SingleField( "Given Name", 0, NMFIELD_TYPE_UTF8, [ NMFIELD_METHOD_EQUAL | NMFIELD_METHOD_MATCHBEGIN | NMFIELD_METHOD_MATCHEND | NMFIELD_METHOD_SEARCH ], searchTerm );
	// Or "Surname", Field::NM_A_SZ_USERID, NM_A_SZ_TITLE, NM_A_SZ_DEPARTMENT in other fields
	
	createTransfer( "createsearch", lst );
}

bool SearchUserTask::take( Transfer * transfer )
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
	// now start the results poll timer
	QTimer::singleShot( GW_POLL_INITIAL_DELAY, this, SLOT(slotPollForResults()) );
	return true;
}

void SearchUserTask::slotPollForResults()
{
	//create a PollSearchResultsTask
	PollSearchResultsTask * psrt = new PollSearchResultsTask( client()->rootTask() );
	psrt->poll( m_queryHandle );
	connect( psrt, SIGNAL(finished()), SLOT(slotGotPollResults()) );
	psrt->go( true );
}

void SearchUserTask::slotGotPollResults()
{
	PollSearchResultsTask * psrt = (PollSearchResultsTask *)sender();
//	kDebug() << "status code is " << psrt->queryStatus();
	m_polls++;
	switch ( psrt->queryStatus() )
	{
		case PollSearchResultsTask::Pending:
		case PollSearchResultsTask::InProgess:
			if ( m_polls < GW_POLL_MAXIMUM ) // restart timer
				QTimer::singleShot( GW_POLL_FREQUENCY_MS, this, SLOT(slotPollForResults()) );
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

QList< GroupWise::ContactDetails > SearchUserTask::results()
{
	return m_results;
}

#include "searchusertask.moc"
