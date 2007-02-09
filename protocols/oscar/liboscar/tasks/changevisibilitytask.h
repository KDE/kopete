/*
    Kopete Oscar Protocol
    changevisibilitytask.h - Changes the visibility of the account via SSI

    Copyright (c) 2005 Matt Rogers <mattr@kde.org>

    Kopete (c) 2002-2005 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef CHANGEVISIBILITYTASK_H
#define CHANGEVISIBILITYTASK_H

#include "task.h"

/**
 * This class provides a way to change how the account user
 * appears on everybody else's contact list. It is used to
 * implement the invisible online status in ICQ and AIM
 * @author Matt Rogers
 */
class ChangeVisibilityTask : public Task
{
public:
	ChangeVisibilityTask( Task* parent );
	~ChangeVisibilityTask();

	void setVisible( bool visible = true );
	
	virtual bool forMe( const Transfer* transfer ) const;
	virtual bool take( Transfer* transfer );
	virtual void onGo();

private:
	//damnit, this is ugly. time to refactor SSI stuff out into it's own
	//class, file, whatever.
	//! Send the SSI edit start packet
	void sendEditStart();
	
	//! Send the SSI edit end packet
	void sendEditEnd();
	
private:
	bool m_visible;
	DWORD m_sequence;
};

#endif

//kate: indent-mode csands; space-indent off; replace-tabs off; tab-width 4;
