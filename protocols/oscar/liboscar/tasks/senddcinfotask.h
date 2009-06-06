/*
  Kopete Oscar Protocol
  Send extended status info to the server

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
#ifndef SENDDCINFOTASK_H
#define SENDDCINFOTASK_H

#include <task.h>

/**
Fire and forget task that sends our direct connection info
 
@author Matt Rogers
*/
class SendDCInfoTask : public Task
{
public:
	SendDCInfoTask( Task* parent, Oscar::DWORD status );
	~SendDCInfoTask();

	void setIcqMood( int mood );
	void setStatusMessage( const QString &message );

	virtual void onGo();

private:
	Oscar::DWORD mStatus;

	bool mSendMood;
	int mMood;

	bool mSendMessage;
	QString mMessage;
};

#endif

// kate: tab-width 4; indent-mode csands;
