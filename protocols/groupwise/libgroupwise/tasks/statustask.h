//
// C++ Interface: statustask
//
// Description: 
//
//
// Author: Kopete Developers <kopete-devel@kde.org>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef STATUSTASK_H
#define STATUSTASK_H

#include "eventtask.h"

/**
@author Kopete Developers
*/
class StatusTask : public EventTask
{
Q_OBJECT
public:
	StatusTask(Task* parent);
	~StatusTask();
	bool take( Transfer * transfer );
signals:
	void gotStatusChange();
};

#endif
