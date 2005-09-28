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
#include "task.h"
#include "client.h"
#include "connection.h"


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

QValueList<int> ChatNavServiceTask::exchangeList() const
{
    return m_exchanges;
}

bool ChatNavServiceTask::forMe( const Transfer* transfer ) const
{
	const SnacTransfer* st = dynamic_cast<const SnacTransfer*>( transfer );
	if ( !st )
		return false;
	if ( st->snacService() == 0x000D && st->snacSubtype() == 0x0009 )
		return true;

	return false;
}

bool ChatNavServiceTask::take( Transfer* transfer )
{
	if ( !forMe( transfer ) )
		return false;

	setTransfer( transfer );
	Buffer* b = transfer->buffer();
    while ( b->length() > 0 )
    {
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
			handleExchangeInfo( t );
            //set the exchanges for the client
            emit haveChatExchanges( m_exchanges );
            break;
        case 0x0004:
            kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "room info TLV found" << endl;
            handleBasicRoomInfo( t );
            break;
        };
    }


    setSuccess( 0, QString::null );
    setTransfer( 0 );
	return true;

}

void ChatNavServiceTask::onGo()
{
    FLAP f =  { 0x02, 0, 0x00 };
    SNAC s = { 0x000D, m_type, 0x0000, client()->snacSequence() };
    Buffer* b = new Buffer();

    Transfer* t = createTransfer( f, s, b );
    send( t );
}

void ChatNavServiceTask::createRoom( WORD exchange, const QString& name )
{
	//most of this comes from gaim. thanks to them for figuring it out
	QString cookie = "create"; //hardcoded, seems to be ignored by AOL
	QString lang = "en";
	QString charset = "us-ascii";

	FLAP f = { 0x02, 0, 0 };
	SNAC s = { 0x000D, 0x0008, 0x0000, client()->snacSequence() };
	Buffer *b = new Buffer;

	b->addWord( exchange );
	b->addBUIN( cookie.latin1() );
	b->addWord( 0xFFFF ); //assign the last instance
	b->addByte( 0x01 ); //detail level

	//just send three TLVs
	b->addWord( 0x0003 );

	//i'm lazy, add TLVs manually

	b->addWord( 0x00D3 ); //type of 0x00D3 - name
	b->addWord( name.length() );
	b->addString( name.latin1(), name.length() );

	b->addWord( 0x00D6 ); //type of 0x00D6 - charset
	b->addWord( charset.length() );
	b->addString( charset.latin1(), charset.length() );

	b->addWord( 0x00D7 ); //type of 0x00D7 - lang
	b->addWord( lang.length() );
	b->addString( lang.latin1(), lang.length() );

    kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "sending join room packet" << endl;
	Transfer* t = createTransfer( f, s, b );
	send( t );
}


