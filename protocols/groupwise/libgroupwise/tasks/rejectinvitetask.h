//
// C++ Interface: rejectinvitetask
//
// Description: 
//
//
// Author: SUSE AG <>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef REJECTINVITETASK_H
#define REJECTINVITETASK_H

#include "requesttask.h"

/**
Used to reject an invitation to join a conference

@author SUSE AG
*/
class RejectInviteTask : public RequestTask
{
Q_OBJECT
public:
	RejectInviteTask(Task* parent);
	~RejectInviteTask();
	void reject( const QString & guid );

};

#endif
