/*
    Kopete Yahoo Protocol
    Send a notification

    Copyright (c) 2005 André Duffeck <duffeck@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef SENDNOTIFYTASK_H
#define SENDNOTIFYTASK_H

#include "task.h"

class QString;

/**
@author André Duffeck
*/
class SendNotifyTask : public Task
{
Q_OBJECT
public:
	enum Type { NotifyTyping, NotifyWebcamInvite, NotifyGame };
	enum State { Active = 1, NotActive = 0 };

	SendNotifyTask(Task *parent);
	~SendNotifyTask();
	
	virtual void onGo();

	void setType( Type type );
	void setTarget( const QString &to );
	void setState( State );
private:
	QString m_target;
	Type m_type;
	State m_state;
};

#endif
