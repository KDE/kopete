/*
    Kopete Groupwise Protocol
    deleteitemtask.cpp - Delete a contact or folder on the server

    Copyright (c) 2004      SUSE Linux AG	 	 http://www.suse.com
    
    Based on Iris, Copyright (C) 2003  Justin Karneges <justin@affinix.com>

    Kopete (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>
 
    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "deleteitemtask.h"

DeleteItemTask::DeleteItemTask(Task* parent): ModifyContactListTask(parent)
{
}


DeleteItemTask::~DeleteItemTask()
{
}

void DeleteItemTask::item( const int parentFolder, const int objectId )
{
	if ( objectId == 0 )
	{
		setError( 1, "Cannot delete the root folder" );
		return;
	}
	Field::FieldList lst;
	lst.append( new Field::SingleField( Field::NM_A_SZ_PARENT_ID, 0, NMFIELD_TYPE_UTF8, QString::number( parentFolder ) ) );
	// this is either a user Id or a DN
	lst.append( new Field::SingleField( Field::NM_A_SZ_OBJECT_ID, 0, NMFIELD_TYPE_UTF8, QString::number( objectId ) ) );
	createTransfer( "deletecontact", lst );
}

#include "deleteitemtask.moc"
