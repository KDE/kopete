//
// C++ Interface: searchtask
//
// Description: 
//
//
// Author: SUSE AG <>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef SEARCHTASK_H
#define SEARCHTASK_H

#include "requesttask.h"

/**
This Task performs user searching on the server

@author SUSE AG
*/
class SearchTask : public RequestTask
{
Q_OBJECT
public:
    SearchTask(Task* parent);

    ~SearchTask();
	/**
	 * Create the search query
	 * @param are currently missing!
	 */
	void search();
	/** 
	 * Send the query and start a timer to poll for results using PollSearchResultsTask
	 */
	virtual void onGo();
	
};

#endif
