/***************************************************************************
                          dcchandler.h  -  description
                             -------------------
    begin                : Fri Apr 19 2002
    copyright            : (C) 2002 by nbetcher
    email                : nbetcher@usinternet.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef DCCHANDLER_H
#define DCCHANDLER_H

#include <qhostaddress.h>
#include <qstring.h>
#include <qsocket.h>
#include <qserversocket.h>
#include <qfile.h>
#include <qregexp.h>

class DCCChat : public QSocket
{
Q_OBJECT
public:
	DCCChat(QHostAddress host, unsigned int port);
	DCCChat();
	void dccAccept();
	void dccCancel();
	bool sendMessage(const QString &message);
	const QHostAddress ipaddress() { return mHost; };
	const unsigned int hostPort() { return mPort; };
signals:
	void incomingDccMessage(const QString &, bool fromMe);
	void terminating();
private:
	QHostAddress mHost;
	unsigned int mPort;
private slots:
	void slotReadyRead();
	void slotConnectionClosed();
	void slotError(int);
};

class DCCSend : public QServerSocket
{
Q_OBJECT
public:
	DCCSend(const QString &filename);
	virtual void newConnection(int socket);
};

#endif
