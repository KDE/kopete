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
#ifndef TYPINGTASK_H
#define TYPINGTASK_H

#include "requesttask.h"

/**
	Notifies the server that we are typing or are no longer typing in a particular conversation
	
@author Kopete Developers
*/
class TypingTask : public RequestTask
{
Q_OBJECT

public:
	TypingTask(Task* parent);
	~TypingTask();
	void typing( const QString & conferenceGuid, const bool typing );
};

#endif
