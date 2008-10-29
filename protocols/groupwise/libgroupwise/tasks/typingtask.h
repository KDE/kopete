/*
    Kopete Groupwise Protocol
    typingtask.h - sends typing notifications to the server

    Copyright (c) 2004      SUSE Linux AG	 	 http://www.suse.com
    
    Based on Iris, Copyright (C) 2003  Justin Karneges <justin@affinix.com>

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
	void typing( const GroupWise::ConferenceGuid & guid, const bool typing );
};

#endif
