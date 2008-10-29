/*
    Kopete Groupwise Protocol
    deleteitemtask.h - Delete a contact or folder on the server

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

#ifndef DELETEITEMTASK_H
#define DELETEITEMTASK_H

#include "libgroupwise_export.h"
#include "modifycontactlisttask.h"

/**
@author SUSE AG
*/
class LIBGROUPWISE_EXPORT DeleteItemTask : public ModifyContactListTask
{
Q_OBJECT
public:
	DeleteItemTask(Task* parent);
	~DeleteItemTask();
	void item( const int parentFolder, const int objectId );
};

#endif
