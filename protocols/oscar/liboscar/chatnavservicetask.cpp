/*
    Kopete Oscar Protocol - Chat Navigation service handlers
    Copyright (c) 2005 Matt Rogers <mattr@kde.org>

    Kopete (c) 2002-2005 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "chatnavservicetask.h"

#include <kdebug.h>

#include "transfer.h"
#include "buffer.h"


ChatNavServiceTask::ChatNavServiceTask( Task* parent ) : Task( parent )
{
	m_type = Limits;
}


ChatNavServiceTask::~ChatNavServiceTask()
{
}

void ChatNavServiceTask::setRequestType( RequestType rt )
{
	m_type = rt;
}

ChatNavServiceTask::RequestType ChatNavServiceTask::requestType()
{
	return m_type;
}


bool ChatNavServiceTask::forMe( const Transfer* transfer ) const
{
	const SnacTransfer* st = dynamic_cast<const SnacTransfer*>( transfer );
	if ( !st )
		return false;
	if ( st->snacService() == 0x0004 && st->snacSubtype() == 0x0009 )
		return true;
	
	return false;
}

bool ChatNavServiceTask::take( Transfer* transfer )
{
	if ( !forMe( transfer ) )
		return false;

	setTransfer( transfer );
	Buffer* b = transfer->buffer();
	TLV t = b->getTLV();
	switch ( t.type )
	{
	case 0x0001:
		kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "got chat redirect TLV" << endl;
		break;
	case 0x0002:
	{
		kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "got max concurrent rooms TLV" << endl;
		Buffer tlvTwo(t.data);
		kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "max concurrent rooms is " << tlvTwo.getByte() << endl;
		break;
	}
	case 0x0003:
		kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "exchange info TLV found" << endl;
		break;
	case 0x0004:
		kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "room info TLV found" << endl;
		break;
	};
	
	return true;
	
}

void ChatNavServiceTask::onGo()
{

}

//kate: indent-mode csands; tab-width 4;
