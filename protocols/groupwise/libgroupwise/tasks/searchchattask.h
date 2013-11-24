/*
    Kopete Groupwise Protocol
    searchchattask.h - search for chatrooms on the server - spawns PollSearchResultsTasks

    Copyright (c) 2005      SUSE Linux Products GmbH	 	 http://www.suse.com
    
    Based on Iris, Copyright (C) 2003  Justin Karneges <justin@affinix.com>

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

#ifndef SEARCHCHATTASK_H
#define SEARCHCHATTASK_H

#include "gwerror.h"
#include "libgroupwise_export.h"

#include "requesttask.h"
#include "getchatsearchresultstask.h"

class QTimer;

/**
This Task searches for chatrooms on the server

@author SUSE Linux Products GmbH
 */
class LIBGROUPWISE_EXPORT SearchChatTask : public RequestTask
{
	Q_OBJECT
	public:
		enum SearchType { FetchAll=0, SinceLastSearch };
		
		SearchChatTask(Task* parent);

		~SearchChatTask();
	/**
		 * Create the search query
	 */
		void search( SearchType type );
	/** 
		 * If the query was accepted, start a timer to poll for results using PollSearchResultsTask
	 */
		virtual bool take( Transfer * transfer );
	/**
		 * Access the results of the search
	 */
		QList< GroupWise::ChatroomSearchResult > results();
	protected slots:
		void slotPollForResults();
		void slotGotPollResults();
	private: 
		QTimer * m_resultsPollTimer;
		QList< GroupWise::ChatroomSearchResult > m_results;
		int m_polls;
		int m_objectId; // used to identify our query to the server, so we can poll for its results
};

#endif
