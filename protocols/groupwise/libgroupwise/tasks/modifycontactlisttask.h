/*
    Kopete Groupwise Protocol
    modifycontactlisttask.h - Ancestor of all tasks that change the server side contact list.

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

#ifndef MODIFYCONTACTLISTTASK_H
#define MODIFYCONTACTLISTTASK_H

#include "libgroupwise_export.h"

#include "requesttask.h"

/**
This is the parent of all tasks that manipulate the contact list.  The server responds to each one in the same way, and this task contains a take() to process this response.

@author SUSE AG
*/

using namespace GroupWise;

class LIBGROUPWISE_EXPORT ModifyContactListTask : public RequestTask
{
Q_OBJECT
public:
	ModifyContactListTask(Task* parent);
	~ModifyContactListTask();
	bool take( Transfer * transfer );
signals:
	void gotFolderAdded( const FolderItem &);
	void gotFolderDeleted( const FolderItem &  );
	void gotContactAdded( const ContactItem & );
	void gotContactDeleted( const ContactItem & );
private:
	void processFolderChange( Field::MultiField * container );
	void processContactChange( Field::MultiField * container );
};

#endif
