//
// C++ Interface: pollsearchresultstask
//
// Description: 
//
//
// Author: SUSE AG <>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef POLLSEARCHRESULTSTASK_H
#define POLLSEARCHRESULTSTASK_H

#include <qvaluelist.h>

#include "gwerror.h"

#include "requesttask.h"

/**
Search results are polled on the server, using the search handle supplied by the client with the original query.  This is a single poll request, which if successful, will retrieve the results.  Otherwise, it will set a status code, so the SearchTask can decide whether to poll again.

@author SUSE AG
*/
class PollSearchResultsTask : public RequestTask
{
Q_OBJECT
public:
	enum SearchResultCode { Pending=0, InProgess=1, Completed=2, TimeOut=3, Cancelled=4, Error=5 };
	PollSearchResultsTask(Task* parent);
	~PollSearchResultsTask();
	void poll( const QString & queryHandle);
	bool take( Transfer * transfer );
	int queryStatus();
	QValueList< GroupWise::ContactDetails > results();
GroupWise::ContactDetails extractUserDetails( Field::FieldList & fields );
private:
	int m_queryStatus;
	QValueList< GroupWise::ContactDetails > m_results;
};

#endif
