/*
    Kopete Oscar Protocol
    serverversionstask.h - Handles the snac family versions

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

#ifndef SERVERVERSIONSTASK_H
#define SERVERVERSIONSTASK_H

#include "task.h"
#include <qvaluelist.h>
#include "oscartypes.h"

class Transfer;

/**
@author Matt Rogers
*/
class ServerVersionsTask : public Task
{
Q_OBJECT
public:
	ServerVersionsTask( Task* parent );

	~ServerVersionsTask();

	bool forMe(const Transfer* transfer) const;
	bool take(Transfer* transfer);


private:
	//! Handles the families the server supports
	void handleFamilies();
	
	//! Handles the version of each family the server supports
	void handleServerVersions();
	
	//! Request the versions we want for each snac family the
	//! the server supports
	void requestFamilyVersions();
	
private:
	QValueList<int> m_familiesList;
	WORD m_family;
};

#endif
