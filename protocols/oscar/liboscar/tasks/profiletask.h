/*
 Kopete Oscar Protocol
 profiletask.h - Update the user's profile on the server

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
#ifndef PROFILETASK_H
#define PROFILETASK_H

#include "task.h"

/**
Task that sets the profile and away message on the server (AIM only).
Also takes care of updating the capabilities supported by the client (AIM and ICQ).

The profile will be updated only if the profile text has been set non-null.
The away message will be set only if the away message has been set non-null.

@author Matt Rogers
*/
class ProfileTask : public Task
{
public:
	ProfileTask( Task* parent );
	~ProfileTask();

	bool forMe( const Transfer* transfer ) const;
	bool take( Transfer* transfer );
	void onGo();

	void setProfileText( const QString& text );
	void setAwayMessage( const QString& text );
	void setXtrazStatus( int xtrazStatus );
	void setCapabilities( bool value );

private:

	void sendProfileUpdate();

private:
	QString m_profileText;
	QString m_awayMessage;
	int m_xtrazStatus;
	bool m_sendCaps;
};

#endif

//kate: tab-width 4; indent-mode csands;
