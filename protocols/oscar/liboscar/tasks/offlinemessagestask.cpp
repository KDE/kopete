/*
   Kopete Oscar Protocol
   offlinemessagestask.cpp - Offline messages handling

   Copyright (c) 2008 Roman Jarosz <kedgedev@centrum.cz>

   Kopete (c) 2008 by the Kopete developers <kopete-devel@kde.org>

   *************************************************************************
   *                                                                       *
   * This library is free software; you can redistribute it and/or         *
   * modify it under the terms of the GNU Lesser General Public            *
   * License as published by the Free Software Foundation; either          *
   * version 2 of the License, or (at your option) any later version.      *
   *                                                                       *
   *************************************************************************
*/
#include "offlinemessagestask.h"
#include <kdebug.h>
#include "connection.h"
#include "transfer.h"

using namespace Oscar;

OfflineMessagesTask::OfflineMessagesTask( Task* parent )
	: Task( parent )
{
}

bool OfflineMessagesTask::forMe( const Transfer* transfer ) const
{
	Q_UNUSED( transfer );
	return false;
}

bool OfflineMessagesTask::take( Transfer* transfer )
{
	Q_UNUSED( transfer );
	return false;
}

void OfflineMessagesTask::onGo()
{
	kDebug( OSCAR_RAW_DEBUG ) << "Requesting offline messages";

	FLAP f = { 0x02, 0, 0 };
	SNAC s = { 0x0004, 0x0010, 0x0000, client()->snacSequence() };

	Buffer* buffer = new Buffer();
	Transfer *t = createTransfer( f, s, buffer );
	send( t );

	setSuccess( 0, QString() );
}

// kate: tab-width 4; indent-mode csands;
