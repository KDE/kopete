//
// C++ Implementation: deletecontacttask
//
// Description: 
//
//
// Author: SUSE AG <>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "deletecontacttask.h"

DeleteContactTask::DeleteContactTask(Task* parent): ModifyContactListTask(parent)
{
}


DeleteContactTask::~DeleteContactTask()
{
}

#include "deletecontacttask.moc"
