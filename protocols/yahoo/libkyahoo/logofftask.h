/*
    Kopete Yahoo Protocol
    Log off the Yahoo server

    Copyright (c) 2005 André Duffeck <duffeck@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef LOGOFFTASK_H
#define LOGOFFTASK_H

#include "task.h"


/**
@author André Duffeck
*/
class LogoffTask : public Task
{
public:
	LogoffTask(Task *parent);
	~LogoffTask();
	
	virtual void onGo();
};

#endif
