//
// C++ Interface: updateitemtask
//
// Description: 
//
//
// Author: SUSE AG <>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef UPDATEITEMTASK_H
#define UPDATEITEMTASK_H

#include "requesttask.h"

/**
Rename a folder or contact on the server.  In future may be used for changing the order of folders or contacts relative to one another, but this is not supported by Kopete yet.

@author SUSE AG
*/
class UpdateItemTask : public RequestTask
{
Q_OBJECT
public:
	UpdateItemTask( Task* parent );
	~UpdateItemTask();
	
};

#endif
