//
// C++ Interface: sendinvitetask
//
// Description: 
//
//
// Author: SUSE AG <>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef SENDINVITETASK_H
#define SENDINVITETASK_H

#include <requesttask.h>

/**
This sends an invitation to a conference

@author SUSE AG
*/
class SendInviteTask : public RequestTask
{
public:
	SendInviteTask(Task* parent);
	~SendInviteTask();
	void invite();
};

#endif
