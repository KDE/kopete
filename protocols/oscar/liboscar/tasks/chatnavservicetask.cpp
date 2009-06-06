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
#include <QList>


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

QList<int> ChatNavServiceTask::exchangeList() const
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
    while ( b->bytesAvailable() > 0 )
    {
        TLV t = b->getTLV();
        switch ( t.type )
        {
        case 0x0001:
            kDebug(OSCAR_RAW_DEBUG) << "got chat redirect TLV";
            break;
        case 0x0002:
        {
            kDebug(OSCAR_RAW_DEBUG) << "got max concurrent rooms TLV";
            Buffer tlvTwo(t.data);
            kDebug(OSCAR_RAW_DEBUG) << "max concurrent rooms is " << tlvTwo.getByte();
            break;
        }
        case 0x0003:
            kDebug(OSCAR_RAW_DEBUG) << "exchange info TLV found";
			handleExchangeInfo( t );
            //set the exchanges for the client
            emit haveChatExchanges( m_exchanges );
            break;
        case 0x0004:
            kDebug(OSCAR_RAW_DEBUG) << "room info TLV found";
            handleBasicRoomInfo( t );
            break;
        };
    }
    setSuccess( 0, QString() );
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

void ChatNavServiceTask::createRoom( Oscar::WORD exchange, const QString& name )
{
	//most of this comes from gaim. thanks to them for figuring it out
	QString cookie = "create"; //hardcoded, seems to be ignored by AOL
	QString lang = "en";
	QString charset = "us-ascii";

	FLAP f = { 0x02, 0, 0 };
	SNAC s = { 0x000D, 0x0008, 0x0000, client()->snacSequence() };
	Buffer *b = new Buffer;

	b->addWord( exchange );
	b->addBUIN( cookie.toLatin1() );
	b->addWord( 0xFFFF ); //assign the last instance
	b->addByte( 0x01 ); //detail level

	//just send three TLVs
	b->addWord( 0x0003 );

	//I'm lazy, add TLVs manually

	b->addWord( 0x00D3 ); //type of 0x00D3 - name
	b->addWord( name.length() );
	b->addString( name.toLatin1() );

	b->addWord( 0x00D6 ); //type of 0x00D6 - charset
	b->addWord( charset.length() );
	b->addString( charset.toLatin1() );

	b->addWord( 0x00D7 ); //type of 0x00D7 - lang
	b->addWord( lang.length() );
	b->addString( lang.toLatin1() );

    kDebug(OSCAR_RAW_DEBUG) << "sending join room packet";
	Transfer* t = createTransfer( f, s, b );
	send( t );
}


void ChatNavServiceTask::handleExchangeInfo( const TLV& t )
{
	kDebug(OSCAR_RAW_DEBUG) << "Parsing exchange info TLV";
	Buffer b(t.data);
    ChatExchangeInfo exchangeInfo;

	exchangeInfo.number = b.getWord();
    kDebug(OSCAR_RAW_DEBUG) << "exchange id is: " << exchangeInfo.number;
    b.getWord();
	while ( b.bytesAvailable() > 0 )
	{
		TLV t = b.getTLV();
        Buffer tmp = t.data;
		switch (t.type)
		{
		case 0x02:
			//kDebug(OSCAR_RAW_DEBUG) << "user class is " << t.data;
			break;
		case 0x03:
            exchangeInfo.maxRooms = tmp.getWord();
			kDebug(OSCAR_RAW_DEBUG) << "max concurrent rooms for the exchange is " << t.data;
			break;
		case 0x04:
            exchangeInfo.maxRoomNameLength = tmp.getWord();
			kDebug(OSCAR_RAW_DEBUG) << "max room name length is " << exchangeInfo.maxRoomNameLength;
			break;
		case 0x05:
			//kDebug(OSCAR_RAW_DEBUG) << "received root rooms info";
			break;
		case 0x06:
			//kDebug(OSCAR_RAW_DEBUG) << "received search tags";
			break;
		case 0xCA:
			//kDebug(OSCAR_RAW_DEBUG) << "have exchange creation time";
			break;
		case 0xC9:
			//kDebug(OSCAR_RAW_DEBUG) << "got chat flag";
			break;
		case 0xD0:
			//kDebug(OSCAR_RAW_DEBUG) << "got mandantory channels";
			break;
		case 0xD1:
            exchangeInfo.maxMsgLength = tmp.getWord();
			kDebug(OSCAR_RAW_DEBUG) << "max message length" << t.data;
			break;
		case 0xD2:
			kDebug(OSCAR_RAW_DEBUG) << "max occupancy" << t.data;
			break;
		case 0xD3:
		{
			QString eName( t.data );
			kDebug(OSCAR_RAW_DEBUG) << "exchange name: " << eName;
            exchangeInfo.description = eName;
			break;
		}
		case 0xD4:
			//kDebug(OSCAR_RAW_DEBUG) << "got optional channels";
			break;
		case 0xD5:
            exchangeInfo.canCreate = tmp.getByte();
			kDebug(OSCAR_RAW_DEBUG) << "creation permissions " << exchangeInfo.canCreate;
			break;
		default:
			kDebug(OSCAR_RAW_DEBUG) << "unknown TLV type " << t.type;
			break;
		}
	}

    m_exchanges.append( exchangeInfo.number );
}

