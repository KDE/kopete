//
// C++ Interface: leaveconferencetask
//
// Description: 
//
//
// Author: SUSE AG <>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
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
	void leave( const QString & guid );
};

#endif
