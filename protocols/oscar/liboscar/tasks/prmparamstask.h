/*
   Kopete Oscar Protocol
   prmparamstask.h - handle OSCAR protocol errors

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
#ifndef PRMPARAMSTASK_H
#define PRMPARAMSTASK_H

#include <task.h>

class Transfer;

/**
@author Matt Rogers
*/
class PRMParamsTask : public Task
{
public:
	PRMParamsTask( Task* parent );
	~PRMParamsTask();

	virtual bool forMe( const Transfer* transfer ) const;
	virtual bool take( Transfer* transfer );
	virtual void onGo();

};

#endif

// kate: space-indent on; tab-width 4; indent-mode csands;
