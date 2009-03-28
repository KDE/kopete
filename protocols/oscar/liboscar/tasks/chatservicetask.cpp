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
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
// 02110-1301  USA


#include "chatservicetask.h"

#include <qstring.h>
#include <kdebug.h>
#include <qtextcodec.h>

#include "connection.h"
#include "transfer.h"
#include "buffer.h"
#include "oscartypes.h"
#include <QList>
#include <krandom.h>

ChatServiceTask::ChatServiceTask( Task* parent, Oscar::WORD exchange, const QString& room )
	: Task( parent ), m_encoding( "us-ascii" )
{
    m_exchange = exchange;
    m_room = room;
}

ChatServiceTask::~ChatServiceTask()
{

}

void ChatServiceTask::setMessage( const Oscar::Message& msg )
{
    m_message = msg;
}

void ChatServiceTask::setEncoding( const QByteArray& enc )
{
    m_encoding = enc;
}

void ChatServiceTask::onGo()
{
    if ( !m_message )
    {
        setSuccess( true, QString() );
        return;
    }

    kDebug(OSCAR_RAW_DEBUG) << "sending '" << m_message.textArray() << "' to the "
                             << m_room << " room" << endl;
    Buffer* b = new Buffer();
    b->addDWord( KRandom::random() ); //use kapp since it's convenient
    b->addDWord( KRandom::random() );
    b->addWord( 0x0003 ); //this be message channel 3 mateys! arrr!!
    b->addDWord( 0x00010000 ); //TLV 1 - this means it's a public message
    b->addDWord( 0x00060000 ); //TLV 6 - enables the server sending you your message back

    Buffer tlv5;
    TLV type2, type3, type1;

    type2.type = 0x0002;
    type2.length = m_encoding.length();
    type2.data = m_encoding;

    type3.type = 0x0003;
    type3.length = 0x0002;
    type3.data = "en"; //hardcode for right now. don't know that we can do others

    type1.type = 0x0001;
    type1.length = m_message.textArray().size();
    type1.data = m_message.textArray();
    tlv5.addWord( 0x0005 );
    tlv5.addWord( 12 + type1.length + type2.length + type3.length );
    tlv5.addTLV( type1 );
    tlv5.addTLV( type2 );
    tlv5.addTLV( type3 );

    b->addString( tlv5.buffer() );

    FLAP f = { 0x02, 0, 0 };
    SNAC s = { 0x000E, 0x0005, 0x0000, client()->snacSequence() };
    Transfer* t = createTransfer( f, s, b );
    send( t );
    setSuccess( true );
}

bool ChatServiceTask::forMe( const Transfer* t ) const
{
	const SnacTransfer* st = dynamic_cast<const SnacTransfer*>( t );
	if ( !st )
		return false;

	if ( st->snacService() != 0x000E )
		return false;

	switch ( st->snacSubtype() )
    {
    case 0x0003:
    case 0x0002:
    case 0x0006:
    case 0x0009:
    case 0x0004:
        return true;
        break;
    default:
		return false;
        break;
    }

	return true;
}

bool ChatServiceTask::take( Transfer* t )
{
	if ( !forMe( t ) )
		return false;

	SnacTransfer* st = dynamic_cast<SnacTransfer*>( t );
    if ( !st )
        return false;

    setTransfer( t );

	switch ( st->snacSubtype() )
	{
	case 0x0002:
		kDebug(OSCAR_RAW_DEBUG) << "Parse room info";
        parseRoomInfo();
		break;
	case 0x0003:
        kDebug(OSCAR_RAW_DEBUG) << "user joined notification";
        parseJoinNotification();
        break;
    case 0x0004:
        kDebug(OSCAR_RAW_DEBUG) << "user left notification";
        parseLeftNotification();
        break;
    case 0x0006:
        kDebug(OSCAR_RAW_DEBUG) << "message from room to client";
        parseChatMessage();
        break;
    case 0x0009:
        kDebug(OSCAR_RAW_DEBUG) << "chat error or data";
        break;
    };

    setSuccess( 0, QString() );
    setTransfer( 0 );
    return true;
}

void ChatServiceTask::parseRoomInfo()
{
    Oscar::WORD instance;
    Oscar::BYTE detailLevel;
    Buffer* b = transfer()->buffer();

    m_exchange = b->getWord();
    QByteArray cookie( b->getBlock( b->getByte() ) );
    instance = b->getWord();

    detailLevel = b->getByte();

    //skip the tlv count, we don't care. Buffer::getTLVList() handles this all
    //correctly anyways
    b->skipBytes( 2 );

    QList<Oscar::TLV> tlvList = b->getTLVList();
    QList<Oscar::TLV>::iterator it = tlvList.begin();
    QList<Oscar::TLV>::iterator itEnd = tlvList.end();
    for ( ; it != itEnd; ++it )
    {
        switch ( ( *it ).type )
        {
        case 0x006A:
            m_internalRoom = QString( ( *it ).data );
            kDebug(OSCAR_RAW_DEBUG) << "room name: " << m_room;
            break;
        case 0x006F:
            kDebug(OSCAR_RAW_DEBUG) << "num occupants: " << ( *it ).data;
            break;
        case 0x0073:
            kDebug(OSCAR_RAW_DEBUG) << "occupant list";
            break;
        case 0x00C9:
            kDebug(OSCAR_RAW_DEBUG) << "flags";
            break;
        case 0x00CA: //creation time
        case 0x00D1: //max message length
        case 0x00D3: //room description
        case 0x00D6: //encoding 1
        case 0x00D7: //language 1
        case 0x00D8: //encoding 2
        case 0x00D9: //language 2
        case 0x00DA: //maximum visible message length
            kDebug(OSCAR_RAW_DEBUG) << "unhandled TLV type " << ( *it ).type;
            break;
        default:
            kDebug(OSCAR_RAW_DEBUG) << "unknown TLV type " << ( *it ).type;
            break;
        }
    }
}

