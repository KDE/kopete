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


#ifndef CHATROOMTASK_H
#define CHATROOMTASK_H

#include "task.h"

#include <QtCore/QFile>
#include <QtCore/QTimer>
#include <QtNetwork/QAbstractSocket>

class QTcpServer;
class QTcpSocket;

class KJob;
class Transfer;
namespace Oscar
{
	class Message;
}

class ChatRoomTask : public Task
{
Q_OBJECT
public:
	/** create an incoming chatroom request */
	ChatRoomTask( Task* parent, const QString& contact, const QString& self, QByteArray cookie, const QString& msg, const Oscar::WORD exchange, const QString& room );
	/** create an outgoing chatroom request */
	ChatRoomTask( Task* parent, const QString& contact, const QString& self, const QString& msg, const Oscar::WORD exchange, const QString& room );
	~ChatRoomTask();

	QString internalId() const;
	QString contactName() const;
	QString inviteMessage() const;
	Oscar::WORD exchange() const;
	QString room() const;

	//! Task implementation
	virtual bool take( Transfer* transfer );

protected:
	virtual void onGo();

public slots:
	void doInvite();
	void doReject();
	void doAccept();

signals:
	void joinChatRoom( const QString& roomName, int exchange );

private:
	QString m_contactName; //other person's username
	QString m_selfName; //my username
	QByteArray m_cookie;
	QString m_msg;
	Oscar::WORD m_exchange;
	QString m_room;
};

#endif
//kate: space-indent off; tab-width 4; indent-mode csands;
