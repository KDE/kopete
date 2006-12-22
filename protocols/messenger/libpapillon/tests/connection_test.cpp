/*
   connection_test.cpp - Connection test

   Copyright (c) 2006 by Michaël Larouche <larouche@kde.org>

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
#include "Papillon/QtConnector"
#include "Papillon/UserContact"

using namespace Papillon;

void Connection_Test::testConnection()
{
	m_client = new Client(new QtConnector(this), this);
	m_client->userContact()->setLoginInformation("ljlkjwerklwjerwlek@mwerewerty.org", "h4x0rl33t");
	m_client->connectToServer();

	connect(m_client, SIGNAL(connectionStatusChanged(Papillon::Client::ConnectionStatus)), this, SLOT(clientConnectionStatusChanged(Papillon::Client::ConnectionStatus)));
}

void Connection_Test::clientConnectionStatusChanged(Papillon::Client::ConnectionStatus status)
{
	switch(status)
	{
		case Client::LoginBadPassword:
			qDebug() << "Got a login bad password (expected behavior). Exiting...";
			m_client->disconnectFromServer();
			QCoreApplication::exit(0);
			break;
	}
}

int main(int argc, char **argv)
{
	QCoreApplication app(argc, argv);

	Connection_Test *test = new Connection_Test();
	test->testConnection();

	return app.exec();
}

#include "connection_test.moc"
