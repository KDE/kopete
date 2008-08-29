/*
    messageacktask.cpp  - Incoming OSCAR Messaging Acknowledgement Handler

    Copyright (c) 2008      by Roman Jarosz <kedgedev@centrum.cz>
    Kopete    (c) 2008      by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "messageacktask.h"

#include "transfer.h"
#include "connection.h"

MessageAckTask::MessageAckTask( Task* parent ) : Task( parent )
{
}

bool MessageAckTask::forMe( const Transfer* transfer ) const
{
	const SnacTransfer* st = dynamic_cast<const SnacTransfer*>( transfer );
	if ( !st )
		return false;

	if ( st->snacService() == 0x0004 && st->snacSubtype() == 0x000C )
		return true;
	else
		return false;
}

bool MessageAckTask::take( Transfer* transfer )
{
	if ( forMe( transfer ) )
	{
		const SnacTransfer* st = dynamic_cast<const SnacTransfer*>( transfer );
		if ( !st )
			return false;

		Oscar::MessageInfo info = client()->takeMessageInfo( st->snacRequest() );
		if ( info.isValid() )
			emit messageAck( info.contact, info.id );

		return true;
	}

	return false;
}


#include "messageacktask.moc"

//kate: indent-mode csands;
