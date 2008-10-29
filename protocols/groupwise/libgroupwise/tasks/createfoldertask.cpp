/*
    Kopete Groupwise Protocol
    createfoldertask.h - Request Task for creating a single folder on the server

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
	lst.append( new Field::SingleField( Field::NM_A_SZ_PARENT_ID, 0, NMFIELD_TYPE_UTF8, QString::number( parentId ) ) );
	lst.append( new Field::SingleField( Field::NM_A_SZ_DISPLAY_NAME, 0, NMFIELD_TYPE_UTF8, displayName ) );
	lst.append( new Field::SingleField( Field::NM_A_SZ_SEQUENCE_NUMBER, 0, NMFIELD_TYPE_UTF8, QString::number( sequence ) ) );
	createTransfer( "createfolder", lst );
}

#include "createfoldertask.moc"
