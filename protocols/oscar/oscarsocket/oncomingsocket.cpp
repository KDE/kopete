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
#include "oscarsocket.h"
#include "oscardirectconnection.h"
#include "oncomingsocket.h"
#include "oncomingsocket.moc"

OncomingSocket::OncomingSocket(QObject *parent, const char *name )
: QServerSocket(0,5,parent,name)
{
}

OncomingSocket::OncomingSocket(OscarSocket *server, const QHostAddress &address,
	Q_UINT16 port, int backlog, QObject *parent, const char *name)
: QServerSocket(address,port,backlog,parent,name)
{
	mServer = server;
  mConns.setAutoDelete(TRUE);
  mPendingConnections.setAutoDelete(TRUE);
}

OncomingSocket::~OncomingSocket()
{
	mConns.clear();
	mPendingConnections.clear();
}

/** Called when someone connects to the server socket */
void OncomingSocket::newConnection( int socket )
{
	for (DirectInfo *tmp=mPendingConnections.first(); tmp; tmp = mPendingConnections.next())
	{
  }
	// TODO: fix this later so that it searches the whole list
  DirectInfo *tmp = mPendingConnections.first();
	OscarDirectConnection *newsock = new OscarDirectConnection(mServer->getSN(), tmp->sn, tmp->cookie);
	setupConnection(newsock);
	newsock->setSocket(socket);
	mPendingConnections.remove(tmp);

	kdDebug() << "[Oscar][OncomingSocket]newConnection called!  socket " << socket << endl;
}

/** Finds the connection named name and returns a pointer to it.
		If no such connection is found, return NULL */
OscarDirectConnection * OncomingSocket::findConnection(const QString &name)
{
	OscarDirectConnection *tmp;
	for (tmp = mConns.first(); tmp; tmp = mConns.next())
	{
		if ( !name.compare(tmp->connectionName()) )
		{
			return tmp;
		}
	}
	return 0L;

}

/** Adds the connection to the list of pending connections */
void OncomingSocket::addPendingConnection(const QString &sn, char cookie[8])
{
	DirectInfo *ninfo = new DirectInfo;
	for (int i=0;i<8;i++)
		ninfo->cookie[i] = cookie[i];
	ninfo->sn = sn;
	mPendingConnections.append(ninfo);
}

/** Called when a connection is ready */
void OncomingSocket::slotConnectionReady(QString name)
{
	OscarDirectConnection *dc = findConnection(name);
	if (!dc)
	{
		kdDebug() << "[OncomingSocket] Connection " << name << " not found!!!  exiting slotConnectionReady()" << endl;
		return;
	}

	kdDebug() << "[OncomingSocket] slotConnectionReady(): Setting up direct IM signals!" << endl;
  // Connect protocol error signal
	QObject::connect(dc, SIGNAL(protocolError(QString, int)),
			mServer, SLOT(OnDirectIMError(QString, int)));
	// Got IM
	QObject::connect(dc, SIGNAL(gotIM(QString, QString, bool)),
			mServer, SLOT(OnDirectIMReceived(QString,QString,bool)));
	// Disconnected
	QObject::connect(dc, SIGNAL(connectionClosed(QString)),
			mServer, SLOT(OnDirectIMConnectionClosed(QString)));
			
	QObject::connect(dc, SIGNAL(connectionClosed(QString)),
			this, SLOT(slotConnectionClosed(QString)));
	// Typing notification
	QObject::connect(dc, SIGNAL(gotMiniTypeNotification(QString,int)),
			mServer, SLOT(OnDirectMiniTypeNotification(QString, int)));
}

/** Set up a connection before adding it to the list of connections */
void OncomingSocket::setupConnection(OscarDirectConnection *newsock)
{
	if (mServer)
		newsock->setDebugDialog(mServer->debugDialog());

	// Ready signal
	QObject::connect(newsock, SIGNAL(directIMReady(QString)),
		this, SLOT(slotConnectionReady(QString)));
	QObject::connect(newsock, SIGNAL(directIMReady(QString)),
		mServer, SLOT(OnDirectIMReady(QString)));

	mConns.append(newsock);
}

/** Adds an outgoing connection to the list and attempts to connect */
void OncomingSocket::addOutgoingConnection(const QString &sn, char * cook, const QString &host, int port)
{
	char ck[8];
	for (int i=0;i<8;i++)
		ck[i] = cook[i];
	addPendingConnection(sn, ck);
	OscarDirectConnection *s = new OscarDirectConnection(mServer->getSN(), sn, ck);
	setupConnection(s);
	kdDebug() << "[OncomingSocket] Connecting to " << host << ":" << port << endl;
	s->connectToHost(host,port);
}

/** Called when connection named name has been closed */
void OncomingSocket::slotConnectionClosed(QString name)
{
	kdDebug() << "[OncomingSocket] direct connection closed, deleting it: " << name << endl;
	removeConnection(name);
}

/** Removes the named connection from the connection list and disconnects it. */
void OncomingSocket::removeConnection(const QString &name)
{
	kdDebug() << "[OncomingSocket] deleting direct connection " << name << endl;
	OscarDirectConnection *dc = findConnection(name);
	if ( !dc )
	{
		kdDebug() << "[OncomingSocket] no connection to delete" << endl;
		return;
	}
	mConns.remove(dc);
}

