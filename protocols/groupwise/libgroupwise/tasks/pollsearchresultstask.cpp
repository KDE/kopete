//
// C++ Implementation: pollsearchresultstask
//
// Description: 
//
//
// Author: SUSE AG <>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "gwfield.h"
#include "response.h"

#include "pollsearchresultstask.h"

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
	lst.append( new Field::SingleField( NM_A_SZ_OBJECT_ID, 0, NMFIELD_TYPE_UTF8, queryHandle ) );
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
	Field::SingleField * sf = responseFields.findSingleField( NM_A_SZ_STATUS );
	int queryStatus = sf->value().toInt();
	responseFields.dump();
	
	Field::MultiField * resultsArray = responseFields.findMultiField( NM_A_FA_RESULTS );
	if ( !resultsArray )
	{
		setError( Protocol );
		return true;
	}
	
	// first field: NM_A_SZ_STATUS contains 
	#define SEARCH_PENDING 0
	#define SEARCH_INPROGRESS 1
	#define SEARCH_COMPLETED 2
	#define SEARCH_TIMEOUT 3
	#define SEARCH_CANCELLED 4
	#define SEARCH_ERROR 5
	// set a status code if needed
	// followed by NM_A_FA_RESULTS, looks like a getdetails
	// add an accessor to get at the results list of ContactItems, probably
	
	if ( queryStatus != 2 )
		setError( queryStatus );
	else
		setSuccess();
	return true;
}

QValueList< GroupWise::ContactItem > PollSearchResultsTask::results()
{
	return m_results;
}

#include "pollsearchresultstask.moc"
