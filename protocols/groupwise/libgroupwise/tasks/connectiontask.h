//
// C++ Interface: %{MODULE}
//
// Description: 
//
//
// Author: SUSE AG (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef CONNECTIONTASK_H
#define CONNECTIONTASK_H

#include "eventtask.h"

/**
This task monitors connection related events, currently 'connected elsewhere' disconnects and server disconnect notification.

@author Kopete Developers
*/
class ConnectionTask : public EventTask
{
Q_OBJECT
public:
	ConnectionTask(Task* parent);
	~ConnectionTask();
	bool take( Transfer * transfer );
signals:
	void connectedElsewhere();
	void serverDisconnect();
};

#endif
