/*
   Kopete Oscar Protocol
   ssiparamstask.h - Get the SSI parameters so we can use them

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
#ifndef SSIPARAMSTASK_H
#define SSIPARAMSTASK_H

#include "task.h"

/**
@author Kopete Developers
*/
class SSIParamsTask : public Task
{
public:
    SSIParamsTask(Task* parent);

    ~SSIParamsTask();

    virtual bool forMe(const Transfer* transfer) const;
    virtual bool take(Transfer* transfer);
    virtual void onGo();

private:
	void handleParamReply();
};

#endif

// kate: tab-width 4; indent-mode csands;
