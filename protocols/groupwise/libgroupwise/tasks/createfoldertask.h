//
// C++ Interface: createfoldertask
//
// Description: 
//
//
// Author: SUSE AG <>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef CREATEFOLDERTASK_H
#define CREATEFOLDERTASK_H

#include "modifycontactlisttask.h"

/**
Creates a folder on the server

@author SUSE AG
*/
class CreateFolderTask : public ModifyContactListTask
{
Q_OBJECT
public:
	CreateFolderTask(Task* parent);
	~CreateFolderTask();

};

#endif
