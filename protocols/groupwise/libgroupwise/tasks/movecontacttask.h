//
// C++ Interface: movecontacttask
//
// Description: 
//
//
// Author: SUSE AG <>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef MOVECONTACTTASK_H
#define MOVECONTACTTASK_H

#include <modifycontactlisttask.h>

/**
Moves a contact between folders on the server

@author SUSE AG
*/
class MoveContactTask : public ModifyContactListTask
{
Q_OBJECT
public:
	MoveContactTask(Task* parent);

	~MoveContactTask();

};

#endif
