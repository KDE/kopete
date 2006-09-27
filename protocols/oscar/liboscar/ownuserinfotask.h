/*
    Kopete Oscar Protocol
    aimlogintask.h - Handles logging into to the AIM service

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
#ifndef OWNUSERINFOTASK_H
#define OWNUSERINFOTASK_H

#include "task.h"
#include "userdetails.h"

/**
Request our user info from the server and handle our user info when it comes back
 
@author Kopete Developers
*/
class OwnUserInfoTask : public Task
{
Q_OBJECT
public:
	OwnUserInfoTask( Task* parent );

	~OwnUserInfoTask();

	virtual bool forMe( const Transfer* transfer ) const;
	virtual bool take( Transfer* transfer );
	virtual void onGo();

	UserDetails getInfo() const;
	
signals:
	/** Emitted when user info is received. Needed because succeeded() is only emitted once. */
	void gotInfo();
	
	void haveAvailableMessage( const QString& );
	
	void haveIconChecksum( const QString& );
	
	void buddyIconUploadRequested();

private:
	UserDetails m_details;
};

#endif

//kate: tab-width 4; indent-mode csands;
