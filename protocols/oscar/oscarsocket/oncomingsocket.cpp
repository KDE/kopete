/***************************************************************************
                          oncomingsocket.cpp  -  description
                             -------------------
    begin                : Sun Jul 28 2002

    Copyright (c) 2002 by Tom Linsky <twl6@po.cwru.edu>
    Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include <kdebug.h>

#include "oscarsocket.h"
#include "oscarfilesendconnection.h"
#include "oncomingsocket.h"
#include "aim.h"

OncomingSocket::OncomingSocket(QObject *parent, const char *name ) : QObject( parent, name )
{
}

OncomingSocket::OncomingSocket(OscarSocket *server, const QString &address,
	OscarConnection::ConnectionType type,
	Q_UINT16 port,
	int /* backlog */,
	QObject *parent,
	const char *name) : QObject( parent, name )
{
	mType = type;
	mServer = server;
	mConns.setAutoDelete(TRUE);
	mPendingConnections.setAutoDelete(TRUE);

	mSocket = new KExtendedSocket(address, port,
		KExtendedSocket::inetSocket | KExtendedSocket::passiveSocket);
}

OncomingSocket::~OncomingSocket()
{
	mConns.clear();
	for ( DirectInfo *tmp = mPendingConnections.first(); tmp; tmp = mPendingConnections.next() )
		delete tmp->finfo;

	mPendingConnections.clear();
	delete mSocket;
}

// Called when someone connects to the server socket

void OncomingSocket::newConnection()
{
	kdDebug(14151) << k_funcinfo << "Called!" << endl;

	for (DirectInfo *tmp=mPendingConnections.first(); tmp; tmp = mPendingConnections.next())
	{
	}

	// TODO: fix this later so that it searches the whole list
	DirectInfo *tmp = mPendingConnections.first();
	if (!tmp)
	{
		kdDebug(14151) << k_funcinfo << "no pending connection exists! uh oh" << endl;
		return;
	}
	OscarConnection *newsock = createAppropriateType(tmp);
	setupConnection(newsock);
	KExtendedSocket *s = newsock->socket();
	mSocket->accept(s);

	// The line below needs to be here if one person is behind a firewall, because
	// AIM clients will try to reverse the connection if that is the case
	// if ( tmp->type == DirectInfo::Incoming ) && mType == OscarConnection::SendFile )
	newsock->sendFileSendRequest();
}


/** Finds the connection with cookie @cookie and returns a pointer to it.
		If no such connection is found, return NULL */
OscarConnection * OncomingSocket::findConnection(const QByteArray &cookie)
{
	OscarConnection *tmp;
	for (tmp = mConns.first(); tmp; tmp = mConns.next())
	{
		if (cookie == tmp->cookie())
			return tmp;
	}
	return 0L;
}

/** Finds the connection named @name and returns a pointer to it.
			If no such connection is found, return NULL */
OscarConnection * OncomingSocket::findConnection(const QString &name)
{
	OscarConnection *tmp;
	for (tmp = mConns.first(); tmp; tmp = mConns.next())
	{
		if(!tmp->connectionName().compare(tocNormalize(name)))
		{
			kdDebug(14151) << k_funcinfo << "'" << tmp->connectionName() <<
				"' matches dest sn '" << tocNormalize(name) << "'." << endl;
			return tmp;
		}
	}
	return 0L;
}

DirectInfo *OncomingSocket::addPendingConnection(const QString &sn,
	const QByteArray &cookie, const KFileItem *finfo, const QString &host,
	int port, DirectInfo::Type typ)
{
	DirectInfo *ninfo = new DirectInfo;
	ninfo->cookie.duplicate(cookie);
	ninfo->sn = tocNormalize(sn);

	if ( finfo )
		ninfo->finfo = new KFileItem( *finfo );
	else
		ninfo->finfo = 0L;

	ninfo->host = host;
	ninfo->port = port;
	ninfo->type = typ;
	mPendingConnections.append(ninfo);

	return ninfo;
}

