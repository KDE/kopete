//
// C++ Implementation: createcontacttask
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
	qDebug( "CreateContactTask::take() I don't take anything.");
	return false;
}

void CreateContactTask::contactFromUserId( const QString & userId, const QString & displayName, const int firstSeqNo, const QValueList< FolderItem > folders )
{
	m_userId = userId;
	m_displayName = displayName;
	m_firstSequenceNumber = firstSeqNo;
	m_folders = folders;
}

void CreateContactTask::onGo()
{
	qDebug( "CreateContactTask::onGo() - Welcome to the Create Contact Task Show!");
	QValueList<FolderItem>::ConstIterator it = m_folders.begin();
	const QValueList<FolderItem>::ConstIterator end = m_folders.end();
	
	// for each folder that isn't on the server, start off a CreateFolderTask
	// connect their gotFolderAdded signals to the client's folderReceived signal, 
	// so the client program will see these and update its folders ( which already exist locally )
	// connect it also to our slotFolderAdded where we assess if all the folders were added ok.
	for ( ; it != end; ++it )
	{
		if ( (*it).id == 0 ) // caller asserts that this isn't on the server...
		{
			int sequence = m_firstSequenceNumber++;
			qDebug( "CreateContactTask::onGo() - Creating folder %s with sequence number %u", (*it).name.ascii(), sequence );
			CreateFolderTask * cct = new CreateFolderTask( client()->rootTask() );
			cct->folder( 0, sequence, (*it).name );
			connect( cct, SIGNAL( gotFolderAdded( const FolderItem & ) ), client(), SIGNAL( folderReceived( const FolderItem & ) ) );
			connect( cct, SIGNAL( gotFolderAdded( const FolderItem & ) ), SLOT( slotFolderAdded( const FolderItem & ) ) );
			cct->go( true );
		}
	}
}

void CreateContactTask::slotFolderAdded( const FolderItem& addedFolder )
{
	qDebug( "CreateContactTask::slotFolderAdded()" );
	// check that all the folders now exist on the server
	// once they do, we can add the contact instances
	bool allAdded = true;
	QValueList<FolderItem>::Iterator it = m_folders.begin();
	const QValueList<FolderItem>::Iterator end = m_folders.end();
	for ( ; it != end; ++it )
	{
		if ( (*it).id == 0 ) // is there a folder that hasn't yet been added on the server?
		{
			// if we have an id for it, record that
			if ( (*it).name == addedFolder.name )
			{
				qDebug( "CreateContactTask::slotFolderAdded() - Folder %s was created on the server, now has objectId %i", addedFolder.name.ascii(), addedFolder.id );
				(*it).id = addedFolder.id;
			}
			else // wait for the next add
				allAdded = false;
		}
	}
	if ( allAdded ) // now add the contact instances
	{
		qDebug( "CreateContactTask::slotFolderAdded() - got all folders" );
		// one for each folder the contact is in
		for ( it = m_folders.begin(); it != end; ++it )
		{
			qDebug( "CreateContactTask::slotFolderAdded() - Creating contact %s in folder %u", m_userId.ascii(), (*it).id );
			CreateContactInstanceTask * ccit = new CreateContactInstanceTask( client()->rootTask() );
			ccit->contactFromUserId( m_userId, m_displayName, (*it).id );
			connect( ccit, SIGNAL( gotContactAdded( const ContactItem & ) ), client(), SIGNAL( contactReceived( const ContactItem & ) ) );
			connect( ccit, SIGNAL( gotContactAdded( const ContactItem & ) ), SLOT( slotContactAdded( const ContactItem & ) ) );
			ccit->go( true );
		}
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
	QValueList<FolderItem>::Iterator it = m_folders.begin();
	const QValueList<FolderItem>::Iterator end = m_folders.end();
	while ( it != end )
	{
		QValueList<FolderItem>::Iterator current = it;
		++it;
		if ( (*current).id == addedContact.parentId )
		{
			qDebug( "CreateContactTask::slotContactAdded() - Contact Instance %s was created on the server, with objectId %i in folder %i",
					addedContact.displayName.ascii(), addedContact.id, addedContact.parentId );
			m_folders.remove( current );
			break;
		}
	}
	if ( m_folders.isEmpty() )
	{
		qDebug( "CreateContactTask::slotContactAdded() - All contacts were created on the server, we're finished!" );
		setSuccess(); 
	}
}

#include "createcontacttask.moc"
