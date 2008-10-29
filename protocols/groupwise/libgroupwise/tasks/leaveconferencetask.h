/*
    Kopete Groupwise Protocol
    leaveconferencetask.h - Tell the server we are leaving a conference 

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

#ifndef LEAVECONFERENCETASK_H
#define LEAVECONFERENCETASK_H

#include "requesttask.h"

/**
Tells the server that you are leaving a conference (closed the chatwindow)

@author SUSE AG
*/
class LeaveConferenceTask : public RequestTask
{
Q_OBJECT
public:
	LeaveConferenceTask(Task* parent);
	~LeaveConferenceTask();
	void leave( const GroupWise::ConferenceGuid & guid );
};

#endif
