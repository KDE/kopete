/*
    Kopete Groupwise Protocol
    createcontactinstancetask.h - Request Task that creates an instance of a contact on the server side contact list

    Copyright (c) 2004      SUSE Linux AG	 	 http://www.suse.com
    
    Based on Iris, Copyright (C) 2003  Justin Karneges

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

#include "createcontactinstancetask.h"

CreateContactInstanceTask::CreateContactInstanceTask(Task* parent): ModifyContactListTask(parent)
{
}


CreateContactInstanceTask::~CreateContactInstanceTask()
{
}

void CreateContactInstanceTask::contactFromUserId( const QString & userId, const QString & displayName, const int parentFolder )
{
	contact( new Field::SingleField( NM_A_SZ_USERID, 0, NMFIELD_TYPE_UTF8, userId ), displayName, parentFolder );
}

void CreateContactInstanceTask::contactFromDN( const QString & dn, const QString & displayName, const int parentFolder )
{
	contact( new Field::SingleField( NM_A_SZ_DN, 0, NMFIELD_TYPE_UTF8, dn ), displayName, parentFolder );
}

void CreateContactInstanceTask::contact( Field::SingleField * id, const QString & displayName, const int parentFolder )
{
	Field::FieldList lst;
	lst.append( new Field::SingleField( NM_A_SZ_PARENT_ID, 0, NMFIELD_TYPE_UTF8, QString::number( parentFolder ) ) );
	// this is either a user Id or a DN
	lst.append( id );
	if ( !displayName.isEmpty() )
		lst.append( new Field::SingleField( NM_A_SZ_DISPLAY_NAME, 0, NMFIELD_TYPE_UTF8, displayName ) );
	createTransfer( "createcontact", lst );
}

#include "createcontactinstancetask.moc"
