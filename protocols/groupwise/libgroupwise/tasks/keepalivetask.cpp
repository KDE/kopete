/*
    Kopete Groupwise Protocol
    keepalivetask.cpp - Send keepalive pings to the server

    Copyright (c) 2004      SUSE Linux AG	 	 http://www.suse.com
    
    Based on Iris, Copyright (C) 2003  Justin Karneges

    Kopete (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>
 
    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include <qtimer.h>

#include "client.h"
#include "request.h"
#include "requestfactory.h"
#include "keepalivetask.h"

#define GW_KEEPALIVE_INTERVAL 60000

KeepAliveTask::KeepAliveTask(Task* parent): RequestTask(parent)
{
	m_keepAliveTimer = new QTimer();
	connect( m_keepAliveTimer, SIGNAL( timeout() ), SLOT( slotSendKeepAlive ) );
	m_keepAliveTimer->start( GW_KEEPALIVE_INTERVAL );
}


KeepAliveTask::~KeepAliveTask()
{
	m_keepAliveTimer->stop();
	delete m_keepAliveTimer;
}

void KeepAliveTask::slotSendKeepAlive()
{
	Field::FieldList lst;
	createTransfer( "ping", lst );
}

#include "keepalivetask.moc"
