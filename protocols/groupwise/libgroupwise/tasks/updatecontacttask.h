/*
    Kopete Groupwise Protocol
    updatecontacttask.h - rename a contact on the server

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

#ifndef UPDATECONTACTTASK_H
#define UPDATECONTACTTASK_H

#include "gwerror.h"

#include "updateitemtask.h"

/**
 * Renames a contact on the server
 * @author Kopete Developers
 */
class UpdateContactTask : public UpdateItemTask
{
Q_OBJECT
public:
	UpdateContactTask(Task* parent);
	~UpdateContactTask();
	void renameContact( const QString& newName, const QValueList<GroupWise::ContactItem> & contactInstances );
	QString displayName();
private:
	QString m_name;
};

#endif
