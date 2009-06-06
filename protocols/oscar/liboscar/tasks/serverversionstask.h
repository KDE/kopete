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
#include <QList>
#include "oscartypes.h"
#include "liboscar_export.h"

class Transfer;

/**
@author Matt Rogers
*/
class LIBOSCAR_EXPORT ServerVersionsTask : public Task
{
Q_OBJECT
public:
	ServerVersionsTask( Task* parent );

	~ServerVersionsTask();

	bool forMe(const Transfer* transfer) const;
	bool take(Transfer* transfer);

	static QList<int> buildFamiliesList( Buffer* );

private:
	//! Handles the families the server supports
	void handleFamilies();
	
	//! Request the versions we want for each snac family the
	//! the server supports
	void requestFamilyVersions();
	
private:
	Oscar::WORD m_family;
};

#endif
