/*
    Kopete Groupwise Protocol
    statustask.h - Event handling task responsible for status change events

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

#ifndef STATUSTASK_H
#define STATUSTASK_H

#include "eventtask.h"

/**
@author Kopete Developers
*/
class StatusTask : public EventTask
{
Q_OBJECT
public:
	StatusTask(Task* parent);
	~StatusTask();
	bool take( Transfer * transfer );
signals:
	void gotStatus( const QString & contactId, quint16 status, const QString & statusText );
};

#endif
