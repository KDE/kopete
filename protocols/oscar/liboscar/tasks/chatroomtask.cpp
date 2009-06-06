/*
    Kopete Oscar Protocol
    Chat Room Task

    Copyright 2009 Benson Tsai <btsai@vrwarp.com>

    Kopete ( c ) 2002-2009 by the Kopete developers <kopete-devel@kde.org>

    based on filetransfertask.h and filetransfertask.cpp

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or ( at your option ) any later version.    *
    *                                                                       *
    *************************************************************************
*/

#include "chatroomtask.h"

#include <QString>
#include <QtCore/QFileInfo>
#include <QtCore/QTextCodec>
#include <QtNetwork/QHostAddress>
#include <QtNetwork/QTcpServer>
#include <QtNetwork/QTcpSocket>
#include <QtXml/QDomDocument>

#include <ksocketfactory.h>
#include <krandom.h>
#include <kio/global.h>

#include <klocale.h>
#include <kdebug.h>

#include "buffer.h"
#include "oscarutils.h"
#include "oscarmessage.h"
#include "connection.h"
#include "oscarsettings.h"
#include "oftmetatransfer.h"

//receive
ChatRoomTask::ChatRoomTask(Task* parent, const QString& contact, const QString& self, QByteArray cookie, const QString& msg, const Oscar::WORD exchange, const QString& room)
: Task(parent), m_contactName(contact), m_selfName(self), m_cookie(cookie), m_msg(msg), m_exchange(exchange), m_room(room)
{
}

//send
ChatRoomTask::ChatRoomTask(Task* parent, const QString& contact, const QString& self, const QString& msg, const Oscar::WORD exchange, const QString& room)
: Task(parent), m_contactName(contact), m_selfName(self), m_msg(msg), m_exchange(exchange), m_room(room)
{
	Buffer b;
	Oscar::DWORD cookie = KRandom::random();
	b.addDWord( cookie );
	cookie = KRandom::random();
	b.addDWord( cookie );
	m_cookie = b.buffer();
}

ChatRoomTask::~ChatRoomTask()
{
	kDebug(OSCAR_RAW_DEBUG) << "done";
}

QString ChatRoomTask::internalId() const
{
	return QString( m_cookie.toHex() );
}

QString ChatRoomTask::contactName() const
{
	return m_contactName;
}

QString ChatRoomTask::inviteMessage() const
{
	return m_msg;
}

Oscar::WORD ChatRoomTask::exchange() const
{
	return m_exchange;
}

QString ChatRoomTask::room() const
{
	return m_room;
}

void ChatRoomTask::onGo()
{
	return;
}

bool ChatRoomTask::take( Transfer* transfer )
{
	Q_UNUSED(transfer);
	return false;
}

void ChatRoomTask::doInvite()
{
	Buffer* b = new Buffer();

	b->addString( m_cookie, 8 );
	b->addWord( 0x0002 );
	// nick name
	b->addByte( m_contactName.toUtf8().length() );
	b->addString( m_contactName.toUtf8() );

	// start creating rendezvous tlv data
	Buffer b2;

	// Message type Request
	b2.addWord( 0x0000 );
	// Repeat cookie
	b2.addString( m_cookie, 8 );
	// Add chatroom GUID
	b2.addString( oscar_caps[CAP_CHAT] );

	// unknown
	b2.addTLV( 0x000a, QByteArray( "\x00\x01", 2 ) );
	// unknown
	b2.addTLV( 0x000f, QByteArray() );
	// char set
	b2.addTLV( 0x000e, QByteArray( "\x30", 1 ) );
	// language
	b2.addTLV( 0x000d, QByteArray( "us-ascii" ) );
	// Add invite message
	b2.addTLV( 0x000c, m_msg.toUtf8() );

	// start creating extended tlv data
	Buffer b3;

	// unknown 4 bytes
	b3.addWord( 0x0004 );
	b3.addWord( 0x2321 );

	QString joinStr = "aol://2719:10-";
	joinStr.append( QString::number( m_exchange ) );
	joinStr.append( "-" );
	joinStr.append( m_room );
	b3.addString( joinStr.toUtf8() );
	b3.addWord( 0x0000 );

	// add extended data
	b2.addTLV( 0x2711, b3 );

	// add rendezvous data
	b->addTLV( 0x0005, b2 );

	// request ack
	b->addTLV( 0x0003, QByteArray() );

	FLAP f = { 0x02, 0, 0 };
	SNAC s = { 0x0004, 0x0006, 0x0000, client()->snacSequence() };
	Transfer* t = createTransfer( f, s, b );
	send( t );
	setSuccess( true );
}

void ChatRoomTask::doReject()
{
	kDebug() << "invitation to join chat " << m_room << " rejected!";

	Buffer* b = new Buffer();
	b->addString( m_cookie, 8 );
	b->addWord( 0x0002 );
	// nick name
	b->addByte( m_contactName.toUtf8().length() );
	b->addString( m_contactName.toUtf8() );
	// rejection!
	b->addWord( 0x0003 );
	b->addWord( 0x0002 );
	b->addWord( 0x0001 );

	FLAP f = { 0x02, 0, 0 };
	SNAC s = { 0x0004, 0x000B, 0x0000, client()->snacSequence() };
	Transfer* t = createTransfer( f, s, b );
	send( t );
	setSuccess( true );
}

void ChatRoomTask::doAccept()
{
	kDebug() << "invitation to join chat " << m_room << " accepted!";
	emit joinChatRoom( m_room, m_exchange );
	setSuccess( true );
}


#include "chatroomtask.moc"
//kate: space-indent off; tab-width 4; indent-mode csands;

