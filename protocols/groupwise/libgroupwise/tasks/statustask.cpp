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
	registerEvent( EventTransfer::StatusChange );
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
		qDebug( "%s changed status to %i, message: %s", event->source().ascii(), status, statusText.ascii() );
		// HACK: check with mike about case sensitivity.
		emit gotStatus( event->source().lower(), status, statusText );
		return true;
	}
	else
		return false;
}
#include "statustask.moc"
