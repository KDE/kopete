/*
    Kopete Groupwise Protocol
    modifycontactlisttask.cpp - Ancestor of all tasks that change the server side contact list.

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

#include "modifycontactlisttask.h"
#include "client.h"
#include "response.h"
#include "gwerror.h"

ModifyContactListTask::ModifyContactListTask(Task* parent): RequestTask(parent)
{
}

ModifyContactListTask::~ModifyContactListTask()
{
}

bool ModifyContactListTask::take( Transfer * transfer )
{
	if ( !forMe( transfer ) )
		return false;
	Response * response = dynamic_cast<Response *>( transfer );
	if ( !response )
		return false;
	client()->debug( "ModifyContactListTask::take()" );

	// scan the contact list received
	// emit each add and delete as a signal
	Field::FieldList fl = response->fields();
	fl.dump( true );
	Field::MultiField * current = fl.findMultiField( Field::NM_A_FA_RESULTS );
	if ( current )
		fl = current->fields();
	current = fl.findMultiField( Field::NM_A_FA_CONTACT_LIST );
	if ( current )
	{
		Field::FieldList contactList = current->fields();
		Field::FieldListIterator cursor = contactList.begin();
		const Field::FieldListIterator end = contactList.end();
		while ( cursor != end )
		{
			Field::MultiField * mf = dynamic_cast< Field::MultiField * >( *cursor );
			if ( mf->tag() == Field::NM_A_FA_CONTACT )
			{
				// contact change
				processContactChange( mf );
			}
			else if ( mf->tag() == Field::NM_A_FA_FOLDER )
			{
				// folder change
				processFolderChange( mf );
			}
			++cursor;
		}
	}
	// TODO: call virtual here to read any fields after the contact list...
	if ( response->resultCode() == GroupWise::None )
		setSuccess();
	else 
		setError( response->resultCode() );
	return true;
}

void ModifyContactListTask::processContactChange( Field::MultiField * container )
{
	if ( !( container->method() == NMFIELD_METHOD_ADD
			|| container->method() == NMFIELD_METHOD_DELETE ) )
		return;

	client()->debug( "ModifyContactListTask::processContactChange()" );
	Field::SingleField * current;
	Field::FieldList fl = container->fields();
	ContactItem contact;
	current = fl.findSingleField( Field::NM_A_SZ_OBJECT_ID );
	contact.id = current->value().toInt();
	current = fl.findSingleField( Field::NM_A_SZ_PARENT_ID );
	contact.parentId = current->value().toInt();
	current = fl.findSingleField( Field::NM_A_SZ_SEQUENCE_NUMBER );
	contact.sequence = current->value().toInt();
	current = fl.findSingleField( Field::NM_A_SZ_DISPLAY_NAME );
	contact.displayName = current->value().toString();
	current = fl.findSingleField( Field::NM_A_SZ_DN );
	contact.dn = current->value().toString();
	
	if ( container->method() == NMFIELD_METHOD_ADD )
		emit gotContactAdded( contact );
	else if ( container->method() == NMFIELD_METHOD_DELETE )
		emit gotContactDeleted( contact );
}

void ModifyContactListTask::processFolderChange( Field::MultiField * container )
{
	if ( !( container->method() == NMFIELD_METHOD_ADD
			|| container->method() == NMFIELD_METHOD_DELETE ) )
		return;

	client()->debug( "ModifyContactListTask::processFolderChange()" );
	FolderItem folder;
	Field::SingleField * current;
	Field::FieldList fl = container->fields();
	// object id
	current = fl.findSingleField( Field::NM_A_SZ_OBJECT_ID );
	folder.id = current->value().toInt();
	// sequence number
	current = fl.findSingleField( Field::NM_A_SZ_SEQUENCE_NUMBER );
	folder.sequence = current->value().toInt();
	// name 
	current = fl.findSingleField( Field::NM_A_SZ_DISPLAY_NAME );
	folder.name = current->value().toString();
	// parent
	current = fl.findSingleField( Field::NM_A_SZ_PARENT_ID );
	folder.parentId = current->value().toInt();
	if ( container->method() == NMFIELD_METHOD_ADD )
		emit gotFolderAdded( folder );
	else if ( container->method() == NMFIELD_METHOD_DELETE )
		emit gotFolderDeleted( folder );
	
}


#include "modifycontactlisttask.moc"
