/*
    Kopete Groupwise Protocol
    createcontacttask.cpp - high level task responsible for creating both a contact and any folders it belongs to locally, on the server

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

#include "client.h"
#include "createfoldertask.h"
#include "createcontactinstancetask.h"

#include "createcontacttask.h"

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

void CreateContactTask::contactFromUserId( const QString & userId, const QString & displayName, const int firstSeqNo, const QValueList< FolderItem > folders, bool topLevel )
{
	m_userId = userId;
	m_displayName = displayName;
	m_firstSequenceNumber = firstSeqNo;
	m_folders = folders;
	m_topLevel = topLevel;
}

void CreateContactTask::onGo()
{
	qDebug( "CreateContactTask::onGo() - Welcome to the Create Contact Task Show!");
	QValueList<FolderItem>::ConstIterator it = m_folders.begin();
	const QValueList<FolderItem>::ConstIterator end = m_folders.end();
	
	// create contacts on the server
	for ( ; it != end; ++it )
	{
		qDebug( " - contact is in folder %s with id %i", (*it).name.ascii(), (*it).id );
		CreateContactInstanceTask * ccit = new CreateContactInstanceTask( client()->rootTask() );
		// the add contact action may cause other contacts' sequence numbers to change
		// CreateContactInstanceTask signals these changes, so we propagate the signal via the Client, to the GroupWiseAccount
		// This updates our local versions of those contacts using the same mechanism by which they are updated at login.
		connect( ccit, SIGNAL( gotContactAdded( const ContactItem & ) ), SLOT( slotContactAdded( const ContactItem & ) ) );
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
		qDebug( " - contact is in top level folder " );
		CreateContactInstanceTask * ccit = new CreateContactInstanceTask( client()->rootTask() );
		connect( ccit, SIGNAL( gotContactAdded( const ContactItem & ) ), SLOT( slotContactAdded( const ContactItem & ) ) );
		ccit->contactFromDN( m_userId, m_displayName, 0 );
		ccit->go( true );
	}
}

void CreateContactTask::slotContactAdded( const ContactItem & addedContact )
{
	qDebug( "CreateContactTask::slotContactAdded()" );
	// as each contact instance has been added on the server, 
	// remove the folderitem it belongs in.
	// once the list is empty, we have been successful

	if ( addedContact.displayName != m_displayName )
	{
		qDebug( " - addedContact is not the one we were trying to add, ignoring it ( Account will update it )" );
		return;
	}
/*	QValueList<FolderItem>::Iterator it = m_folders.begin();
	const QValueList<FolderItem>::Iterator end = m_folders.end();
	while ( it != end )
	{
		QValueList<FolderItem>::Iterator current = it;
		++it;
		if ( (*current).id == addedContact.parentId )
		{*/
			qDebug( "CreateContactTask::slotContactAdded() - Contact Instance %s was created on the server, with objectId %i in folder %i",
					addedContact.displayName.ascii(), addedContact.id, addedContact.parentId );
			
			if ( m_dn.isEmpty() )
				m_dn = addedContact.dn;
				
			m_folders.remove( /*current*/m_folders.begin() );
/*			break;
		}
	}*/
	
	// clear the topLevel flag once the corresponding server side entry has been successfully created
	if ( addedContact.parentId == 0 )
		m_topLevel = false;
	
	if ( m_folders.isEmpty() && !m_topLevel )
	{
		qDebug( "CreateContactTask::slotContactAdded() - All contacts were created on the server, we're finished!" );
		setSuccess(); 
	}
}

#include "createcontacttask.moc"
