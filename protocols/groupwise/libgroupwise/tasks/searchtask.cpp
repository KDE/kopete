//
// C++ Implementation: searchtask
//
// Description: 
//
//
// Author: SUSE AG <>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <qdatetime.h>
#include <qtimer.h>

#include "client.h"
#include "gwerror.h"
#include "gwfield.h"
#include "response.h"

#include "pollsearchresultstask.h"

#include "searchtask.h"

using namespace GroupWise;

SearchTask::SearchTask(Task* parent): RequestTask(parent)
{
}


SearchTask::~SearchTask()
{
}

void SearchTask::search( const QValueList<UserSearchQueryTerm> & query )
{
	m_queryHandle = QString::number( QDateTime::currentDateTime().toTime_t () );
	Field::FieldList lst;
	// object Id identifies the search for later reference
	lst.append( new Field::SingleField( NM_A_SZ_OBJECT_ID, 0, NMFIELD_TYPE_UTF8, m_queryHandle ) );
	QValueList<UserSearchQueryTerm>::ConstIterator it = query.begin();
	const QValueList<UserSearchQueryTerm>::ConstIterator end = query.begin();
	for ( ; it != end; ++it )
	{
		UserSearchQueryTerm term = *it;
		lst.append( new Field::SingleField( term.field.ascii(), 0, NMFIELD_TYPE_UTF8, term.operation, term.argument ) );
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
		setError( response->resultCode() );
		return true;
	}
	// now start the results poll timer
	QTimer::singleShot( 1000, this, SLOT( slotPollForResults() ) );
	return true;
}

void SearchTask::slotPollForResults()
{
	//create a PollSearchResultsTask
	PollSearchResultsTask * psrt = new PollSearchResultsTask( client()->rootTask() );
	psrt->poll( m_queryHandle );
	connect( psrt, SIGNAL( finished() ), SLOT( slotGotPollResults() ) );
}

void SearchTask::slotGotPollResults()
{
	PollSearchResultsTask * psrt = (PollSearchResultsTask *)sender();
	switch ( psrt->statusCode() )
	{
		case PollSearchResultsTask::Pending:
		case PollSearchResultsTask::InProgess:
			// restart timer
			QTimer::singleShot( 1000, this, SLOT( slotPollForResults() ) );
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
#include "searchtask.moc"
