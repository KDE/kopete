// Kopete Oscar Protocol - chat service task

// Copyright (C)  2005	Matt Rogers <mattr@kde.org>

// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.

// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA
// 02110-1301  USA


#include "chatservicetask.h"

#include <kdebug.h>

#include "connection.h"
#include "transfer.h"
#include "buffer.h"
#include "oscartypes.h"

ChatServiceTask::ChatServiceTask( Task* parent )
	: Task( parent )
{

}

ChatServiceTask::~ChatServiceTask()
{

}

void ChatServiceTask::onGo()
{

}

bool ChatServiceTask::forMe( const Transfer* t ) const
{
	const SnacTransfer* st = dynamic_cast<const SnacTransfer*>( t );
	if ( !st )
		return false;

	if ( !st->snacService() != 0x000E )
		return false;

	if ( st->snacSubtype() == 0x0001 )
		return false;

	return true;
}

bool ChatServiceTask::take( Transfer* t )
{
	if ( !forMe( t ) )
		return false;

	setTransfer( t );
	SnacTransfer* st = dynamic_cast<SnacTransfer*>( t );
	switch ( st->snacSubtype() )
	{
	case 0x0002:
		kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Parse room info" << endl;
		break;
	case 0x0003:
        kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "user joined notification" << endl;
        break;
    case 0x0004:
        kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "user left notification" << endl;
        break;
    case 0x0006:
        kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "message from room to client" << endl;
        break;
    case 0x0009:
        kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "chat error or data" << endl;
        break;
    };

    setSuccess( 0, QString::null );

}

void ChatServiceTask::parseJoinNotification()
{
    Buffer* b = transfer()->buffer();
    while ( b->length() > 0 )
    {
        QString sender( b->getBUIN() );
        kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "user name:" << sender << endl;
        WORD warningLevel = b->getWord();
        WORD numTLVs = b->getWord();
        for ( int i = 0; i < numTLVs; i++ )
        {
            TLV t = b->getTLV();
            switch ( t.type )
            {
            case 0x0001:
                kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "user class: " << t.data << endl;
                break;
            case 0x000F:
                kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "idle time: " << t.data << endl;
                break;
            case 0x0003:
                kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "signon: " << t.data << endl;
                break;
            }
        }
    }
}

void ChatServiceTask::parseLeftNotification()
{
    Buffer* b = transfer()->buffer();
    while ( b->length() > 0 )
    {
        QString sender( b->getBUIN() );
        kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "user name:" << sender << endl;
        WORD warningLevel = b->getWord();
        WORD numTLVs = b->getWord();
        for ( int i = 0; i < numTLVs; i++ )
        {
            TLV t = b->getTLV();
            switch ( t.type )
            {
            case 0x0001:
                kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "user class: " << t.data << endl;
                break;
            case 0x000F:
                kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "idle time: " << t.data << endl;
                break;
            case 0x0003:
                kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "signon: " << t.data << endl;
                break;
            }
        }
    }
}

void ChatServiceTask::parseChatMessage()
{

}

void ChatServiceTask::parseChatError()
{

}

