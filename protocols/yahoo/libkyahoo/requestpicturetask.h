/*
    Kopete Yahoo Protocol
    Request a Picture of a Buddy

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

#ifndef REQUESTPICTURETASK_H
#define REQUESTPICTURETASK_H

#include "task.h"

class QString;

/**
@author André Duffeck
*/
class RequestPictureTask : public Task
{
Q_OBJECT
public:
	RequestPictureTask(Task *parent);
	virtual ~RequestPictureTask();
	
	virtual void onGo();
	
	void setTarget( const QString &target );
private:
	QString m_target;
};

#endif
