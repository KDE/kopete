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
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
// 02111-1307  USA


#ifndef SERVERREDIRECTTASK_H
#define SERVERREDIRECTTASK_H

#include "task.h"

#include <qcstring.h>

#include "oscartypes.h"

class Transfer;

class ServerRedirectTask : public Task
{
Q_OBJECT
public:
	ServerRedirectTask( Task* parent );

	void setService( WORD family );
    void setChatParams( WORD exchange, QByteArray cookie, WORD instance );
    void setChatRoom( const QString& roomName );

    WORD chatExchange() const;
    QString chatRoomName() const;

	//Task implementation
	void onGo();
	bool forMe( const Transfer* transfer );
	bool take( Transfer* transfer );

	void requestNewService();
	bool handleRedirect();

	QByteArray cookie() const;
	QString newHost() const;
	WORD service() const;

signals:
	void haveServer( const QString&, const QByteArray&, WORD );

private:
	WORD m_service;
	QString m_newHost;
	QByteArray m_cookie;

    WORD m_chatExchange;
    QByteArray m_chatCookie;
    WORD m_chatInstance;
    QString m_chatRoom;
};


#endif
