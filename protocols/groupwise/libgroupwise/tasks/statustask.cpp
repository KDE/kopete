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
		EventTransfer * event = dynamic_cast<EventTransfer *>( transfer );
		QDataStream din( event->payload(), IO_ReadOnly);
		din.setByteOrder( QDataStream::LittleEndian );
		Q_UINT16 status;
		din >> status;
		QString statusText;
		uint val;
		char* rawData;
		din.readBytes( rawData, val );
		if ( val ) // val is 0 if there is no status text
			statusText = QString::fromUtf8( rawData, val );
		emit gotStatus( event->source(), status, statusText );
		return true;
	}
	else
		return false;
}
#include "statustask.moc"
