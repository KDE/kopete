//
// C++ Interface: %{MODULE}
//
// Description: 
//
//
// Author: %{AUTHOR} <%{EMAIL}>, (C) %{YEAR}
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef SETSTATUSTASK_H
#define SETSTATUSTASK_H

#include "requesttask.h"

/**
@author Kopete Developers
*/
class SetStatusTask : public RequestTask
{
public:
	SetStatusTask(Task* parent);
	void status( const uint newStatus, const QString &awayMessage, const QString &autoReply );
	void onGo();
	~SetStatusTask();
};

#endif
