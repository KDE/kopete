//
// C++ Implementation: DeleteItemTask
//
// Description: 
//
//
// Author: SUSE AG <>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "deleteitemtask.h"

DeleteItemTask::DeleteItemTask(Task* parent): ModifyContactListTask(parent)
{
}


DeleteItemTask::~DeleteItemTask()
{
}

void DeleteItemTask::item( const int parentFolder, const int objectId )
{
	Field::FieldList lst;
	lst.append( new Field::SingleField( NM_A_SZ_PARENT_ID, 0, NMFIELD_TYPE_UTF8, QString::number( parentFolder ) ) );
	// this is either a user Id or a DN
	lst.append( new Field::SingleField( NM_A_SZ_OBJECT_ID, 0, NMFIELD_TYPE_UTF8, QString::number( objectId ) ) );
	createTransfer( "deletecontact", lst );
}

#include "deleteitemtask.moc"
