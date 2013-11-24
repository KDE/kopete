/*
    Kopete Groupwise Protocol
    createcontactinstancetask.h - Request Task that creates an instance of a contact on the server side contact list

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
#include "createcontactinstancetask.h"

#include "client.h"

CreateContactInstanceTask::CreateContactInstanceTask(Task* parent) : NeedFolderTask(parent)
{
	// make the client tell the client app (Kopete) when we receive a contact
	connect( this, SIGNAL(gotContactAdded(ContactItem)), client(), SIGNAL(contactReceived(ContactItem)) );
}

CreateContactInstanceTask::~CreateContactInstanceTask()
{
}

void CreateContactInstanceTask::contactFromUserId( const QString & userId, const QString & displayName, const int parentFolder )
{
	contact( new Field::SingleField( Field::NM_A_SZ_USERID, 0, NMFIELD_TYPE_UTF8, userId ), displayName, parentFolder );
}

void CreateContactInstanceTask::contactFromUserIdAndFolder( const QString & userId, const QString & displayName, const int folderSequence, const QString & folderDisplayName )
{
	// record the user details
	m_userId = userId;
	m_displayName = displayName;
	// record the folder details
	m_folderSequence = folderSequence;
	m_folderDisplayName = folderDisplayName;
}

void CreateContactInstanceTask::contactFromDN( const QString & dn, const QString & displayName, const int parentFolder )
{
	contact( new Field::SingleField( Field::NM_A_SZ_DN, 0, NMFIELD_TYPE_UTF8, dn ), displayName, parentFolder );
}

void CreateContactInstanceTask::contactFromDNAndFolder( const QString & dn, const QString & displayName, const int folderSequence, const QString & folderDisplayName )
{
	// record the user details
	m_dn = dn;
	m_displayName = displayName;
	// record the folder details
	m_folderSequence = folderSequence;
	m_folderDisplayName = folderDisplayName;
}

void CreateContactInstanceTask::contact( Field::SingleField * id, const QString & displayName, const int parentFolder )
{
	Field::FieldList lst;
	lst.append( new Field::SingleField( Field::NM_A_SZ_PARENT_ID, 0, NMFIELD_TYPE_UTF8, QString::number( parentFolder ) ) );
	// this is either a user Id or a DN
	lst.append( id );
	if ( displayName.isEmpty() ) // fallback so that the contact is created
		lst.append( new Field::SingleField( Field::NM_A_SZ_DISPLAY_NAME, 0, NMFIELD_TYPE_UTF8, m_dn ) );
	else
		lst.append( new Field::SingleField( Field::NM_A_SZ_DISPLAY_NAME, 0, NMFIELD_TYPE_UTF8, displayName ) );
	createTransfer( "createcontact", lst );
}

void CreateContactInstanceTask::onGo()
{
	// are we creating a folder first or can we just proceed as normal?
	if ( m_folderDisplayName.isEmpty() )
		RequestTask::onGo();
	else // create the folder, when the folder has been created, onFolderCreated gets called and creates the contact
		createFolder();
}

void CreateContactInstanceTask::onFolderCreated()
{
	// now the folder exists, perform the requested type of contact instance creation
	if ( m_userId.isEmpty() )
		contact( new Field::SingleField( Field::NM_A_SZ_DN, 0, NMFIELD_TYPE_UTF8, m_dn ), m_displayName, m_folderId );
	else
		contact( new Field::SingleField( Field::NM_A_SZ_USERID, 0, NMFIELD_TYPE_UTF8, m_userId ), m_displayName, m_folderId );
	// send the transfer immediately
	RequestTask::onGo();
}

#include "createcontactinstancetask.moc"
