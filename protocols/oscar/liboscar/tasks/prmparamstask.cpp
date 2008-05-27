/*
   Kopete Oscar Protocol
   prmparamstask.h - handle OSCAR protocol errors

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
#include "prmparamstask.h"
#include <kdebug.h>
#include "connection.h"
#include "transfer.h"
#include "oscartypes.h"
#include "oscarutils.h"

using namespace Oscar;

PRMParamsTask::PRMParamsTask( Task* parent )
	: Task( parent )
{
}


PRMParamsTask::~PRMParamsTask()
{
}


bool PRMParamsTask::forMe( const Transfer* transfer ) const
{
	const SnacTransfer* st = dynamic_cast<const SnacTransfer*>( transfer );
	if ( !st )
		return false;

	if ( st->snacService() == 0x0009 && st->snacSubtype() == 0x0003 )
		return true;
	
	return false;
}

bool PRMParamsTask::take( Transfer* transfer )
{
	if ( forMe( transfer ) )
	{
		kDebug(OSCAR_RAW_DEBUG) << "Ignoring PRM Parameters. We don't use them";
		setSuccess( 0, QString() );
		return true;
	}

	return false;
}

void PRMParamsTask::onGo()
{
	kDebug( OSCAR_RAW_DEBUG ) << "Sending PRM Parameters request";
	FLAP f = { 0x02, 0, 0 };
	SNAC s = { 0x0009, 0x0002, 0x0000, client()->snacSequence() };
	Buffer* buffer = new Buffer();
	Transfer *t = createTransfer( f, s, buffer );
	send( t );
}

// kate: tab-width 4; indent-mode csands;
