/***************************************************************************
                          oncomingsocket.cpp  -  description
                             -------------------
    begin                : Sun Jul 28 2002
    copyright            : (C) 2002 by twl6
    email                : twl6@paranoia.STUDENT.CWRU.Edu
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <kdebug.h>
#include "oncomingsocket.h"
#include "oncomingsocket.moc"

OncomingSocket::OncomingSocket(QObject *parent, const char *name )
: QServerSocket(0,5,parent,name)
{
}

OncomingSocket::OncomingSocket(QList<ServiceSocket> *socketz, const QHostAddress &address,
	Q_UINT16 port, int backlog, QObject *parent, const char *name)
: QServerSocket(address,port,backlog,parent,name)
{
	conns = socketz;
}

OncomingSocket::~OncomingSocket()
{
}

/** Called when someone connects to the serversocket */
void OncomingSocket::newConnection( int socket )
{
	ServiceSocket *newsock = new ServiceSocket;
	newsock->setSocket(socket);
	if (conns)
		conns->append(newsock);
	else
		kdDebug() << "[Oscar] Oncomingsocket::newConnection: conns is NULL!" << endl;
	kdDebug() << "[Oscar][OncomingSocket]newConnection called!  socket " << socket << endl;
}
