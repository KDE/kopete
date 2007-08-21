/*
  Kopete Oscar Protocol
  sendidletimetask.cpp - Sends the idle time to the server

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
#include "sendidletimetask.h"

#include <kdebug.h>
#include "connection.h"
#include "buffer.h"
#include "oscarutils.h"
#include "transfer.h"

SendIdleTimeTask::SendIdleTimeTask( Task* parent ) : Task( parent )
{
	m_idleTime = 0;
}


SendIdleTimeTask::~SendIdleTimeTask()
{
	
}


void SendIdleTimeTask::onGo()
{
	kDebug( OSCAR_RAW_DEBUG ) << "Sending idle time of " << m_idleTime;
	FLAP f = { 0x02, 0, 0 };
	SNAC s = { 0x0001, 0x0011, 0x0000, client()->snacSequence() };
	Buffer* buffer = new Buffer();
	buffer->addDWord( m_idleTime );
	
	Transfer *t = createTransfer( f, s, buffer );
	send( t );
	setSuccess( 0, QString() );
	
}

void SendIdleTimeTask::setIdleTime( Oscar::DWORD newTime )
{
	m_idleTime = newTime;
}

// kate: tab-width 4; indent-mode csands;
