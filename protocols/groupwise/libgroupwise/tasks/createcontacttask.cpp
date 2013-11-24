/*
    Kopete Groupwise Protocol
    createcontacttask.cpp - high level task responsible for creating both a contact and any folders it belongs to locally, on the server

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

#include "createcontacttask.h"

#include "client.h"
#include "createfoldertask.h"
#include "createcontactinstancetask.h"

CreateContactTask::CreateContactTask(Task* parent): Task(parent)
{
}

CreateContactTask::~CreateContactTask()
{
}

QString CreateContactTask::userId()
{
	return m_userId;
}

QString CreateContactTask::dn()
{
	return m_dn;
}

QString CreateContactTask::displayName()
{
	return m_displayName;
}

bool CreateContactTask::take( Transfer * transfer )
{
	Q_UNUSED( transfer );
	return false;
}

void CreateContactTask::contactFromUserId( const QString & userId, const QString & displayName, const int firstSeqNo, const QList< FolderItem > folders, bool topLevel )
{
	m_userId = userId;
	m_displayName = displayName;
	m_firstSequenceNumber = firstSeqNo;
	m_folders = folders;
	m_topLevel = topLevel;
}

void CreateContactTask::onGo()
{
	client()->debug( "CreateContactTask::onGo() - Welcome to the Create Contact Task Show!");
	QList<FolderItem>::ConstIterator it = m_folders.constBegin();
	QList<FolderItem>::ConstIterator end = m_folders.constEnd();
	
	// create contacts on the server
	for ( ; it != end; ++it )
	{
		client()->debug( QString( " - contact is in folder %1 with id %2" ).arg( (*it).name ).arg( (*it).id ) );
		CreateContactInstanceTask * ccit = new CreateContactInstanceTask( client()->rootTask() );
		// the add contact action may cause other contacts' sequence numbers to change
		// CreateContactInstanceTask signals these changes, so we propagate the signal via the Client, to the GroupWiseAccount
		// This updates our local versions of those contacts using the same mechanism by which they are updated at login.
		connect( ccit, SIGNAL(gotContactAdded(ContactItem)), SLOT(slotContactAdded(ContactItem)) );
        connect( ccit, SIGNAL(finished()), SLOT(slotCheckContactInstanceCreated()) );
		if ( (*it).id == 0 ) // caller asserts that this isn't on the server...
		{
			ccit->contactFromDNAndFolder( m_userId, m_displayName, m_firstSequenceNumber++, ( *it ).name );
		}
		else
			ccit->contactFromDN( m_userId, m_displayName, (*it).id );

		ccit->go( true );
	}

	if ( m_topLevel )
	{
		client()->debug( " - contact is in top level folder " );
		CreateContactInstanceTask * ccit = new CreateContactInstanceTask( client()->rootTask() );
		connect( ccit, SIGNAL(gotContactAdded(ContactItem)), SLOT(slotContactAdded(ContactItem)) );
        connect( ccit, SIGNAL(finished()), SLOT(slotCheckContactInstanceCreated()) );
		ccit->contactFromDN( m_userId, m_displayName, 0 );
		ccit->go( true );
	}
	client()->debug( "CreateContactTask::onGo() - DONE" );
}

void CreateContactTask::slotContactAdded( const ContactItem & addedContact )
{
	client()->debug( "CreateContactTask::slotContactAdded()" );
	// as each contact instance has been added on the server, 
	// remove the folderitem it belongs in.
	// once the list is empty, we have been successful

	if ( addedContact.displayName != m_displayName )
	{
		client()->debug( " - addedContact is not the one we were trying to add, ignoring it ( Account will update it )" );
		return;
	}
	client()->debug( QString( "CreateContactTask::slotContactAdded() - Contact Instance %1 was created on the server, with objectId %2 in folder %3" ).arg
			( addedContact.displayName ).arg( addedContact.id ).arg( addedContact.parentId ) );
			
	if ( m_dn.isEmpty() )
		m_dn = addedContact.dn;
			
			
	if ( !m_folders.isEmpty() )
		m_folders.pop_back();

	// clear the topLevel flag once the corresponding server side entry has been successfully created
	if ( addedContact.parentId == 0 )
		m_topLevel = false;
	
	if ( m_folders.isEmpty() && !m_topLevel )
	{
		client()->debug( "CreateContactTask::slotContactAdded() - All contacts were created on the server, we are finished!" );
		setSuccess(); 
	}
}
void CreateContactTask::slotCheckContactInstanceCreated()
{
	CreateContactInstanceTask * ccit = ( CreateContactInstanceTask * )sender();
	if ( !ccit->success() )
	{
		setError( ccit->statusCode(), ccit->statusString() );
	}
}

#include "createcontacttask.moc"
