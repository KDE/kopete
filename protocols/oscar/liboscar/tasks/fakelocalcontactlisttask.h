/*
  Kopete Oscar Protocol

  Copyright (c) 2005 Jan Ritzerfeld <kde@bugs.jan.ritzerfeld.net>
  Copyright (c) 2004-2005 Matt Rogers <mattr@kde.org>

  Kopete (c) 2002-2005 by the Kopete developers <kopete-devel@kde.org>

  *************************************************************************
  *                                                                       *
  * This library is free software; you can redistribute it and/or         *
  * modify it under the terms of the GNU Lesser General Public            *
  * License as published by the Free Software Foundation; either          *
  * version 2 of the License, or (at your option) any later version.      *
  *                                                                       *
  *************************************************************************
*/
#ifndef FAKELOCALCONTACTLISTTASK_H
#define FAKELOCALCONTACTLISTTASK_H

#include "task.h"

/**
Fire and forget task to let the server think we're using a local contact list
This will show status of "waiting for authorization" contacts

@author Jan Ritzerfeld
*/
class FakeLocalContactListTask : public Task
{
public:
	FakeLocalContactListTask( Task* parent );
	~FakeLocalContactListTask();
	virtual void onGo();
};

#endif

//kate: tab-width 4; indent-mode csands;
