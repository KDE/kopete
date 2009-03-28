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
#ifndef CHATROOMHANDLER_H
#define CHATROOMHANDLER_H

#include <QtCore/QObject>
#include "oscartypes.h"

#include "liboscar_export.h"

class ChatRoomTask;

class LIBOSCAR_EXPORT ChatRoomHandler : public QObject
{
	Q_OBJECT
public:
	ChatRoomHandler( ChatRoomTask* chatRoomTask );

	void send();

	QString internalId() const;
	QString contact() const;
	QString invite() const;
	Oscar::WORD exchange() const;
	QString room() const;

public Q_SLOTS:
	void reject();
	void accept();

signals:
	void joinChatRoom( const QString& roomName, int exchange );

private:
	ChatRoomTask* m_chatRoomTask;
};

#endif
