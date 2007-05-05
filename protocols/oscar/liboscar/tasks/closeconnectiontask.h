/*
    Kopete Oscar Protocol
    closeconnectiontask.h - Handles the closing of the connection to the server

    Copyright (c) 2004 Matt Rogers <mattr@kde.org>
    Copyright (c) 2007 Roman Jarosz <kedgedev@centrum.cz>

    Kopete (c) 2002-2007 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef CLOSECONNECTIONTASK_H
#define CLOSECONNECTIONTASK_H

#include <task.h>

class Transfer;

/**
@author Matt Rogers
*/
class CloseConnectionTask : public Task
{
public:
	CloseConnectionTask(Task* parent);
	
	~CloseConnectionTask();
	
	virtual bool take(Transfer* transfer);

protected:
	virtual bool forMe(const Transfer* transfer) const;
	virtual void onGo();

};

#endif

//kate: indent-mode csands; space-indent off; tab-width 4; replace-tabs off;
