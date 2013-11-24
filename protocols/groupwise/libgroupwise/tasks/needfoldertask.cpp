//
// C++ Implementation: %{MODULE}
//
// Description:
//
//
// Author: %{AUTHOR} <%{EMAIL}>, (C) %{YEAR}
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "needfoldertask.h"

#include "client.h"
#include "tasks/createcontactinstancetask.h"
#include "tasks/createfoldertask.h"

NeedFolderTask::NeedFolderTask(Task* parent): ModifyContactListTask(parent)
{
}

NeedFolderTask::~NeedFolderTask()
{
}

void NeedFolderTask::createFolder()
{
	CreateFolderTask * cct = new CreateFolderTask( client()->rootTask() );
	cct->folder( 0, m_folderSequence, m_folderDisplayName );
	connect( cct, SIGNAL(gotFolderAdded(FolderItem)), client(), SIGNAL(folderReceived(FolderItem)) );
	connect( cct, SIGNAL(gotFolderAdded(FolderItem)), SLOT(slotFolderAdded(FolderItem)) );
	connect( cct, SIGNAL(finished()), SLOT(slotFolderTaskFinished()) );
	cct->go( true );
}

void NeedFolderTask::slotFolderAdded( const FolderItem & addedFolder )
{
	// if this is the folder we were trying to create
	if ( m_folderDisplayName == addedFolder.name )
	{
		client()->debug( QString( "NeedFolderTask::slotFolderAdded() - Folder %1 was created on the server, now has objectId %2" ).arg( addedFolder.name ).arg( addedFolder.id ) );
		m_folderId = addedFolder.id;
	}
}

void NeedFolderTask::slotFolderTaskFinished()
{
	CreateFolderTask *cct = ( CreateFolderTask* )sender();
	if ( cct->success() )
	{
		// call our child class's action to be performed
		onFolderCreated();
	}
	else
		setError( 1, "Folder creation failed" );
}

#include "needfoldertask.moc"
