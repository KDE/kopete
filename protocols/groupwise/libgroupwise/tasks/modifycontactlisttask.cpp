//
// C++ Implementation: modifycontactlisttask
//
// Description: 
//
//
// Author: SUSE AG <>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "client.h"
#include "response.h"

#include "modifycontactlisttask.h"

ModifyContactListTask::ModifyContactListTask(Task* parent): RequestTask(parent)
{
}

ModifyContactListTask::~ModifyContactListTask()
{
}

bool ModifyContactListTask::take( Transfer * transfer )
{
	qDebug( "ModifyContactListTask::take()" );
	if ( !forMe( transfer ) )
		return false;
	Response * response = dynamic_cast<Response *>( transfer );
	if ( !response )
		return false;

	// scan the contact list received
	// emit each add and delete as a signal
	Field::FieldList fl = response->fields();
	fl.dump( true );
	Field::FieldListIterator it = fl.begin();
	Field::FieldListIterator end = fl.end();
	Field::MultiField * current = fl.findMultiField( NM_A_FA_RESULTS );
	if ( current )
		fl = current->fields();
	current = fl.findMultiField( NM_A_FA_CONTACT_LIST );
	if ( current )
	{
		Field::FieldList contactList = current->fields();
		Field::FieldListIterator cursor = contactList.begin();
		const Field::FieldListIterator end = contactList.end();
		while ( cursor != end )
		{
			Field::MultiField * mf = dynamic_cast< Field::MultiField * >( *cursor );
			if ( mf->tag() == NM_A_FA_CONTACT )
			{
				// contact change
				processContactChange( mf );
			}
			else if ( mf->tag() == NM_A_FA_FOLDER )
			{
				// folder change
				processFolderChange( mf );
			}
			++cursor;
		}
	}
	// TODO: call virtual here to read any fields after the contact list...
	return true;
}

void ModifyContactListTask::processContactChange( Field::MultiField * container )
{
	qDebug( "ModifyContactListTask::processContactChange()" );
	Field::SingleField * current;
	Field::FieldList fl = container->fields();
	ContactItem contact;
	current = fl.findSingleField( NM_A_SZ_OBJECT_ID );
	contact.id = current->value().toInt();
	current = fl.findSingleField( NM_A_SZ_PARENT_ID );
	contact.parentId = current->value().toInt();
	current = fl.findSingleField( NM_A_SZ_SEQUENCE_NUMBER );
	contact.sequence = current->value().toInt();
	current = fl.findSingleField( NM_A_SZ_DISPLAY_NAME );
	contact.displayName = current->value().toString();
	if ( container->method() == NMFIELD_METHOD_ADD )
		emit gotContactAdded( contact );
	else if ( container->method() == NMFIELD_METHOD_DELETE )
		emit gotContactDeleted( contact );
}

void ModifyContactListTask::processFolderChange( Field::MultiField * container )
{
	qDebug( "ModifyContactListTask::processFolderChange()" );
	FolderItem folder;
	Field::SingleField * current;
	Field::FieldList fl = container->fields();
	// object id
	current = fl.findSingleField( NM_A_SZ_OBJECT_ID );
	folder.id = current->value().toInt();
	// sequence number
	current = fl.findSingleField( NM_A_SZ_SEQUENCE_NUMBER );
	folder.sequence = current->value().toInt();
	// name 
	current = fl.findSingleField( NM_A_SZ_DISPLAY_NAME );
	folder.name = current->value().toString();
	// parent
	current = fl.findSingleField( NM_A_SZ_PARENT_ID );
	folder.parentId = current->value().toInt();
	if ( container->method() == NMFIELD_METHOD_ADD )
		emit gotFolderAdded( folder );
	else if ( container->method() == NMFIELD_METHOD_DELETE )
		emit gotFolderDeleted( folder );
	
}


#include "modifycontactlisttask.moc"
