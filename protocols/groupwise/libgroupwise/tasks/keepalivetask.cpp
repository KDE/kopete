/*
    Kopete Groupwise Protocol
    keepalivetask.cpp - Send keepalive pings to the server

    Copyright (c) 2004      SUSE Linux AG	 	 http://www.suse.com
              (c) 2006      Novell, Inc.

    Based on Iris, Copyright (C) 2003  Justin Karneges <justin@affinix.com>

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

#include "keepalivetask.h"
#include "client.h"
#include "request.h"
#include "requestfactory.h"

KeepAliveTask::KeepAliveTask(Task* parent): RequestTask(parent)
{
}


KeepAliveTask::~KeepAliveTask()
{
}

void KeepAliveTask::setup()
{
	Field::FieldList lst;
	createTransfer( "ping", lst );
}

#include "keepalivetask.moc"
