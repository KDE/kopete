/*
    Kopete Groupwise Protocol
    updatecontacttask.h - rename a contact on the server

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

#ifndef UPDATECONTACTTASK_H
#define UPDATECONTACTTASK_H

#include "gwerror.h"

#include "libgroupwise_export.h"
#include "updateitemtask.h"
#include <QtCore/QList>

/**
 * Renames a contact on the server
 * @author Kopete Developers
 */
class LIBGROUPWISE_EXPORT UpdateContactTask : public UpdateItemTask
{
Q_OBJECT
public:
	UpdateContactTask(Task* parent);
	~UpdateContactTask();
	void renameContact( const QString& newName, const QList<GroupWise::ContactItem> & contactInstances );
	QString displayName();
private:
	QString m_name;
};

#endif
