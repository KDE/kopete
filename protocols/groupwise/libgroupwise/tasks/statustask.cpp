/*
    Kopete Groupwise Protocol
    statustask.cpp - Event handling task responsible for status change events

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

#include "statustask.h"

#include "client.h"

StatusTask::StatusTask(Task* parent): EventTask(parent)
{
	registerEvent( GroupWise::StatusChange );
}

StatusTask::~StatusTask()
{
}

bool StatusTask::take( Transfer * transfer )
{
	if ( forMe( transfer ) )
	{
		EventTransfer * event = static_cast<EventTransfer *>(transfer);
		client()->debug( "Got a status change!" );
		client()->debug( QString( "%1 changed status to %2, message: %3" ).arg( event->source() ).arg( event->status() ).arg( event->statusText() ) );
		emit gotStatus( event->source().toLower(), event->status(), event->statusText() );
		return true;
	}
	else
		return false;
}
#include "statustask.moc"
