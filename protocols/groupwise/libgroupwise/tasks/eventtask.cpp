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
#include "eventtask.h"

EventTask::EventTask( Task * parent )
: Task( parent )
{
}

void EventTask::registerEvent( GroupWise::Event e )
{
	m_eventCodes.append( e );
}

bool EventTask::forMe( Transfer * transfer, EventTransfer*& event ) const
{
	// see if we can down-cast transfer to an EventTransfer
	/*EventTransfer * */
	event = dynamic_cast<EventTransfer *>(transfer);
	if ( event )
	{
		// see if we are supposed to handle this kind of event
		// consider speeding this up by having 1 handler per event
		//qDebug( "EventTask::forMe - do I handle event %i?", event->event() );
		return ( m_eventCodes.find( event->event() ) != m_eventCodes.end() );
	}
	return false;
}

#include "eventtask.moc"
