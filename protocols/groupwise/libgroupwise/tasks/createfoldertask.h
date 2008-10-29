/*
    Kopete Groupwise Protocol
    createfoldertask.h - Request Task for creating a single folder on the server

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

#ifndef CREATEFOLDERTASK_H
#define CREATEFOLDERTASK_H

#include "modifycontactlisttask.h"

/**
Creates a folder on the server

@author SUSE AG
*/
class CreateFolderTask : public ModifyContactListTask
{
Q_OBJECT
public:
	CreateFolderTask(Task* parent);
	~CreateFolderTask();
	void folder( const int parentId, const int sequence, const QString & displayName );
};

#endif