void ChatNavServiceTask::handleBasicRoomInfo( const TLV& t )
{
	kDebug(OSCAR_RAW_DEBUG) << "Parsing room info TLV" << t.length;
	Buffer b(t.data);
    Oscar::WORD exchange = b.getWord();
    QByteArray cookie( b.getBlock( b.getByte() ) );
    Oscar::WORD instance = b.getWord();
    b.getByte(); //detail level, which I'm not sure we need
    Oscar::WORD tlvCount = b.getWord();
    Q_UNUSED(tlvCount);

    kDebug(OSCAR_RAW_DEBUG) << "e: " << exchange
                             << " c: " << cookie << " i: " << instance << endl;

    QList<Oscar::TLV> tlvList = b.getTLVList();
    QList<Oscar::TLV>::iterator it, itEnd = tlvList.end();
    QString roomName;
    for ( it = tlvList.begin(); it != itEnd; ++it )
    {
        TLV t = ( *it );
		switch (t.type)
		{
		case 0x66:
			kDebug(OSCAR_RAW_DEBUG) << "user class is " << t.data;
			break;
		case 0x67:
			kDebug(OSCAR_RAW_DEBUG) << "user array";
			break;
		case 0x68:
			kDebug(OSCAR_RAW_DEBUG) << "evil generated" << t.data;
			break;
		case 0x69:
			kDebug(OSCAR_RAW_DEBUG) << "evil generated array";
			break;
		case 0x6A:
            roomName = QString( t.data );
			kDebug(OSCAR_RAW_DEBUG) << "fully qualified name" << roomName;
			break;
		case 0x6B:
			kDebug(OSCAR_RAW_DEBUG) << "moderator";
			break;
		case 0x6D:
			kDebug(OSCAR_RAW_DEBUG) << "num children";
			break;
		case 0x06F:
			kDebug(OSCAR_RAW_DEBUG) << "occupancy";
			break;
		case 0x71:
			kDebug(OSCAR_RAW_DEBUG) << "occupant evil";
			break;
		case 0x75:
			kDebug(OSCAR_RAW_DEBUG) << "room activity";
			break;
		case 0xD0:
			kDebug(OSCAR_RAW_DEBUG) << "got mandantory channels";
			break;
		case 0xD1:
			kDebug(OSCAR_RAW_DEBUG) << "max message length" << t.data;
			break;
		case 0xD2:
			kDebug(OSCAR_RAW_DEBUG) << "max occupancy" << t.data;
			break;
		case 0xD3:
			kDebug(OSCAR_RAW_DEBUG) << "exchange name";
			break;
		case 0xD4:
			kDebug(OSCAR_RAW_DEBUG) << "got optional channels";
			break;
		case 0xD5:
			kDebug(OSCAR_RAW_DEBUG) << "creation permissions " << t.data;
			break;
		default:
			kDebug(OSCAR_RAW_DEBUG) << "unknown TLV type " << t.type;
			break;
		}
	}

    emit connectChat( exchange, cookie, instance, roomName );
}

void ChatNavServiceTask::handleCreateRoomInfo( const TLV& t )
{
	Buffer b( t.data );
	Oscar::WORD exchange = b.getWord();
	Oscar::WORD cookieLength = b.getByte();
	QByteArray cookie( b.getBlock( cookieLength ) );
	Oscar::WORD instance = b.getWord();
	Oscar::BYTE detailLevel = b.getByte();

	//clear compiler warnings
	Q_UNUSED(exchange);
	Q_UNUSED(instance);

	if ( detailLevel != 0x02 )
	{
		kWarning(OSCAR_RAW_DEBUG) << "unknown detail level in response";
		return;
	}

	Oscar::WORD numberTlvs = b.getWord();
	Q_UNUSED(numberTlvs);
	QList<Oscar::TLV> roomTLVList = b.getTLVList();
	QList<Oscar::TLV>::iterator itEnd = roomTLVList.end();
	for ( QList<Oscar::TLV>::iterator it = roomTLVList.begin();
		  it != itEnd; ++ it )
	{
		switch( ( *it ).type )
		{
		case 0x006A:
		{
			QString fqcn = QString( ( *it ).data );
			kDebug(OSCAR_RAW_DEBUG) << "fqcn: " << fqcn;
			break;
		}
		case 0x00C9:
			kDebug(OSCAR_RAW_DEBUG) << "flags: " << t.data;
			break;
		case 0x00CA:
			kDebug(OSCAR_RAW_DEBUG) << "create time: " << t.data;
			break;
		case 0x00D1:
			kDebug(OSCAR_RAW_DEBUG) << "max msg len: " << t.data;
			break;
		case 0x00D2:
			kDebug(OSCAR_RAW_DEBUG) << "max occupancy: " << t.data;
			break;
		case 0x00D3:
			kDebug(OSCAR_RAW_DEBUG) << "name: " << QString( t.data );
			break;
		case 0x00D5:
			kDebug(OSCAR_RAW_DEBUG) << "create perms: " << t.data;
			break;
		};
	}
}

#include "chatnavservicetask.moc"
//kate: indent-mode csands; tab-width 4;
