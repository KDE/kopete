//
// C++ Implementation: createfoldertask
//
// Description: 
//
//
// Author: SUSE AG <>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "createfoldertask.h"

CreateFolderTask::CreateFolderTask(Task* parent): ModifyContactListTask(parent)
{
}


CreateFolderTask::~CreateFolderTask()
{
}

void CreateFolderTask::folder( const int parentId, const int sequence, const QString & displayName )
{
	Field::FieldList lst;
	lst.append( new Field::SingleField( NM_A_SZ_PARENT_ID, 0, NMFIELD_TYPE_UTF8, QString::number( parentId ) ) );
	lst.append( new Field::SingleField( NM_A_SZ_DISPLAY_NAME, 0, NMFIELD_TYPE_UTF8, displayName ) );
	lst.append( new Field::SingleField( NM_A_SZ_SEQUENCE_NUMBER, 0, NMFIELD_TYPE_UTF8, QString::number( sequence ) ) );
	createTransfer( "createfolder", lst );
}

#include "createfoldertask.moc"
