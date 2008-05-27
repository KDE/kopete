/*
    Kopete Yahoo Protocol
    Send a authorization request response

    Copyright (c) 2006 André Duffeck <duffeck@kde.org>
    Kopete    (c) 2003-2006 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef SENDAUTHRESPTASK_H
#define SENDAUTHRESPTASK_H

#include "task.h"

class QString;

/**
@author André Duffeck
*/
class SendAuthRespTask : public Task
{
Q_OBJECT
public:
	SendAuthRespTask(Task *parent);
	~SendAuthRespTask();
	
	virtual void onGo();

	void setGranted( bool );
	void setTarget( const QString &to );
	void setMessage( const QString &msg );
private:
	QString m_target;
	bool m_granted;
	QString m_msg;
};

#endif
