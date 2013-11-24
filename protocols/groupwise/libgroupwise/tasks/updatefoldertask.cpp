/*
    Kopete Groupwise Protocol
    updatefoldertask.cpp - rename a folder on the server

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
#include "updatefoldertask.h"

#include "gwfield.h"

UpdateFolderTask::UpdateFolderTask(Task* parent): UpdateItemTask(parent)
{
}

UpdateFolderTask::~UpdateFolderTask()
{
}

void UpdateFolderTask::renameFolder( const QString & newName, const GroupWise::FolderItem & existing )
{
	Field::FieldList lst;
	// add the old version of the folder, marked delete
	lst.append( new Field::MultiField( Field::NM_A_FA_FOLDER, NMFIELD_METHOD_DELETE, 0, NMFIELD_TYPE_ARRAY, folderToFields( existing) ) );
	
	GroupWise::FolderItem renamed = existing;
	renamed.name = newName;
	// add the new version of the folder, marked add
	lst.append( new Field::MultiField( Field::NM_A_FA_FOLDER, NMFIELD_METHOD_ADD, 0, NMFIELD_TYPE_ARRAY, folderToFields( renamed ) ) );
	// let our parent class package it up as a contact list in a transfer
	UpdateItemTask::item( lst );
}

Field::FieldList UpdateFolderTask::folderToFields( const GroupWise::FolderItem & folder )
{
	Field::FieldList lst;
	lst.append( new Field::SingleField( Field::NM_A_SZ_OBJECT_ID, 0, NMFIELD_TYPE_UTF8, folder.id ) );
	lst.append( new Field::SingleField( Field::NM_A_SZ_PARENT_ID, 0, NMFIELD_TYPE_UTF8, 0 ) );
	lst.append( new Field::SingleField( Field::NM_A_SZ_TYPE, 0, NMFIELD_TYPE_UTF8, 1 ) );
	lst.append( new Field::SingleField( Field::NM_A_SZ_SEQUENCE_NUMBER, 0, NMFIELD_TYPE_UTF8, folder.sequence ) );
	if ( !folder.name.isEmpty() )
		lst.append( new Field::SingleField( Field::NM_A_SZ_DISPLAY_NAME, 0, NMFIELD_TYPE_UTF8, folder.name ) );
	return lst;
}

#include "updatefoldertask.moc"

