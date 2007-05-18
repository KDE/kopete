// Kopete Oscar Protocol - Server redirections

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


#ifndef SERVERREDIRECTTASK_H
#define SERVERREDIRECTTASK_H

#include "task.h"
#include "oscartypes.h"

class Transfer;

class ServerRedirectTask : public Task
{
    Q_OBJECT
public:
    ServerRedirectTask( Task* parent );

    void setService( Oscar::WORD family );
    void setChatParams( Oscar::WORD exchange, QByteArray cookie, Oscar::WORD instance );
    void setChatRoom( const QString& roomName );

    Oscar::WORD chatExchange() const;
    QString chatRoomName() const;

    //Task implementation
    void onGo();
    bool forMe( const Transfer* transfer ) const;
    bool take( Transfer* transfer );

    void requestNewService();
    bool handleRedirect();

    QByteArray cookie() const;
    QString newHost() const;
    Oscar::WORD service() const;

signals:
	void haveServer( const QString&, const QByteArray&, Oscar::WORD );

private:
	Oscar::WORD m_service;
	QString m_newHost;
	QByteArray m_cookie;

    Oscar::WORD m_chatExchange;
    QByteArray m_chatCookie;
    Oscar::WORD m_chatInstance;
    QString m_chatRoom;
};


#endif
