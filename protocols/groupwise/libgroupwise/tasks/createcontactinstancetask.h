/*
    Kopete Groupwise Protocol
    createcontactinstancetask.h - Request Task that creates an instance of a contact on the server side contact list

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

#ifndef CreateContactInstanceTask_H
#define CreateContactInstanceTask_H

#include "modifycontactlisttask.h"

/**
Creates a contact on the server.  The response to this action is handled by its parent

@author SUSE AG
*/
class CreateContactInstanceTask : public ModifyContactListTask
{
Q_OBJECT
public:
	CreateContactInstanceTask(Task* parent);
	~CreateContactInstanceTask();
	/** 
	 * Sets up the request message.  
	 */
	void contactFromUserId( const QString & userId, const QString & displayName, const int parentFolder );
	void contactFromDN( const QString & dn, const QString & displayName, const int parentFolder );
protected:
	void contact( Field::SingleField * id, const QString & displayName, const int parentFolder );
};

#endif
