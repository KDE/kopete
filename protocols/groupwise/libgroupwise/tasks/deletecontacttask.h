//
// C++ Interface: deletecontacttask
//
// Description: 
//
//
// Author: SUSE AG <>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef DELETECONTACTTASK_H
#define DELETECONTACTTASK_H

#include "modifycontactlisttask.h"

/**
@author SUSE AG
*/
class DeleteContactTask : public ModifyContactListTask
{
Q_OBJECT
public:
	DeleteContactTask(Task* parent);

	~DeleteContactTask();

};

#endif
