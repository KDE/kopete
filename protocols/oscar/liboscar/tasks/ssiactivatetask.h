/*
  Kopete Oscar Protocol
  ssiactivatetask.h - Send the SNAC for SSI activation

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
#ifndef SSIACTIVATETASK_H
#define SSIACTIVATETASK_H

#include "task.h"

/**
A fire and forget task to send the activation SNAC for the Contact list to the server.
 
@author Matt Rogers
*/
class SSIActivateTask : public Task
{
public:
	SSIActivateTask( Task* parent );
	~SSIActivateTask();
	void onGo();
};

#endif

// kate: tab-width 4; indent-mode csands;
