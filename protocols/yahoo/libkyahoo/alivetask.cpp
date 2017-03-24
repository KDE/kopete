/*
    alivetask.cpp
    Send a alive to the server

    Copyright (c) 2006 André Duffeck <duffeck@kde.org>

    Kopete    (c) 2002-2006 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "alivetask.h"
#include "transfer.h"
#include "ymsgtransfer.h"
#include "yahootypes.h"
#include "client.h"
#include <qstring.h>
#include "yahoo_protocol_debug.h"

AliveTask::AliveTask(Task* parent) : Task(parent)
{
    qCDebug(YAHOO_PROTOCOL_LOG) ;
}

AliveTask::~AliveTask()
{
}

void AliveTask::onGo()
{
    qCDebug(YAHOO_PROTOCOL_LOG) ;

	YMSGTransfer *t = new YMSGTransfer(Yahoo::ServicePing7);
	t->setParam( 0, client()->userId().toLocal8Bit() );
	t->setId( client()->sessionID() );
	send( t );
	
	setSuccess();
}
