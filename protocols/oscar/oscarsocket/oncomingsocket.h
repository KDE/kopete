/***************************************************************************
                          oncomingsocket.h  -  description
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

#ifndef ONCOMINGSOCKET_H
#define ONCOMINGSOCKET_H

#include <qwidget.h>
#include <qserversocket.h>
#include <qlist.h>

struct DirectInfo { //info used for keeping track of direct connections
	QByteArray cookie;
	QString sn;
	QString host;
	int port;
	const KFileItem *finfo;
	enum Type { Outgoing, Incoming };
	Type type;
};

#define DIRECTIM_PORT		4443
#define SENDFILE_PORT		5190

/**Handles oncoming connections
  *@author twl6
  */

class OscarConnection;
class OscarSocket;

class OncomingSocket : public QServerSocket  {
   Q_OBJECT
public: 
	OncomingSocket(QObject *parent=0, const char *name=0);
	OncomingSocket(OscarSocket *server, const QHostAddress &address, OscarConnection::ConnectionType type, Q_UINT16 port=DIRECTIM_PORT,
		int backlog=5, QObject *parent=0, const char *name=0);
	~OncomingSocket();
  /** Called when someone connects to the serversocket */
  virtual void newConnection( int socket );
  /** Finds the connection with cookie @cookie and returns a pointer to it.
			If no such connection is found, return NULL */
  OscarConnection * findConnection(const QByteArray &cookie);
  /** Finds the connection named @name and returns a pointer to it.
			If no such connection is found, return NULL */
  OscarConnection * findConnection(const QString &name);
  /** Adds the connection to the list of pending connections, returns it */
  DirectInfo *addPendingConnection(const QString &sn, const QByteArray &cookie, const KFileItem *finfo, const QString &host, int port, DirectInfo::Type typ);
  /** Adds an outgoing connection to the list and attempts to connect */
  OscarConnection * establishOutgoingConnection(const QString &sn);
  /** Removes the named connection from the connection list and disconnects it. */
  void removeConnection(const QString &name);
  /** Adds the passed file info to the appropriate screen name in the list of pending connections */
  void addFileInfo(const QString &sn, KFileItem *finfo);
  /** Gets the cookie associated with the pending connection for @sn, stores it in cook */
  bool getPendingCookie(const QString &sn, QByteArray &cook);
private:
  /** A list of all connections */
  QPtrList<OscarConnection> mConns;
  /** A list with pending connection info */
  QPtrList<DirectInfo> mPendingConnections;
  /** The type of objects to be created */
  OscarConnection::ConnectionType mType;
	OscarSocket *mServer;
public slots: // Public slots
  /** Called when a connection is ready */
  void slotConnectionReady(QString name);
  /** Called when connection named name has been closed */
  void slotConnectionClosed(QString name);
private: // Private methods
  /** Set up a connection before adding it to the list of connections */
  void setupConnection(OscarConnection *newsock);
  /** Allocates memory to ptr of the proper type */
  OscarConnection * createAppropriateType(DirectInfo *info);
};

#endif
