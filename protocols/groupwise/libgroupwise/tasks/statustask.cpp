//
// C++ Implementation: statustask
//
// Description: 
//
//
// Author: Kopete Developers <kopete-devel@kde.org>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "statustask.h"

StatusTask::StatusTask(Task* parent): EventTask(parent)
{
	addEventCode( EventTransfer::StatusChange );
}

StatusTask::~StatusTask()
{
}

bool StatusTask::take( Transfer * transfer )
{
	if ( forMe( transfer ) )
	{
		qDebug( "Got a status change!" );
	}
}
#include "statustask.moc"
