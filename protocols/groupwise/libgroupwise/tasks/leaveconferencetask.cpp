//
// C++ Implementation: leaveconferencetask
//
// Description: 
//
//
// Author: SUSE AG <>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "leaveconferencetask.h"

LeaveConferenceTask::LeaveConferenceTask(Task* parent): RequestTask(parent)
{
}


LeaveConferenceTask::~LeaveConferenceTask()
{
}

void LeaveConferenceTask::leave( const QString & guid )
{
	Field::FieldList lst, tmp;
	tmp.append( new Field::SingleField( NM_A_SZ_OBJECT_ID, 0, NMFIELD_TYPE_UTF8, guid ) );
	lst.append( new Field::MultiField( NM_A_FA_CONVERSATION, NMFIELD_METHOD_VALID, 0, NMFIELD_TYPE_ARRAY, tmp ) );
	createTransfer( "leaveconf", lst );
}

#include "leaveconferencetask.moc"
