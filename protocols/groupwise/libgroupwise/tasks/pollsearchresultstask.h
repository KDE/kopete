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

#include "requesttask.h"

/**
Search results are polled on the server, using the search handle supplied by the client with the original query.  This is a single poll request, which if successful, will retrieve the results.  Otherwise, it will set a status code, so the SearchTask can decide whether to poll again.

@author SUSE AG
*/
class PollSearchResultsTask : public RequestTask
{
Q_OBJECT
public:
	PollSearchResultsTask(Task* parent);
	~PollSearchResultsTask();
	void poll( const QString & queryHandle);
	bool take( Transfer * transfer );
};

#endif
