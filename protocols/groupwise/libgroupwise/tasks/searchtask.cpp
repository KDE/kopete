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
#include "searchtask.h"

SearchTask::SearchTask(Task* parent): RequestTask(parent)
{
}


SearchTask::~SearchTask()
{
}

void SearchTask::search(/*... what we are searching for and how*/)
{
	Field::FieldList lst;
	// object Id identifies the search for later reference
	//lst.append( new Field::SingleField( NM_A_SZ_OBJECT_ID, 0, NMFIELD_TYPE_UTF8, QDateTime::currentDateTime().toString() );
	//lst.append( new Field::SingleField( "Given Name", 0, NMFIELD_TYPE_UTF8, [ NMFIELD_METHOD_EQUAL | NMFIELD_METHOD_MATCHBEGIN | NMFIELD_METHOD_MATCHEND | NMFIELD_METHOD_SEARCH ], searchTerm );
	// Or "Surname", NM_A_SZ_USERID, NM_A_SZ_TITLE, NM_A_SZ_DEPARTMENT in other fields
	
	createTransfer( "createsearch", lst );
}

void SearchTask::onGo()
{
	// kick off a timer here that will begin polling for results using PollSearchResultsTask

	//send transfer
}

#include "searchtask.moc"
