/*
    Kopete Groupwise Protocol
    eventtask.cpp - Ancestor of all Event Handling tasks

    Copyright (c) 2004      SUSE Linux AG	 	 http://www.suse.com
    
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

#include "eventtask.h"
#include "gwfield.h"

EventTask::EventTask( Task * parent )
: Task( parent )
{
}

void EventTask::registerEvent( GroupWise::Event e )
{
	m_eventCodes.append( e );
}

bool EventTask::forMe( const Transfer * transfer ) const
{
	// see if we can down-cast transfer to an EventTransfer
	const EventTransfer * event = dynamic_cast<const EventTransfer *>(transfer);
	if ( event )
	{
		// see if we are supposed to handle this kind of event
		// consider speeding this up by having 1 handler per event
		return ( m_eventCodes.indexOf( event->eventType() ) != -1 );
	}
	return false;
}

#include "eventtask.moc"
