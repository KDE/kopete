/*
   Kopete Oscar Protocol
   blmlimitstask.h - Fetch the limits for the BLM service

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
#ifndef BLMLIMITSTASK_H
#define BLMLIMITSTASK_H

#include "task.h"

/**
Fetch the limits for the BLM service
 
@author Matt Rogers
*/
class BLMLimitsTask : public Task
{
public:
	BLMLimitsTask( Task* parent );

	~BLMLimitsTask();

	virtual bool forMe( const Transfer* transfer ) const;
	virtual bool take( Transfer* transfer );
	virtual void onGo();

};

#endif

//kate: tab-width 4; indent-mode csands;
