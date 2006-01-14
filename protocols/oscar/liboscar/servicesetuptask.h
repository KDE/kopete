/*
	Kopete Oscar Protocol
	servicesetuptask.h - Set up the services for the BOS connection
	
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

#ifndef SERVICESETUPTASK_H
#define SERVICESETUPTASK_H

#include "task.h"

class LocationRightsTask;
class ProfileTask;
class BLMLimitsTask;
class ICBMParamsTask;
class PRMParamsTask;
class SSIParamsTask;
class SSIListTask;
class SSIActivateTask;


/**
Set up the various services for the BOS connection
@author Matt Rogers
*/
class ServiceSetupTask : public Task
{
Q_OBJECT
public:
	ServiceSetupTask( Task* parent );
	~ServiceSetupTask();

	bool forMe( const Transfer* transfer ) const;
	bool take( Transfer* transfer );
	void onGo();

public slots:
	void childTaskFinished();

private:

	/** Tracks how many tasks have finished */
	int m_finishedTaskCount;

	LocationRightsTask* m_locRightsTask;
	ProfileTask* m_profileTask;
	BLMLimitsTask* m_blmLimitsTask;
	ICBMParamsTask* m_icbmTask;
	PRMParamsTask* m_prmTask;
	SSIParamsTask* m_ssiParamTask;
	SSIListTask* m_ssiListTask;
	SSIActivateTask* m_ssiActivateTask;
};

#endif

//kate: tab-width 4; indent-mode csands;
