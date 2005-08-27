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
//Added by qt3to4:
#include <Q3ValueList>

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
    setTransfer( 0 );
    return true;
}

void ChatServiceTask::parseRoomInfo()
{
    WORD exchange, instance;
    BYTE detailLevel;
    Buffer* b = transfer()->buffer();

    exchange = b->getWord();
    QString name( b->getBUIN() );
    instance = b->getByte();

    detailLevel = b->getByte();

    //skip the tlv count, we don't care. Buffer::getTLVList() handles this all
    //correctly anyways
    b->skipBytes( 2 );

    Q3ValueList<Oscar::TLV> tlvList = b->getTLVList();
    Q3ValueList<Oscar::TLV>::iterator it = tlvList.begin();
    Q3ValueList<Oscar::TLV>::iterator itEnd = tlvList.end();
    for ( ; it != itEnd; ++it )
    {
        switch ( ( *it ).type )
        {
        case 0x006A:
            kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "room name: " << QString( ( *it ).data ) << endl;
            break;
        case 0x006F:
            kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "num occupants: " << ( *it ).data << endl;
            break;
        case 0x0073:
            kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "occupant list" << endl;
            break;
        case 0x00C9:
            kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "flags" << endl;
            break;
        case 0x00CA: //creation time
        case 0x00D1: //max message length
        case 0x00D3: //room description
        case 0x00D6: //encoding 1
        case 0x00D7: //language 1
        case 0x00D8: //encoding 2
        case 0x00D9: //language 2
        case 0x00DA: //maximum visible message length
            kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "unhandled TLV type " << ( *it ).type << endl;
            break;
        default:
            kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "unknown TLV type " << ( *it ).type << endl;
            break;
        }
    }
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
    Buffer* b = transfer()->buffer();
    bool whisper = true, reflection = false;
    QString language, encoding, message;
    QByteArray icbmCookie( b->getBlock( 8 ) );
    b->skipBytes( 2 ); //message channel always 0x03
    Q3ValueList<Oscar::TLV> chatTLVs = b->getTLVList();
    Q3ValueList<Oscar::TLV>::iterator it,  itEnd = chatTLVs.end();
    for ( it = chatTLVs.begin(); it != itEnd; ++it )
    {
        switch ( ( *it ).type )
        {
        case 0x0001: //if present, message was sent to the room
            whisper = false;
            break;
        case 0x0006: //enable reflection
            reflection = true;
            break;
        case 0x0005: //the good stuff - the actual message
        {
            //oooh! look! more TLVS! i love those!
            Buffer b( ( *it ).data );
            Q3ValueList<Oscar::TLV> messageTLVs = b.getTLVList();
            Q3ValueList<Oscar::TLV>::iterator mit,  mitEnd = messageTLVs.end();
            for ( mit = messageTLVs.begin(); mit != mitEnd; ++mit )
            {
                switch( ( *it ).type )
                {
                case 0x0003:
                    language = QString( ( *it ).data );
                    kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "language: " << language << endl;
                    break;
                case 0x0002:
                    encoding = QString( ( *it ).data );
                    kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "encoding: " << encoding << endl;
                    break;
                case 0x0001:
                    message = QString( ( *it ).data );
                    kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "message: " << message << endl;
                    break;
                }
            }
        }
        break;
        case 0x0003: //user info
            kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "got user info" << endl;
            break;
        }
    }

    Oscar::Message omessage;
    omessage.setReceiver( client()->userId() );
    omessage.setTimestamp( QDateTime::currentDateTime() );
    omessage.setText( message );
    omessage.setType( 0x03 );
}

void ChatServiceTask::parseChatError()
{

}


#include "chatservicetask.moc"