void OncomingSocket::slotConnectionReady(QString name)
{
	OscarConnection *dc = 0L;
	for (DirectInfo *tmp=mPendingConnections.first(); tmp; tmp = mPendingConnections.next())
	{
		if ( tmp->sn == tocNormalize(name) )
		{
			dc = findConnection(tmp->cookie);
			mPendingConnections.remove(tmp);
			break;
		}
	}

	if (dc)
	{
		kdDebug(14151) << k_funcinfo << "Connection '" << name <<
			"' not found!!! exiting slotConnectionReady()" << endl;
	}
	else
	{
		kdDebug(14151) << k_funcinfo <<
			"slotConnectionReady(): Setting up direct IM signals!" << endl;

		// Connect protocol error signal
		QObject::connect(
			dc, SIGNAL(protocolError(QString, int)),
			mServer, SLOT(OnDirectIMError(QString, int)));
		// Got IM
		QObject::connect(
			dc, SIGNAL(gotIM(QString, QString, bool)),
			mServer, SLOT(OnDirectIMReceived(QString,QString,bool)));
		// Disconnected
		QObject::connect(
			dc, SIGNAL(connectionClosed(QString)),
			this, SLOT(slotConnectionClosed(QString)));
		QObject::connect(
			dc, SIGNAL(connectionClosed(QString)),
			mServer, SLOT(OnDirectIMConnectionClosed(QString)));
		// Typing notification
		QObject::connect(
			dc, SIGNAL(gotMiniTypeNotification(QString,int)),
			mServer, SLOT(OnDirectMiniTypeNotification(QString, int)));
		// File transfer complete
		QObject::connect(
			dc, SIGNAL(transferComplete(QString)),
			mServer, SLOT(OnFileTransferComplete(QString)));
		// File transfer begun
		QObject::connect(
			dc, SIGNAL(transferBegun(OscarConnection *, const QString &, const unsigned long, const QString &)),
			mServer, SLOT(OnFileTransferBegun(OscarConnection *, const QString &, const unsigned long, const QString &)));
	}
}

/** Set up a connection before adding it to the list of connections */
void OncomingSocket::setupConnection(OscarConnection *newsock)
{
	// Ready signal
	QObject::connect(newsock, SIGNAL(connectionReady(QString)),
		this, SLOT(slotConnectionReady(QString)));

	//only connect this if it is a direct IM connection
	if ( mType == OscarConnection::DirectIM )
	{
		QObject::connect(newsock, SIGNAL(connectionReady(QString)),
		mServer, SLOT(OnDirectIMReady(QString)));
	}

	kdDebug(14151) << k_funcinfo << "setting up connection.. .there are currently " << mConns.count() << endl;
	mConns.append(newsock);
}

/** Adds an outgoing connection to the list and attempts to connect */
OscarConnection *OncomingSocket::establishOutgoingConnection(const QString &sn)
{
	for (DirectInfo *tmp=mPendingConnections.first(); tmp; tmp = mPendingConnections.next())
	{
		if ( tmp->sn == tocNormalize(sn) )
		{
			OscarConnection *s = createAppropriateType(tmp);
			setupConnection(s);
			kdDebug(14151) << k_funcinfo << "Connecting to " << tmp->host << ":" << tmp->port << endl;
			s->socket()->setHost(tmp->host);
			s->socket()->setPort(tmp->port);
			s->socket()->connect();
			return s;
		}
	}

	kdDebug(14151) << k_funcinfo <<
		"WARNING: outgoing connection not found in pending list, returning NULL" << endl;
	return 0L;
}

/** Called when connection named name has been closed */
void OncomingSocket::slotConnectionClosed(QString name)
{
	kdDebug(14151) << k_funcinfo << "Direct connection closed, deleting it: " << name << endl;
	removeConnection(name);
}

/** Removes the named connection from the connection list and disconnects it. */
void OncomingSocket::removeConnection(const QString &name)
{
	kdDebug(14151) << k_funcinfo << "Deleting direct connection " << name << endl;
	OscarConnection *dc = findConnection(name);
	if(dc)
	{
		mConns.remove(dc);
	}
	else
		kdDebug(14151) << k_funcinfo << "No connection to delete" << endl;
}

/** Allocates memory to ptr of the proper type */
OscarConnection * OncomingSocket::createAppropriateType(DirectInfo *tmp)
{
	if ( mType == OscarConnection::DirectIM )
		return new OscarDirectConnection(mServer->getSN(), tmp->sn, tmp->cookie);
	else if ( mType == OscarConnection::SendFile )
	{
		return new OscarFileSendConnection(tmp->finfo, mServer->getSN(), tmp->sn, tmp->cookie);
	}
	else // other type?? this should never happen
	{
		kdDebug(14151) << k_funcinfo << "Creating generic OscarConnection type. INVESTIGATE." << endl;
		return new OscarConnection(mServer->getSN(), tmp->sn, mType, tmp->cookie);
	}
}

/** Adds the passed file info to the appropriate screen name in the list of pending connections */
void OncomingSocket::addFileInfo(const QString &sn, KFileItem *finfo)
{
	for (DirectInfo *tmp=mPendingConnections.first(); tmp; tmp = mPendingConnections.next())
	{
		if ( tmp->sn == tocNormalize(sn) )
			tmp->finfo = finfo;
	}
}

/** Gets the cookie associated with the pending connection for @sn, stores it in cook */
bool OncomingSocket::getPendingCookie(const QString &sn, QByteArray &cook)
{
	for (DirectInfo *tmp=mPendingConnections.first(); tmp; tmp = mPendingConnections.next())
	{
		if ( tmp->sn == sn )
		{
			cook.assign(tmp->cookie);
			return true;
		}
	}
	return false;
}

#include "oncomingsocket.moc"
// vim: set noet ts=4 sts=4 sw=4:
