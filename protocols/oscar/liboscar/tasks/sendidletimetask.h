/*
  Kopete Oscar Protocol
  sendidletimetask.h - Send the idle time to the server

  Copyright (c) 2004 Matt Rogers <mattr@kde.org>

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
#ifndef SENDIDLETIMETASK_H
#define SENDIDLETIMETASK_H

#include "task.h"
#include "oscartypes.h"

/**
Sends the idle time to the server
 
@author Matt Rogers
*/
class SendIdleTimeTask : public Task
{
public:
	SendIdleTimeTask( Task* parent );
	~SendIdleTimeTask();
	virtual void onGo();

	//! Set the idle time to send
	void setIdleTime( Oscar::DWORD );

private:
	
	Oscar::DWORD m_idleTime;
};

#endif

// kate: tab-width 4; indent-mode csands;
