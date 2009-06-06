/*
    Kopete Groupwise Protocol
    pollsearchresultstask.h - Poll the server once to see if it has processed our search yet.

    Copyright (c) 2004      SUSE Linux AG	 	 http://www.suse.com
    
    Based on Iris, Copyright (C) 2003  Justin Karneges <justin@affinix.com>

    Kopete (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>
 
    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef POLLSEARCHRESULTSTASK_H
#define POLLSEARCHRESULTSTASK_H

#include <q3valuelist.h>

#include "gwerror.h"

#include "requesttask.h"

/**
Search results are polled on the server, using the search handle supplied by the client with the original query.  This is a single poll request, which if successful, will retrieve the results.  Otherwise, it will set a status code, so the ContactSearchTask can decide whether to poll again.

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
	QList< GroupWise::ContactDetails > results();
GroupWise::ContactDetails extractUserDetails( Field::FieldList & fields );
private:
	int m_queryStatus;
	QList< GroupWise::ContactDetails > m_results;
};

#endif
