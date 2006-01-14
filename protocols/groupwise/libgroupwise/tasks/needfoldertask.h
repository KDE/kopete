//
// C++ Interface: %{MODULE}
//
// Description: 
//
//
// Author: %{AUTHOR} <%{EMAIL}>, (C) %{YEAR}
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef NEEDFOLDERTASK_H
#define NEEDFOLDERTASK_H

#include "modifycontactlisttask.h"

/**
This Task is the ancestor of Tasks that may need to create a folder on the server before they can carry out their own operation.

@author Kopete Developers
*/
class NeedFolderTask : public ModifyContactListTask
{
Q_OBJECT
public:
	NeedFolderTask(Task* parent);
	~NeedFolderTask();
	void createFolder();
	virtual void onFolderCreated() = 0;
protected slots:
	void slotFolderAdded( const FolderItem & );
	void slotFolderTaskFinished();
protected:
	int m_folderSequence;
	int m_folderId;
	QString m_folderDisplayName;
};

#endif
