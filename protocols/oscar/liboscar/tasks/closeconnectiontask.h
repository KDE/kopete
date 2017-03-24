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
	
	bool take(Transfer* transfer) Q_DECL_OVERRIDE;

protected:
	bool forMe(const Transfer* transfer) const Q_DECL_OVERRIDE;
	void onGo() Q_DECL_OVERRIDE;

};

#endif

