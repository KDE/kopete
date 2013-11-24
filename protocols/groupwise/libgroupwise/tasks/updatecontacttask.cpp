/*
    Kopete Groupwise Protocol
    updatecontacttask.cpp - rename a contact on the server

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

#include "updatecontacttask.h"
#include "gwfield.h" 

#include <QtCore/QList>

using namespace GroupWise; 

UpdateContactTask::UpdateContactTask(Task* parent): UpdateItemTask(parent)
{
}


UpdateContactTask::~UpdateContactTask()
{
}

QString UpdateContactTask::displayName()
{
	return m_name;
}

void UpdateContactTask::renameContact( const QString & newName, const QList<ContactItem> & contactInstances )
{
	m_name = newName;
	// build a list of delete, add fields that removes each instance on the server and then readds it with the new name
	Field::FieldList lst;
	const QList<ContactItem>::ConstIterator end = contactInstances.end();
	for( QList<ContactItem>::ConstIterator it = contactInstances.begin(); it != end; ++it )
	{
		Field::FieldList contactFields;
		contactFields.append( new Field::SingleField( Field::NM_A_SZ_OBJECT_ID, 0, NMFIELD_TYPE_UTF8, (*it).id ) );
		contactFields.append( new Field::SingleField( Field::NM_A_SZ_PARENT_ID, 0, NMFIELD_TYPE_UTF8, (*it).parentId ) );
		contactFields.append( new Field::SingleField( Field::NM_A_SZ_SEQUENCE_NUMBER, 0, NMFIELD_TYPE_UTF8, (*it).sequence ) );
		if ( !(*it).dn.isNull() )
			contactFields.append( new Field::SingleField( Field::NM_A_SZ_DN, 0, NMFIELD_TYPE_UTF8, (*it).dn ) );
		if ( !(*it).displayName.isNull() )
			contactFields.append( new Field::SingleField( Field::NM_A_SZ_DISPLAY_NAME, 0, NMFIELD_TYPE_UTF8, (*it).displayName ) );
		lst.append( 
			new Field::MultiField( Field::NM_A_FA_CONTACT, NMFIELD_METHOD_DELETE, 0, NMFIELD_TYPE_ARRAY, contactFields ) );
	}
	for( QList<ContactItem>::ConstIterator it = contactInstances.begin(); it != end; ++it )
	{
		Field::FieldList contactFields;
		contactFields.append( new Field::SingleField( Field::NM_A_SZ_OBJECT_ID, 0, NMFIELD_TYPE_UTF8, (*it).id ) );
		contactFields.append( new Field::SingleField( Field::NM_A_SZ_PARENT_ID, 0, NMFIELD_TYPE_UTF8, (*it).parentId ) );
		contactFields.append( new Field::SingleField( Field::NM_A_SZ_SEQUENCE_NUMBER, 0, NMFIELD_TYPE_UTF8, (*it).sequence ) );
		if ( !(*it).dn.isNull() )
			contactFields.append( new Field::SingleField( Field::NM_A_SZ_DN, 0, NMFIELD_TYPE_UTF8, (*it).dn ) );
		contactFields.append( new Field::SingleField( Field::NM_A_SZ_DISPLAY_NAME, 0, NMFIELD_TYPE_UTF8, newName ) );
		lst.append( 
			new Field::MultiField( Field::NM_A_FA_CONTACT, NMFIELD_METHOD_ADD, 0, NMFIELD_TYPE_ARRAY, contactFields ) );
	}
	//lst.dump( true );
	UpdateItemTask::item( lst );
}

#include "updatecontacttask.moc"
