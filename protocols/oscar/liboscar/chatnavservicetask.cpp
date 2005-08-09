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

void ChatNavServiceTask::handleExchangeInfo( const TLV& t )
{
	kdDebug(OSCAR_RAW_DEBUG) << "Parsing exchange info TLV" << t.length << endl;
	Buffer b(t.data);
	WORD id = b.getWord();
	int tlvCount = b.getWord();
	int realCount = 0;
	kdDebug(OSCAR_RAW_DEBUG) << "Expecting " << tlvCount << " TLVs" << endl;
	while ( b.length() > 0 )
	{
		TLV t = b.getTLV();
		switch (t.type)
		{
		case 0x02:
			kdDebug(OSCAR_RAW_DEBUG) << "user class is " << t.data << endl;
			break;
		case 0x03:
			kdDebug(OSCAR_RAW_DEBUG) << "max concurrent rooms for the exchange is " << t.data << endl;
			break;
		case 0x04:
			kdDebug(OSCAR_RAW_DEBUG) << "max room name length is " << t.data << endl;
			break;
		case 0x05:
			kdDebug(OSCAR_RAW_DEBUG) << "received root rooms info" << endl;
			break;
		case 0x06:
			kdDebug(OSCAR_RAW_DEBUG) << "received search tags" << endl;
			break;
		case 0xCA:
			kdDebug(OSCAR_RAW_DEBUG) << "have exchange creation time" << endl;
			break;
		case 0xC9:
			kdDebug(OSCAR_RAW_DEBUG) << "got chat flag" << endl;
			break;
		case 0xD0:
			kdDebug(OSCAR_RAW_DEBUG) << "got mandantory channels" << endl;
			break;
		case 0xD1:
			kdDebug(OSCAR_RAW_DEBUG) << "max message length" << t.data << endl;
			break;
		case 0xD2:
			kdDebug(OSCAR_RAW_DEBUG) << "max occupancy" << t.data << endl;
			break;
		case 0xD3:
			kdDebug(OSCAR_RAW_DEBUG) << "exchange name" << endl;
			break;
		case 0xD4:
			kdDebug(OSCAR_RAW_DEBUG) << "got optional channels" << endl;
			break;
		case 0xD5:
			kdDebug(OSCAR_RAW_DEBUG) << "creation permissions " << t.data << endl;
			break;
		default:
			kdDebug(OSCAR_RAW_DEBUG) << "unknown TLV type " << t.type << endl;
			break;
		}
		realCount++;
	}
}

void ChatNavServiceTask::handleBasicRoomInfo( const TLV& t )
{
	kdDebug(OSCAR_RAW_DEBUG) << "Parsing exchange info TLV" << t.length << endl;
	Buffer b(t.data);
	WORD id = b.getWord();
	int tlvCount = b.getWord();
	int realCount = 0;
	kdDebug(OSCAR_RAW_DEBUG) << "Expecting " << tlvCount << " TLVs" << endl;
	while ( b.length() > 0 )
	{
		TLV t = b.getTLV();
		switch (t.type)
		{
		case 0x66:
			kdDebug(OSCAR_RAW_DEBUG) << "user class is " << t.data << endl;
			break;
		case 0x67:
			kdDebug(OSCAR_RAW_DEBUG) << "user array" << endl;
			break;
		case 0x68:
			kdDebug(OSCAR_RAW_DEBUG) << "evil generated" << t.data << endl;
			break;
		case 0x69:
			kdDebug(OSCAR_RAW_DEBUG) << "evil generated array" << endl;
			break;
		case 0x6A:
			kdDebug(OSCAR_RAW_DEBUG) << "fully qualified name" << endl;
			break;
		case 0x6B:
			kdDebug(OSCAR_RAW_DEBUG) << "moderator" << endl;
			break;
		case 0x6D:
			kdDebug(OSCAR_RAW_DEBUG) << "num children" << endl;
			break;
		case 0x06F:
			kdDebug(OSCAR_RAW_DEBUG) << "occupancy" << endl;
			break;
		case 0x71:
			kdDebug(OSCAR_RAW_DEBUG) << "occupant evil" << endl;
			break;
		case 0x75:
			kdDebug(OSCAR_RAW_DEBUG) << "room activity" << endl;
			break;
		case 0xD0:
			kdDebug(OSCAR_RAW_DEBUG) << "got mandantory channels" << endl;
			break;
		case 0xD1:
			kdDebug(OSCAR_RAW_DEBUG) << "max message length" << t.data << endl;
			break;
		case 0xD2:
			kdDebug(OSCAR_RAW_DEBUG) << "max occupancy" << t.data << endl;
			break;
		case 0xD3:
			kdDebug(OSCAR_RAW_DEBUG) << "exchange name" << endl;
			break;
		case 0xD4:
			kdDebug(OSCAR_RAW_DEBUG) << "got optional channels" << endl;
			break;
		case 0xD5:
			kdDebug(OSCAR_RAW_DEBUG) << "creation permissions " << t.data << endl;
			break;
		default:
			kdDebug(OSCAR_RAW_DEBUG) << "unknown TLV type " << t.type << endl;
			break;
		}
		realCount++;
	}
}

//kate: indent-mode csands; tab-width 4;
