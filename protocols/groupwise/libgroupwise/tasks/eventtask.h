//
// C++ Interface: eventtask
//
// Description: 
//
//
// Author: SUSE AG  (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef GW_EVENTTASK_H
#define GW_EVENTTASK_H

#include <qvaluelist.h>

#include "task.h"

class Transfer;

class EventTask : public Task
{
Q_OBJECT
	public:
		EventTask( Task *parent );
	protected:
		bool forMe( Transfer * transfer ) const;
	private:
		QValueList<int> m_eventCodes;
};

#endif