void ChatNavServiceTask::handleExchangeInfo( const TLV& t )
{
	kdDebug(OSCAR_RAW_DEBUG) << "Parsing exchange info TLV" << endl;
	Buffer b(t.data);
    ChatExchangeInfo exchangeInfo;

	exchangeInfo.number = b.getWord();
    kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "exchange id is: " << exchangeInfo.number << endl;
    b.getWord();
	while ( b.length() > 0 )
	{
		TLV t = b.getTLV();
        Buffer tmp = t.data;
		switch (t.type)
		{
		case 0x02:
			//kdDebug(OSCAR_RAW_DEBUG) << "user class is " << t.data << endl;
			break;
		case 0x03:
            exchangeInfo.maxRooms = tmp.getWord();
			kdDebug(OSCAR_RAW_DEBUG) << "max concurrent rooms for the exchange is " << t.data << endl;
			break;
		case 0x04:
            exchangeInfo.maxRoomNameLength = tmp.getWord();
			kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "max room name length is " << exchangeInfo.maxRoomNameLength << endl;
			break;
		case 0x05:
			//kdDebug(OSCAR_RAW_DEBUG) << "received root rooms info" << endl;
			break;
		case 0x06:
			//kdDebug(OSCAR_RAW_DEBUG) << "received search tags" << endl;
			break;
		case 0xCA:
			//kdDebug(OSCAR_RAW_DEBUG) << "have exchange creation time" << endl;
			break;
		case 0xC9:
			//kdDebug(OSCAR_RAW_DEBUG) << "got chat flag" << endl;
			break;
		case 0xD0:
			//kdDebug(OSCAR_RAW_DEBUG) << "got mandantory channels" << endl;
			break;
		case 0xD1:
            exchangeInfo.maxMsgLength = tmp.getWord();
			kdDebug(OSCAR_RAW_DEBUG) << "max message length" << t.data << endl;
			break;
		case 0xD2:
			kdDebug(OSCAR_RAW_DEBUG) << "max occupancy" << t.data << endl;
			break;
		case 0xD3:
		{
			QString eName( t.data );
			kdDebug(OSCAR_RAW_DEBUG) << "exchange name: " << eName << endl;
            exchangeInfo.description = eName;
			break;
		}
		case 0xD4:
			//kdDebug(OSCAR_RAW_DEBUG) << "got optional channels" << endl;
			break;
		case 0xD5:
            exchangeInfo.canCreate = tmp.getByte();
			kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "creation permissions " << exchangeInfo.canCreate << endl;
			break;
		default:
			kdDebug(OSCAR_RAW_DEBUG) << "unknown TLV type " << t.type << endl;
			break;
		}
	}
    m_exchanges.append( exchangeInfo.number );
}

void ChatNavServiceTask::handleBasicRoomInfo( const TLV& t )
{
	kdDebug(OSCAR_RAW_DEBUG) << "Parsing room info TLV" << t.length << endl;
	Buffer b(t.data);
    WORD exchange = b.getWord();
    QByteArray cookie( b.getBlock( b.getByte() ) );
    WORD instance = b.getWord();
    b.getByte(); //detail level, which i'm not sure we need
    WORD tlvCount = b.getWord();
    kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "e: " << exchange
                             << " c: " << cookie << " i: " << instance << endl;

    QValueList<Oscar::TLV> tlvList = b.getTLVList();
    QValueList<Oscar::TLV>::iterator it, itEnd = tlvList.end();
    QString roomName;
    for ( it = tlvList.begin(); it != itEnd; ++it )
    {
        TLV t = ( *it );
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
            roomName = QString( t.data );
			kdDebug(OSCAR_RAW_DEBUG) << "fully qualified name" << roomName << endl;
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
	}

    emit connectChat( exchange, cookie, instance, roomName );
}

void ChatNavServiceTask::handleCreateRoomInfo( const TLV& t )
{
	Buffer b( t.data );
	WORD exchange = b.getWord();
	WORD cookieLength = b.getByte();
	QByteArray cookie( b.getBlock( cookieLength ) );
	WORD instance = b.getWord();
	BYTE detailLevel = b.getByte();

	if ( detailLevel != 0x02 )
	{
		kdWarning(OSCAR_RAW_DEBUG) << k_funcinfo << "unknown detail level in response" << endl;
		return;
	}

	WORD numberTlvs = b.getWord();
	QValueList<Oscar::TLV> roomTLVList = b.getTLVList();
	QValueList<Oscar::TLV>::iterator itEnd = roomTLVList.end();
	for ( QValueList<Oscar::TLV>::iterator it = roomTLVList.begin();
		  it != itEnd; ++ it )
	{
		switch( ( *it ).type )
		{
		case 0x006A:
		{
			QString fqcn = QString( ( *it ).data );
			kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "fqcn: " << fqcn << endl;
			break;
		}
		case 0x00C9:
			kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "flags: " << t.data << endl;
			break;
		case 0x00CA:
			kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "create time: " << t.data << endl;
			break;
		case 0x00D1:
			kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "max msg len: " << t.data << endl;
			break;
		case 0x00D2:
			kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "max occupancy: " << t.data << endl;
			break;
		case 0x00D3:
			kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "name: " << QString( t.data ) << endl;
			break;
		case 0x00D5:
			kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "create perms: " << t.data << endl;
			break;
		};
	}
}

#include "chatnavservicetask.moc"
//kate: indent-mode csands; tab-width 4;
