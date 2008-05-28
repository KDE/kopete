/*
    Kopete Oscar Protocol
    blmlimitstask - Get the BLM service limits

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

#include "blmlimitstask.h"
#include <kdebug.h>
#include "connection.h"
#include "transfer.h"
#include "oscartypes.h"
#include "oscarutils.h"

BLMLimitsTask::BLMLimitsTask( Task* parent )
 : Task( parent )
{
}


BLMLimitsTask::~BLMLimitsTask()
{
}


bool BLMLimitsTask::forMe(const Transfer* transfer) const
{
	const SnacTransfer* st = dynamic_cast<const SnacTransfer*>( transfer );

	if (!st)
		return false;

	if ( st->snacService() == 3 && st->snacSubtype() == 3 )
		return true;
	else
		return false;
}

bool BLMLimitsTask::take(Transfer* transfer)
{
	if ( forMe( transfer ) )
	{
		Buffer* buffer = transfer->buffer();
		while ( buffer->length() != 0 )
		{
			TLV t = buffer->getTLV();
			switch ( t.type )
			{
				case 0x0001:
					kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Max BLM entries: " 
						<< t.data << endl;
					break;
				case 0x0002:
					kdDebug( OSCAR_RAW_DEBUG ) << k_funcinfo << "Max watcher entries: " 
						<< t.data << endl;
					break;
				case 0x0003:
					kdDebug( OSCAR_RAW_DEBUG ) << k_funcinfo << "Max online notifications(?): "
						<< t.data << endl;
					break;
			}
		}
		setSuccess( 0, QString::null );
		return true;
	}
	else
		return false;
}

void BLMLimitsTask::onGo()
{
	kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Sending BLM limits request" << endl;
	FLAP f = { 0x02, 0, 0 };
	SNAC s = { 0x0003, 0x0002, 0x0000, client()->snacSequence() };

	Buffer* buffer = new Buffer();
	buffer->addTLV16( 0x0005, 0x0003 );

	Transfer *t = createTransfer( f, s, buffer );
	send( t );
}

//kate: tab-width 4; indent-mode csands;
