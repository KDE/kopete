/*
   connection_test.h - Connection test

   Copyright (c) 2006 by Michaël Larouche <michael.larouche@kdemail.net>

   *************************************************************************
   *                                                                       *
   * This library is free software; you can redistribute it and/or         *
   * modify it under the terms of the GNU Lesser General Public            *
   * License as published by the Free Software Foundation; either          *
   * version 2 of the License, or (at your option) any later version.      *
   *                                                                       *
   *************************************************************************
*/
#ifndef CONNECTIONTEST_H
#define CONNECTIONTEST_H

#include <QObject>
#include "connection.h"

namespace Papillon
{
	class ClientStream;
	class Task;
	class LoginTask;;
}

using namespace Papillon;

class FakeConnection : public Connection
{
	Q_OBJECT
public:
	FakeConnection(Papillon::ClientStream *stream);

public slots:
	void start();

private slots:
	void redirect(const QString &newServer, quint16 newPort);
	void loginTaskFinished(Papillon::Task *task);

signals:
	void loginFinished(Papillon::LoginTask *task);
private:
	LoginTask *m_loginTask;
};

class Connection_Test : public QObject
{
	Q_OBJECT
public slots:
	void testConnection();

private slots:
	void slotLoginFinished();
private:
	FakeConnection *m_connection;
};


#endif
