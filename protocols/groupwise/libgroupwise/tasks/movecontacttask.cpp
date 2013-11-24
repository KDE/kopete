/*
    Kopete Groupwise Protocol
    movecontacttask.cpp - Move a contact between folders on the server

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

#include "movecontacttask.h"

#include "client.h"

MoveContactTask::MoveContactTask(Task* parent): NeedFolderTask(parent)
{
	// make the client tell the client app (Kopete) when we receive a contact
	connect( this, SIGNAL(gotContactAdded(ContactItem)), client(), SIGNAL(contactReceived(ContactItem)) );
}


MoveContactTask::~MoveContactTask()
{
}

void MoveContactTask::moveContact( const ContactItem & contact, const int newParent )
{
	Field::FieldList lst;
	// TODO: - write a contact_item_to_fields method and factor duplicate code like this out
	Field::FieldList contactFields;
	contactFields.append( new Field::SingleField( Field::NM_A_SZ_OBJECT_ID, 0, NMFIELD_TYPE_UTF8, contact.id ) );
	contactFields.append( new Field::SingleField( Field::NM_A_SZ_PARENT_ID, 0, NMFIELD_TYPE_UTF8, contact.parentId ) );
	contactFields.append( new Field::SingleField( Field::NM_A_SZ_SEQUENCE_NUMBER, 0, NMFIELD_TYPE_UTF8, contact.sequence ) );
	if ( !contact.dn.isNull() )
		contactFields.append( new Field::SingleField( Field::NM_A_SZ_DN, 0, NMFIELD_TYPE_UTF8, contact.dn ) );
	if ( !contact.displayName.isNull() )
		contactFields.append( new Field::SingleField( Field::NM_A_SZ_DISPLAY_NAME, 0, NMFIELD_TYPE_UTF8, contact.displayName ) );
	Field::FieldList contactList;
	contactList.append( 
		new Field::MultiField( Field::NM_A_FA_CONTACT, NMFIELD_METHOD_DELETE, 0, NMFIELD_TYPE_ARRAY, contactFields ) );

	lst.append( new Field::MultiField( Field::NM_A_FA_CONTACT_LIST, NMFIELD_METHOD_VALID, 0, NMFIELD_TYPE_ARRAY, contactList ) );

	lst.append( new Field::SingleField( Field::NM_A_SZ_SEQUENCE_NUMBER, 0, NMFIELD_TYPE_UTF8, "-1" ) );
	lst.append( new Field::SingleField( Field::NM_A_SZ_PARENT_ID, 0, NMFIELD_TYPE_UTF8, QString::number( newParent ) ) );
	createTransfer( "movecontact", lst );
}

void MoveContactTask::moveContactToNewFolder( const ContactItem & contact, const int newSequenceNumber, const QString & folderDisplayName )
{
	client()->debug("MoveContactTask::moveContactToNewFolder()" );
	m_folderSequence = newSequenceNumber;
	m_folderDisplayName = folderDisplayName;
	m_contactToMove = contact;
	
}

void MoveContactTask::onGo()
{
	// are we creating a folder first or can we just proceed as normal?
	if ( m_folderDisplayName.isEmpty() )
		RequestTask::onGo();
	else // create the folder, when the folder has been created, onFolderCreated gets called and creates the contact
		createFolder();
}

void MoveContactTask::onFolderCreated()
{
	client()->debug("MoveContactTask::onFolderCreated()" );
	moveContact( m_contactToMove, m_folderId );
	RequestTask::onGo();
}
#include "movecontacttask.moc"
