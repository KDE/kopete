//
// C++ Implementation: modifycontactlisttask
//
// Description: 
//
//
// Author: SUSE AG <>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "modifycontactlisttask.h"

ModifyContactListTask::ModifyContactListTask(Task* parent): RequestTask(parent)
{
}

ModifyContactListTask::~ModifyContactListTask()
{
}

bool ModifyContactListTask::take( Transfer * transfer )
{
	qDebug( "ModifyContactListTask::take() NOT IMPLEMENTED" );
	return false;
}

#include "modifycontactlisttask.moc"
