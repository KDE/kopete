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

#include "logintask.h"

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
	m_queryStatus = sf->value().toInt();
	
	Field::MultiField * resultsArray = responseFields.findMultiField( NM_A_FA_RESULTS );
	if ( !resultsArray )
	{
		setError( Protocol );
		return true;
	}
	Field::FieldList matches = resultsArray->fields();
	const Field::FieldListIterator end = matches.end();
	for ( Field::FieldListIterator it = matches.find( NM_A_FA_CONTACT );
		  it != end;
		  it = matches.find( ++it, NM_A_FA_CONTACT ) )
	{
		Field::MultiField * mf = static_cast<Field::MultiField *>( *it );
		Field::FieldList contact = mf->fields();
		GroupWise::ContactDetails cd = extractUserDetails( contact );
		m_results.append( cd );
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
	
	if ( m_queryStatus != 2 )
		setError( m_queryStatus );
	else
		setSuccess( m_queryStatus );
	return true;
}

QValueList< GroupWise::ContactDetails > PollSearchResultsTask::results()
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
	// read the supplied fields, set metadata and status.
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

#include "pollsearchresultstask.moc"
