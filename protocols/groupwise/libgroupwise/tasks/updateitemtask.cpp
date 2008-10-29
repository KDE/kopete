/*
    Kopete Groupwise Protocol
    updateitemtask.cpp - ancestor for tasks that rename objects on the server

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
	lst.append( new Field::MultiField( Field::NM_A_FA_CONTACT_LIST, NMFIELD_METHOD_VALID, 0, NMFIELD_TYPE_ARRAY, updateItemFields ) );
	createTransfer( "updateitem", lst );
}

#include "updateitemtask.moc"
