//
// C++ Implementation: %{MODULE}
//
// Description: 
//
//
// Author: SUSE AG (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "connectiontask.h"

ConnectionTask::ConnectionTask(Task* parent): EventTask(parent)
{
	registerEvent( EventTransfer::UserDisconnect );
	registerEvent( EventTransfer::ServerDisconnect );
}


ConnectionTask::~ConnectionTask()
{
}

bool ConnectionTask::take( Transfer * transfer )
{
	EventTransfer * incomingEvent;
	if ( forMe( transfer, incomingEvent ) )
	{
		qDebug( "Got a connection event:" );
		switch ( incomingEvent->event() )
		{
			case EventTransfer::UserDisconnect:
				emit connectedElsewhere();
				break;
			case EventTransfer::ServerDisconnect:
				emit serverDisconnect();
				break;
		}
		return true;
	}
	return false;
}

#include "connectiontask.moc"
