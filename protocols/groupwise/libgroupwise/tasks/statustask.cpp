/*
    Kopete Groupwise Protocol
    statustask.cpp - Event handling task responsible for status change events

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
