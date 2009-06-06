/*
    Kopete Yahoo Protocol
    Stealth/Unstealth a buddy

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

#ifndef STEALTHTASK_H
#define STEALTHTASK_H

#include "task.h"
#include "yahootypes.h"
#include <kdebug.h>

class QString;

/**
@author André Duffeck
*/
class StealthTask : public Task
{
public:
	StealthTask(Task *parent);
	~StealthTask();
	
	virtual void onGo();

	void setTarget( const QString &to );
	void setState( Yahoo::StealthStatus state );
	void setMode( Yahoo::StealthMode mode );
private:
	QString m_target;
	Yahoo::StealthMode m_mode;
	Yahoo::StealthStatus m_state;
};

#endif
