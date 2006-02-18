/*
    pingtask.cpp
    Send a ping to the server

    Copyright (c) 2006 André Duffeck <andre.duffeck@kdemail.net>

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

#include "pingtask.h"
#include "transfer.h"
#include "ymsgtransfer.h"
#include "yahootypes.h"
#include "client.h"
#include <qstring.h>
#include <kdebug.h>

PingTask::PingTask(Task* parent) : Task(parent)
{
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;
}

PingTask::~PingTask()
{
}

void PingTask::onGo()
{
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;

	YMSGTransfer *t = new YMSGTransfer(Yahoo::ServicePing7);
	t->setParam( 0, client()->userId().local8Bit() );
	t->setId( client()->sessionID() );
	send( t );
	
	setSuccess( true );
}
