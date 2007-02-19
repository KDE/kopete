// Kopete Oscar Protocol - Chat service handling

// Copyright (C)  2005  Matt Rogers <mattr@kde.org>

// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.

// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
// 02110-1301  USA

#ifndef CHATSERVICETASK_H
#define CHATSERVICETASK_H

#include "task.h"
#include "oscarmessage.h"
#include <QByteArray>

class Transfer;

class ChatServiceTask : public Task
{
Q_OBJECT
public:
    ChatServiceTask( Task* parent, Oscar::WORD exchange, const QString& room );
    ~ChatServiceTask();

    void onGo();
    bool take( Transfer* t );

    void parseRoomInfo();

    void parseJoinNotification();
    void parseLeftNotification();

    void parseChatMessage();
    void parseChatError();

    void setMessage( const Oscar::Message& msg );
    void setEncoding( const QByteArray &enc );

signals:
    void userJoinedChat( Oscar::WORD, const QString& r, const QString& u );
    void userLeftChat( Oscar::WORD, const QString& r, const QString& u );
    void newChatMessage( const Oscar::Message& msg );

protected:
    bool forMe( const Transfer* t ) const;

private:
    Oscar::WORD m_exchange;
    QString m_room;
    QString m_internalRoom;
    Oscar::Message m_message;
    QByteArray m_encoding;
};

#endif
