//
// C++ Implementation: statustask
//
// Description: 
//
//
// Author: SUSE AG (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "statustask.h"

StatusTask::StatusTask(Task* parent): EventTask(parent)
{
	registerEvent( GroupWise::StatusChange );
}

StatusTask::~StatusTask()
{
}

bool StatusTask::take( Transfer * transfer )
{
	EventTransfer * event;
	if ( forMe( transfer, event ) )
	{
		qDebug( "Got a status change!" );
		qDebug( "%s changed status to %i, message: %s", event->source().ascii(), event->status(), event->statusText().ascii() );
		emit gotStatus( event->source().lower(), event->status(), event->statusText() );
		return true;
	}
	else
		return false;
}
#include "statustask.moc"
