/*
  Kopete Oscar Protocol

  Copyright (c) 2004-2005 Matt Rogers <mattr@kde.org>

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
#ifndef CLIENTREADYTASK_H
#define CLIENTREADYTASK_H

#include "task.h"

#include "rateclass.h"
#include "QList"

/**
Fire and forget task to let the server know we're ready

@author Matt Rogers
*/
class ClientReadyTask : public Task
{
public:
	ClientReadyTask( Task* parent );
	~ClientReadyTask();
	virtual void onGo();

	void setFamilies( const QList<int>& families );

private:
	QList<RateClass*> m_classList;
	QList<int> m_familyList;
};

#endif

//kate: tab-width 4; indent-mode csands;
