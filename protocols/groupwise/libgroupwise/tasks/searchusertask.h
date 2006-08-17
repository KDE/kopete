/*
    Kopete Groupwise Protocol
    searchusertask.h - high level search for users on the server - spawns PollSearchResultsTasks

    Copyright (c) 2005      SUSE Linux Products GmbH	 	 http://www.suse.com
    
    Based on Iris, Copyright (C) 2003  Justin Karneges

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

#ifndef SEARCHUSERTASK_H
#define SEARCHUSERTASK_H

#include "requesttask.h"

class QTimer;

/**
This Task performs user searching on the server

@author SUSE AG
*/
class SearchUserTask : public RequestTask
{
Q_OBJECT
public:
    SearchUserTask(Task* parent);

    ~SearchUserTask();
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
