//
// C++ Interface: modifycontactlisttask
//
// Description: 
//
//
// Author: SUSE AG <>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef MODIFYCONTACTLISTTASK_H
#define MODIFYCONTACTLISTTASK_H

#include "requesttask.h"

/**
This is the parent of all tasks that manipulate the contact list.  The server responds to each one in the same way, and this task contains a take() to process this response.

@author SUSE AG
*/
class ModifyContactListTask : public RequestTask
{
Q_OBJECT
public:
	ModifyContactListTask(Task* parent);
	~ModifyContactListTask();
	bool take( Transfer * transfer );
};

#endif
