/*
    Kopete Yahoo Protocol
    Log off the Yahoo server

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

#include "logofftask.h"
#include "transfer.h"
#include "ymsgtransfer.h"
#include "yahootypes.h"
#include "client.h"
#include <qstring.h>
#include "yahoo_protocol_debug.h"

LogoffTask::LogoffTask(Task* parent) : Task(parent)
{
	qCDebug(YAHOO_PROTOCOL_LOG) ;
}

LogoffTask::~LogoffTask()
{
}

void LogoffTask::onGo()
{
	qCDebug(YAHOO_PROTOCOL_LOG) ;

	YMSGTransfer *t = new YMSGTransfer(Yahoo::ServiceLogoff);
	t->setId( client()->sessionID() );
	send( t );
	
	setSuccess();
}
