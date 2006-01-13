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
#include <kapplication.h>
#include <kdebug.h>
#include <qtextcodec.h>

#include "connection.h"
#include "transfer.h"
#include "buffer.h"
#include "oscartypes.h"

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

void ChatServiceTask::setEncoding( const QCString& enc )
{
    m_encoding = enc;
}

void ChatServiceTask::onGo()
{
    if ( !m_message )
    {
        setSuccess( true, QString::null );
        return;
    }

    kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "sending '" << m_message.textArray() << "' to the "
                             << m_room << " room" << endl;
    Buffer* b = new Buffer();
    b->addDWord( KApplication::random() ); //use kapp since it's convenient
    b->addDWord( KApplication::random() );
    b->addWord( 0x0003 ); //this be message channel 3 mateys! arrr!!
    b->addDWord( 0x00010000 ); //TLV 1 - this means it's a public message
    b->addDWord( 0x00060000 ); //TLV 6 - enables the server sending you your message back

    Buffer tlv5;
    TLV type2, type3, type1;

    type2.type = 0x0002;
    type2.length = 0x0008;
    type2.data = m_encoding;

    type3.type = 0x0003;
    type3.length = 0x0002;
    type3.data = QCString( "en" ); //hardcode for right now. don't know that we can do others

    type1.type = 0x0001;
    type1.length = m_message.textArray().size();
    type1.data = m_message.textArray();
    tlv5.addWord( 0x0005 );
    tlv5.addWord( 12 + type1.length + type2.length + type3.length );
    tlv5.addTLV( type1 );
    tlv5.addTLV( type2 );
    tlv5.addTLV( type3 );

    b->addString( tlv5.buffer(), tlv5.length() );

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
		kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Parse room info" << endl;
        parseRoomInfo();
		break;
	case 0x0003:
        kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "user joined notification" << endl;
        parseJoinNotification();
        break;
    case 0x0004:
        kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "user left notification" << endl;
        parseLeftNotification();
        break;
    case 0x0006:
        kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "message from room to client" << endl;
        parseChatMessage();
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
    WORD instance;
    BYTE detailLevel;
    Buffer* b = transfer()->buffer();

    m_exchange = b->getWord();
    QByteArray cookie( b->getBlock( b->getByte() ) );
    instance = b->getWord();

    detailLevel = b->getByte();

    //skip the tlv count, we don't care. Buffer::getTLVList() handles this all
    //correctly anyways
    b->skipBytes( 2 );

    QValueList<Oscar::TLV> tlvList = b->getTLVList();
    QValueList<Oscar::TLV>::iterator it = tlvList.begin();
    QValueList<Oscar::TLV>::iterator itEnd = tlvList.end();
    for ( ; it != itEnd; ++it )
    {
        switch ( ( *it ).type )
        {
        case 0x006A:
            m_internalRoom = QString( ( *it ).data );
            kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "room name: " << m_room << endl;
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
        kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "emitted userJoinedChat" << endl;
        emit userJoinedChat( m_exchange, m_room, sender );
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
        emit userLeftChat( m_exchange, m_room, sender );
    }
}

void ChatServiceTask::parseChatMessage()
{
    kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "have new chat room message" << endl;
    Buffer* b = transfer()->buffer();
    bool whisper = true, reflection = false;
    QByteArray language, encoding, message;
    QString sender;
    QByteArray icbmCookie( b->getBlock( 8 ) );
    b->skipBytes( 2 ); //message channel always 0x03
    QValueList<Oscar::TLV> chatTLVs = b->getTLVList();
    QValueList<Oscar::TLV>::iterator it,  itEnd = chatTLVs.end();
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
            kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "parsing the message" << endl;
            //oooh! look! more TLVS! i love those!
            Buffer b( ( *it ).data );
            while ( b.length() >= 4 )
            {
                TLV t = b.getTLV();
                switch( t.type )
                {
                case 0x0003:
                    language = t.data;
                    kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "language: " << language << endl;
                    break;
                case 0x0002:
                    encoding = t.data;
                    kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "encoding: " << encoding << endl;
                    break;
                case 0x0001:
                    message = t.data;
                    kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "message: " << message << endl;
                    break;
                }
            }
        }
        break;
        case 0x0003: //user info
        {
            Buffer b( ( *it ).data );
            sender = QString( b.getBUIN() );
            kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "got user info. sender is " << sender << endl;
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
    omessage.setType( 0x03 );
    omessage.setExchange( m_exchange );
    omessage.setChatRoom( m_room );
    emit newChatMessage( omessage );
}

void ChatServiceTask::parseChatError()
{

}


#include "chatservicetask.moc"

