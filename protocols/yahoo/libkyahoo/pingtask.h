/*
    pingtask.h
    Send a ping to the server

    Copyright (c) 2006 André Duffeck <duffeck@kde.org>

    Kopete    (c) 2002-2006 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef PINGTASK_H
#define PINGTASK_H

#include "task.h"

/**
@author André Duffeck
*/
class PingTask : public Task
{
public:
	PingTask(Task *parent);
	~PingTask();
	
	virtual void onGo();
};

#endif
