/*
  Kopete Oscar Protocol
  errortask.cpp - handle OSCAR protocol errors

  Copyright (c) 2004 Matt Rogers <mattr@kde.org>

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
#include "errortask.h"
#include <kdebug.h>
#include "oscartypes.h"
#include "transfer.h"
#include "connection.h"

ErrorTask::ErrorTask( Task* parent )
	: Task( parent )
{
}


ErrorTask::~ErrorTask()
{
}


bool ErrorTask::forMe( const Transfer* transfer ) const
{
	const SnacTransfer* st = dynamic_cast<const SnacTransfer*>( transfer );
	
	if (!st)
		return false;
	
	if ( st->flapChannel() == 2 && st->snacSubtype() == 1 )
		return true;
	else
		return false;
}

bool ErrorTask::take( Transfer* transfer )
{
	if ( forMe( transfer ) )
	{
		const SnacTransfer* st = dynamic_cast<const SnacTransfer*>( transfer );
		if ( !st )
			return false;

		Buffer* buffer = transfer->buffer();
		//get the error code
		Oscar::WORD errorCode = buffer->getWord();
		kDebug(OSCAR_RAW_DEBUG) << "Error code is " << errorCode;
		TLV t = buffer->getTLV();
		if ( t.type == 0x0008 && t.length > 0 )
		{
			kDebug(OSCAR_RAW_DEBUG) << "TLV error subcode is " 
					<< t.data << endl;
		}

		Oscar::MessageInfo info = client()->takeMessageInfo( st->snacRequest() );
		if ( info.isValid() )
			emit messageError( info.contact, info.id );

		return true;
	}
	else 
		return false;
}

#include "errortask.moc"

//kate indent-mode csands;
