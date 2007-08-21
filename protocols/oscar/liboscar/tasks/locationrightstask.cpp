/*
    Kopete Oscar Protocol
    locationrightstask.cpp - Set up the service limitations

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

#include "locationrightstask.h"
#include <kdebug.h>
#include "buffer.h"
#include "transfer.h"
#include "oscartypes.h"
#include "oscarutils.h"
#include "connection.h"

using namespace Oscar;

LocationRightsTask::LocationRightsTask( Task* parent ) 
	: Task( parent )
{
}


LocationRightsTask::~LocationRightsTask()
{
}


bool LocationRightsTask::forMe( const Transfer* transfer ) const
{
	const SnacTransfer* st = dynamic_cast<const SnacTransfer*>( transfer );
	
	if (!st)
		return false;
	
	if ( st->snacService() == 2 && st->snacSubtype() == 3 )
		return true;
	else
		return false;
}

bool LocationRightsTask::take( Transfer* transfer )
{
	if ( forMe( transfer ) )
	{
		setTransfer( transfer );
		handleLocationRightsResponse();
		setTransfer( 0 );
		return true;
	}
	else
		return false;
}

void LocationRightsTask::onGo()
{
	sendLocationRightsRequest();
}

void LocationRightsTask::sendLocationRightsRequest()
{
	FLAP f = { 0x02, 0, 0 };
	SNAC s = { 0x0002, 0x0002, 0x0000, client()->snacSequence() };
	Buffer* b = new Buffer();
	Transfer* st = createTransfer( f, s, b );
	send( st );
}

void LocationRightsTask::handleLocationRightsResponse()
{
	kDebug(OSCAR_RAW_DEBUG) << "Ignoring location rights response";
	setSuccess( 0, QString() );
}


//kate: tab-width 4; indent-mode csands;
