//
// C++ Interface: DeleteItemTask
//
// Description: 
//
//
// Author: SUSE AG <>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef DELETEITEMTASK_H
#define DELETEITEMTASK_H

#include "modifycontactlisttask.h"

/**
@author SUSE AG
*/
class DeleteItemTask : public ModifyContactListTask
{
Q_OBJECT
public:
	DeleteItemTask(Task* parent);
	~DeleteItemTask();
	void item( const int parentFolder, const int objectId );
};

#endif
