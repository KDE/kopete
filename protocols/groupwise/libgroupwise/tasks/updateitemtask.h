/*
    Kopete Groupwise Protocol
    updateitemtask.h - ancestor for tasks that rename objects on the server

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

#ifndef UPDATEITEMTASK_H
#define UPDATEITEMTASK_H

#include "requesttask.h"

/**
Rename a folder or contact on the server.  In future may be used for changing the order of folders or contacts relative to one another, but this is not supported by Kopete yet.

@author SUSE AG
*/
class UpdateItemTask : public RequestTask
{
Q_OBJECT
public:
	UpdateItemTask( Task* parent );
	~UpdateItemTask();
	void item( Field::FieldList updateItemFields );
};

#endif
