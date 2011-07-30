/*
    Kopete Oscar Protocol
    Chat Room Handler

    Copyright 2009 Benson Tsai <btsai@vrwarp.com>

    Kopete ( c ) 2002-2009 by the Kopete developers <kopete-devel@kde.org>

    based on filetransferhandler.h and filetransferhandler.cpp

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or ( at your option ) any later version.    *
    *                                                                       *
    *************************************************************************
*/
#include "chatroomhandler.h"

#include "chatroomtask.h"

ChatRoomHandler::ChatRoomHandler( ChatRoomTask* chatRoomTask )
		: QObject( chatRoomTask ), m_chatRoomTask( chatRoomTask )
{
	connect( chatRoomTask, SIGNAL(joinChatRoom(QString,int)),
	         this, SIGNAL(joinChatRoom(QString,int)) );
}

QString ChatRoomHandler::internalId() const
{
	return m_chatRoomTask->internalId();
}

QString ChatRoomHandler::contact() const
{
	return m_chatRoomTask->contactName();
}

QString ChatRoomHandler::invite() const
{
	return m_chatRoomTask->inviteMessage();
}

Oscar::WORD ChatRoomHandler::exchange() const
{
	return m_chatRoomTask->exchange();
}

QString ChatRoomHandler::room() const
{
	return m_chatRoomTask->room();
}

void ChatRoomHandler::reject()
{
	m_chatRoomTask->doReject();
}

void ChatRoomHandler::accept()
{
	m_chatRoomTask->doAccept();
}

#include "chatroomhandler.moc"
