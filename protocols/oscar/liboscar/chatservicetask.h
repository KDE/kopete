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
// Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA
// 02110-1301  USA

#ifndef CHATSERVICETASK_H
#define CHATSERVICETASK_H

#include "task.h"

class Transfer;

class ChatServiceTask : public Task
{
Q_OBJECT
public:
    ChatServiceTask( Task* parent );
    ~ChatServiceTask();

    void onGo();
    bool take( Transfer* t );

    void parseRoomInfo();

    void parseJoinNotification();
    void parseLeftNotification();

    void parseChatMessage();
    void parseChatError();

    void sendChatMessage();

signals:
    void newChatMessage( Oscar::Message msg );

protected:
    bool forMe( const Transfer* t ) const;
};

#endif
