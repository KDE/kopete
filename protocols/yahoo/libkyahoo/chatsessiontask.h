/*
    Kopete Yahoo Protocol
    chatsessiontask.h - Register / Unregister a chatsession

    Copyright (c) 2006 André Duffeck <duffeck@kde.org>

    Kopete (c) 2002-2006 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef CHATSESSIONTASK_H
#define CHATSESSIONTASK_H

#include "task.h"

class QString;

/**
@author André Duffeck
*/
class ChatSessionTask : public Task
{
public:
	enum Type { RegisterSession, UnregisterSession };
	ChatSessionTask(Task *parent);
	~ChatSessionTask();
	
	virtual void onGo();
	
	void setTarget( const QString &to );
	void setType( Type type );
private:
	QString m_target;
	Type m_type;
};

#endif
