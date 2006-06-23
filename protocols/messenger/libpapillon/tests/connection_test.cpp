/*
   connection_test.cpp - Connection test

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
#include "connection_test.h"

// Qt includes
#include <QtDebug>
#include <QtCore/QCoreApplication>

// Papillon includes
#include "connection.h"
#include "papillonclientstream.h"
#include "logintask.h"
#include "qtconnector.h"
#include "client.h"

using namespace Papillon;

FakeConnection::FakeConnection(Papillon::ClientStream *stream)
	: Papillon::Connection(stream), m_loginTask(0)
{
}

void FakeConnection::start()
{
	m_loginTask = new LoginTask( rootTask() );
	m_loginTask->setUserInfo("ljlkjwerklwjerwlek@mwerewerty.org", "h4x0rl33t");
	connect(m_loginTask, SIGNAL(redirection(const QString &, quint16)), this, SLOT(redirect(const QString &, quint16)));
	connect(m_loginTask, SIGNAL(finished(Papillon::Task*)), this, SLOT(loginTaskFinished(Papillon::Task *)));
	
	m_loginTask->go();
}

void FakeConnection::loginTaskFinished(Papillon::Task *task)
{
	LoginTask *login = static_cast<LoginTask*>(task);

	// LoginTask will always fail.
	if( !login->success() )
		emit loginFinished(login);
}

void FakeConnection::redirect(const QString &newServer, quint16 newPort)
{
	qDebug() << "Redirect to" << newServer;

	// Reset Login Task.
	delete m_loginTask;
	m_loginTask = 0;

	disconnectFromServer();
	connectToServer(newServer, newPort);
}

void Connection_Test::testConnection()
{
	Client *client = new Client(new QtConnector(this), this);
	ClientStream *stream = new ClientStream(new QtConnector(this), this);
	m_connection = new FakeConnection(stream);
	m_connection->connectToServer("messenger.hotmail.com", 1863);
	m_connection->setClient(client);

	connect(m_connection, SIGNAL(connected()), m_connection, SLOT(start()));
	connect(m_connection, SIGNAL(loginFinished(Papillon::LoginTask*)), this, SLOT(slotLoginFinished()));;
}

void Connection_Test::slotLoginFinished()
{
	qDebug() << "Login Task test complete. Exiting.";
	m_connection->disconnectFromServer();
	QCoreApplication::exit(0);
}

int main(int argc, char **argv)
{
	QCoreApplication app(argc, argv);

	Connection_Test *test = new Connection_Test();
	test->testConnection();

	return app.exec();
}

#include "connection_test.moc"
