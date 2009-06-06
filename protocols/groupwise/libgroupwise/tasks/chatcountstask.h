/*
    Kopete Groupwise Protocol
    chatcountstask.cpp - Task to update chatroom participant counts

    Copyright (c) 2005      SUSE Linux Products GmbH	 http://www.suse.com

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

#ifndef CHATCOUNTSTASK_H
#define CHATCOUNTSTASK_H

#include <qlist.h>

#include "gwerror.h"
#include "gwfield.h"

#include "requesttask.h"

/**
Get the current number of users in each chat on the server

@author SUSE Linux Products GmbH
 */
class ChatCountsTask : public RequestTask
{
	Q_OBJECT
	public:
		ChatCountsTask(Task* parent);
		~ChatCountsTask();
		bool take( Transfer * transfer );
		/**
		 * Contains a list of all the chatrooms that have participants on the server.  If a chatroom exists but is empty, this task does not return a result, so update the participants count to 0.
		 */
		QMap< QString, int > results();
	private:
		QMap< QString, int > m_results;
};

#endif
