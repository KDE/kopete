/*
    Kopete Groupwise Protocol
    connectiontask.h - Event Handling task responsible for all connection related events

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

#ifndef CONNECTIONTASK_H
#define CONNECTIONTASK_H

#include "eventtask.h"

/**
This task monitors connection related events, currently 'connected elsewhere' disconnects and server disconnect notification.

@author Kopete Developers
*/
class ConnectionTask : public EventTask
{
Q_OBJECT
public:
	ConnectionTask(Task* parent);
	~ConnectionTask();
	bool take( Transfer * transfer );
signals:
	void connectedElsewhere();
	void serverDisconnect();
};

#endif
