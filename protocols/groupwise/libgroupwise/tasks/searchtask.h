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

class QTimer;

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
	 * @param query a list of search terms
	 */
	void search( const QValueList<GroupWise::UserSearchQueryTerm> & query);
	/** 
	 * If the query was accepted, start a timer to poll for results using PollSearchResultsTask
	 */
	virtual bool take( Transfer * transfer );
	/**
	 * Access the results of the search
	 */
	QValueList< GroupWise::ContactDetails > results();
protected slots:
	void slotPollForResults();
	void slotGotPollResults();
private: 
	QString m_queryHandle;  // used to identify our query to the server, so we can poll for its results
	QTimer * m_resultsPollTimer;
	QValueList< GroupWise::ContactDetails > m_results;
	int m_polls;
};

#endif
