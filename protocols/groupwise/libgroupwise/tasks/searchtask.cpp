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

SearchTask::SearchTask(Task* parent): RequestTask(parent), m_polls( 0 )
{
}


SearchTask::~SearchTask()
{
}

void SearchTask::search( const QValueList<UserSearchQueryTerm> & query )
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
	QValueList<UserSearchQueryTerm>::ConstIterator it = query.begin();
	const QValueList<UserSearchQueryTerm>::ConstIterator end = query.end();
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
	QTimer::singleShot( 1000, this, SLOT( slotPollForResults() ) );
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
			if ( m_polls < 5 ) // restart timer
				QTimer::singleShot( 10000, this, SLOT( slotPollForResults() ) );
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

QValueList< GroupWise::ContactDetails > SearchTask::results()
{
	return m_results;
}

#include "searchtask.moc"