void ChatServiceTask::parseJoinNotification()
{
    Buffer* b = transfer()->buffer();
    while ( b->bytesAvailable() > 0 )
    {
        QString sender( b->getBUIN() );
        kDebug(OSCAR_RAW_DEBUG) << "user name:" << sender;
        Oscar::WORD warningLevel = b->getWord();
	Q_UNUSED(warningLevel);
        Oscar::WORD numTLVs = b->getWord();
        for ( int i = 0; i < numTLVs; i++ )
        {
            TLV t = b->getTLV();
            switch ( t.type )
            {
            case 0x0001:
                kDebug(OSCAR_RAW_DEBUG) << "user class: " << t.data;
                break;
            case 0x000F:
                kDebug(OSCAR_RAW_DEBUG) << "idle time: " << t.data;
                break;
            case 0x0003:
                kDebug(OSCAR_RAW_DEBUG) << "signon: " << t.data;
                break;
            }
        }
        kDebug(OSCAR_RAW_DEBUG) << "emitted userJoinedChat";
        emit userJoinedChat( m_exchange, m_room, sender );
    }

}

void ChatServiceTask::parseLeftNotification()
{
    Buffer* b = transfer()->buffer();
    while ( b->bytesAvailable() > 0 )
    {
        QString sender( b->getBUIN() );
        kDebug(OSCAR_RAW_DEBUG) << "user name:" << sender;
        Oscar::WORD warningLevel = b->getWord();
	Q_UNUSED(warningLevel);
        Oscar::WORD numTLVs = b->getWord();
        for ( int i = 0; i < numTLVs; i++ )
        {
            TLV t = b->getTLV();
            switch ( t.type )
            {
            case 0x0001:
                kDebug(OSCAR_RAW_DEBUG) << "user class: " << t.data;
                break;
            case 0x000F:
                kDebug(OSCAR_RAW_DEBUG) << "idle time: " << t.data;
                break;
            case 0x0003:
                kDebug(OSCAR_RAW_DEBUG) << "signon: " << t.data;
                break;
            }
        }
        emit userLeftChat( m_exchange, m_room, sender );
    }
}

void ChatServiceTask::parseChatMessage()
{
    kDebug(OSCAR_RAW_DEBUG) << "have new chat room message";
    Buffer* b = transfer()->buffer();
    bool whisper = true, reflection = false;
    QByteArray language, encoding, message;
    QString sender;
    QByteArray icbmCookie( b->getBlock( 8 ) );
    b->skipBytes( 2 ); //message channel always 0x03
    QList<Oscar::TLV> chatTLVs = b->getTLVList();
    QList<Oscar::TLV>::iterator it,  itEnd = chatTLVs.end();
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
            kDebug(OSCAR_RAW_DEBUG) << "parsing the message";
            //oooh! look! more TLVS! i love those!
            Buffer b( ( *it ).data );
            while ( b.bytesAvailable() >= 4 )
            {
                TLV t = b.getTLV();
                switch( t.type )
                {
                case 0x0003:
                    language = t.data;
                    kDebug(OSCAR_RAW_DEBUG) << "language: " << language;
                    break;
                case 0x0002:
                    encoding = t.data;
                    kDebug(OSCAR_RAW_DEBUG) << "encoding: " << encoding;
                    break;
                case 0x0001:
                    message = t.data;
                    kDebug(OSCAR_RAW_DEBUG) << "message: " << message;
                    break;
                }
            }
        }
        break;
        case 0x0003: //user info
        {
            Buffer b( ( *it ).data );
            sender = QString( b.getBUIN() );
            kDebug(OSCAR_RAW_DEBUG) << "got user info. sender is " << sender;
        }
        break;

        }
    }

    QTextCodec* codec = QTextCodec::codecForName( encoding );
    if ( ! codec )
        codec = QTextCodec::codecForMib( 4 );
    QString msgText( codec->toUnicode( message ) );
    Oscar::Message omessage;
    omessage.setReceiver( client()->userId() );
    omessage.setSender( sender );
    omessage.setTimestamp( QDateTime::currentDateTime() );
    omessage.setText( Oscar::Message::UTF8, msgText );
    omessage.setChannel( 0x03 );
    omessage.setExchange( m_exchange );
    omessage.setChatRoom( m_room );
    emit newChatMessage( omessage );
}

void ChatServiceTask::parseChatError()
{

}


#include "chatservicetask.moc"

