/*
    Kopete Groupwise Protocol
    connectiontask.cpp - Event Handling task responsible for all connection related events

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
#include "client.h"

#include "connectiontask.h"

ConnectionTask::ConnectionTask(Task* parent): EventTask(parent)
{
	registerEvent( GroupWise::UserDisconnect );
	registerEvent( GroupWise::ServerDisconnect );
}


ConnectionTask::~ConnectionTask()
{
}

bool ConnectionTask::take( Transfer * transfer )
{
	EventTransfer * incomingEvent;
	if ( forMe( transfer, incomingEvent ) )
	{
		client()->debug( "Got a connection event:" );
		switch ( incomingEvent->eventType() )
		{
			case GroupWise::UserDisconnect:
				emit connectedElsewhere();
				break;
			case GroupWise::ServerDisconnect:
				emit serverDisconnect();
				break;
		}
		return true;
	}
	return false;
}

#include "connectiontask.moc"
