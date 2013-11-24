/*
   Kopete Oscar Protocol
   ssiactivatetask.cpp - Send the SNAC for SSI activation

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

#include "ssiactivatetask.h"
#include <kdebug.h>
#include "connection.h"
#include "buffer.h"
#include "oscartypes.h"
#include "oscarutils.h"
#include "transfer.h"



SSIActivateTask::SSIActivateTask(Task* parent): Task(parent)
{
}


SSIActivateTask::~SSIActivateTask()
{
}


void SSIActivateTask::onGo()
{
	kDebug( OSCAR_RAW_DEBUG ) << "Sending Contact activate";
	FLAP f = { 0x02, 0, 0 } ;
	SNAC s = { 0x0013, 0x0007, 0x0000, client()->snacSequence() };
	Buffer* buffer = new Buffer();
	Transfer* t = createTransfer( f, s, buffer );
	send( t );
	setSuccess( 0, QString() );
}

// kate: tab-width 4; indent-mode csands;
