//
// C++ Implementation: eventtask
//
// Description: 
//
//
// Author: SUSE AG  (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "gwfield.h"
#include "eventtransfer.h"

#include "eventtask.h"

EventTask::EventTask( Task * parent )
: Task( parent )
{
}

bool EventTask::forMe( Transfer * transfer ) const
{
	// see if we can down-cast transfer to a Response
	EventTransfer * event = dynamic_cast<EventTransfer *>(transfer);
	if ( event )
	{
		// see if we are supposed to handle this kind of event
		// consider speeding this up by having 1 handler per event
		return ( m_eventCodes.find( event->eventType() ) != m_eventCodes.end() );
	}
	return false;
}

#include "eventtask.moc"
