//
// C++ Implementation: updateitemtask
//
// Description: 
//
//
// Author: SUSE AG <>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "updateitemtask.h"

UpdateItemTask::UpdateItemTask( Task* parent) : RequestTask( parent )
{
}


UpdateItemTask::~UpdateItemTask()
{
}

void UpdateItemTask::item( Field::FieldList updateItemFields )
{
	Field::FieldList lst;
	lst.append( new Field::MultiField( NM_A_FA_CONTACT_LIST, NMFIELD_METHOD_VALID, 0, NMFIELD_TYPE_ARRAY, updateItemFields ) );
	createTransfer( "updateitem", lst );
}

#include "updateitemtask.moc"
