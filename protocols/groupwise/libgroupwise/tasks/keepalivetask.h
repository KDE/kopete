/*
    Kopete Groupwise Protocol
    keepalivetask.h - Send keepalive pings to the server

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

#ifndef KEEPALIVETASK_H
#define KEEPALIVETASK_H

#include "requesttask.h"

/**
@author Kopete Developers
*/
class KeepAliveTask : public RequestTask
{
Q_OBJECT
public:
	KeepAliveTask(Task* parent);
	~KeepAliveTask();
	void setup();
};

#endif
