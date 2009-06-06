/*
    Kopete Yahoo Protocol
    Change our Status

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

#ifndef CHANGESTATUSTASK_H
#define CHANGESTATUSTASK_H

#include "task.h"
#include "yahootypes.h"

class QString;


/**
@author André Duffeck
*/
class ChangeStatusTask : public Task
{
public:
	enum Type { Available, Away };
	ChangeStatusTask(Task *parent);
	~ChangeStatusTask();
	
	virtual void onGo();

	void setMessage( const QString &msg );
	void setStatus( Yahoo::Status status );
	void setType( Yahoo::StatusType type );
private:
	enum Visibility { Visible = 1, Invisible = 2 };
	QString m_message;
	Yahoo::Status m_status;
	Yahoo::StatusType m_type;

	void sendVisibility( Visibility visible );
};

#endif
