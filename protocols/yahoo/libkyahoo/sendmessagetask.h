/*
    Kopete Yahoo Protocol
    Send a message

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

#ifndef SENDMESSAGETASK_H
#define SENDMESSAGETASK_H

#include "task.h"

class QString;

/**
@author André Duffeck
*/
class SendMessageTask : public Task
{
public:
	SendMessageTask(Task *parent);
	~SendMessageTask();
	
	virtual void onGo();
	
	void setText( const QString &text );
	void setTarget( const QString &to );
	void setPicureFlag( int flag );
private:
	QString m_text;
	QString m_target;
	int m_pictureFlag;
};

#endif
