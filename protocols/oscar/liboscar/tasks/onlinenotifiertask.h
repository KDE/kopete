/*
	Kopete Oscar Protocol
	onlinenotifiertask.h - handles all the status notifications
	
	Copyright (c) 2004 by Matt Rogers <mattr@kde.org>
	
	Based on code Copyright (c) 2004 SuSE Linux AG <http://www.suse.com>
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
#ifndef ONLINENOTIFIERTASK_H
#define ONLINENOTIFIERTASK_H

#include <task.h>
#include "userdetails.h"

class Transfer;
class QString;
/**
Tracks status notifications (online, offline, etc.) for contacts
Implements SNACS (0x03, 0x11) and (0x03, 0x12)
 
@author Matt Rogers
*/
class OnlineNotifierTask : public Task
{
Q_OBJECT
public:
	OnlineNotifierTask( Task* parent );

	~OnlineNotifierTask();

	virtual bool take( Transfer* transfer );

protected:
	virtual bool forMe( const Transfer* transfer ) const;

signals:
	void userIsOnline( const QString& user, const UserDetails& ud );
	void userIsOffline( const QString& user, const UserDetails& ud );
	
private:
	void userOnline();
	void userOffline();

};

#endif

//kate: tab-width 4; indent-mode csands;
