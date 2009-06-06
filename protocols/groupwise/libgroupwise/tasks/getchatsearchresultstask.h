/*
    Kopete Groupwise Protocol
    getchatsearchresultstask.h - Poll the server once to see if it has processed our chatroom search yet.

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

#ifndef CHATSEARCHRESULTSTASK_H
#define CHATSEARCHRESULTSTASK_H

#include <QList>

#include "gwchatrooms.h"

#include "requesttask.h"

/**
Search results are polled on the server, using the search handle returned by the server with the original query.  This is a single poll request, which if successful, will retrieve the results.  Otherwise, it will set a status code, so the SearchChatTask can decide whether to poll again.

@author SUSE Linux Products GmbH
 */
class GetChatSearchResultsTask : public RequestTask
{
	Q_OBJECT
	public:
		enum SearchResultCode { Completed=2, Cancelled=4, Error=5, GettingData=8, DataRetrieved=9 };
		GetChatSearchResultsTask(Task* parent);
		~GetChatSearchResultsTask();
		void poll( int queryHandle);
		bool take( Transfer * transfer );
		int queryStatus();
		QList< GroupWise::ChatroomSearchResult > results();
	private:
		GroupWise::ChatroomSearchResult extractChatDetails( Field::FieldList & fields );
		SearchResultCode m_queryStatus;
		QList< GroupWise::ChatroomSearchResult > m_results;
};

#endif
