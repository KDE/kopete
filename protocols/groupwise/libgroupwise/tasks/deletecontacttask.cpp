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

void DeleteContactTask::contact( const int parentFolder, const int objectId )
{
	Field::FieldList lst;
	lst.append( new Field::SingleField( NM_A_SZ_PARENT_ID, 0, NMFIELD_TYPE_UTF8, QString::number( parentFolder ) ) );
	// this is either a user Id or a DN
	lst.append( new Field::SingleField( NM_A_SZ_OBJECT_ID, 0, NMFIELD_TYPE_UTF8, QString::number( parentFolder ) ) );
	createTransfer( "deletecontact", lst );
}

#include "deletecontacttask.moc"
