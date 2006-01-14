// aimchatsession.h
// Copyright (C)  2005  Matt Rogers <mattr@kde.org>

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA
// 02110-1301, USA.

#ifndef AIMCHATSESSION_H
#define AIMCHATSESSION_H

#include "kopetechatsession.h"
#include "oscartypes.h"

class Client;

class AIMChatSession : public Kopete::ChatSession
{
Q_OBJECT
public:
    AIMChatSession( const Kopete::Contact* contact, Kopete::ContactPtrList others,
                    Kopete::Protocol* protocol, Oscar::WORD exchange = 0,
                    const QString& room = QString::null );
    virtual ~AIMChatSession();

    /**
     * Set the engine to use so that we can disconnect from the chat service
     * properly
     */
    void setEngine( Client* engine );

    /**
     * Get the name of the AIM chat room represented by
     * this ChatSession object
     * @return the name of the chat room
     */
    QString roomName() const;

    /**
     * Set the name of the AIM chat room represented by
     * this ChatSession object
     * @param room the name of the AIM chat room
     */
    void setRoomName( const QString& room );

    /**
     * Get the exchange of the AIM chat room represented by
     * this ChatSession object
     * @return the exchange of the chat room
     */
    Oscar::WORD exchange() const;

    /**
     * Set the exchange of the AIM chat room represented by
     * this ChatSession object
     * @param exchange the exchange of the AIM chat room
     */
    void setExchange( Oscar::WORD exchange );

private:
    QString m_roomName;
    Oscar::WORD m_exchange;
    Client* m_engine;
};


#endif
