/*
   client.cpp - Papillon Client to Windows Live Messenger.

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
#include "client.h"

// Qt includes

// QCA include
#include <QtCrypto>

// Papillon includes
#include "connection.h"
#include "connector.h"
#include "logintask.h"
#include "securestream.h"
#include "papillonclientstream.h"

namespace Papillon
{

class Client::Private
{
public:
	Private()
	 : connector(0), notificationConnection(0),
	   server( QLatin1String("messenger.hotmail.com") ), port(1863)
	{}

	Connector *connector;
	Connection *notificationConnection;

	QString server;
	quint16 port;

	QString passportId;
	QString password;
	// Convience object that init QCA.
	QCA::Initializer qcaInit;
};

Client::Client(Connector *connector, QObject *parent)
 : QObject(parent), d(new Private)
{
	d->connector = connector;
}

Client::~Client()
{
	delete d;
}

SecureStream *Client::createSecureStream()
{
	Connector *newConnector = d->connector->createNewConnector(this);
	SecureStream *secureStream = new SecureStream(newConnector);
	
	return secureStream;
}

Connection *Client::createConnection()
{
	Connector *connector = d->connector->createNewConnector(this);
	ClientStream *clientStream = new ClientStream(connector, this);
	Connection *newConnection = new Connection(clientStream, this);
	newConnection->setClient(this);

	return newConnection;
}

void Client::connectToServer(const QString &server, quint16 port)
{
	if( !server.isEmpty() )
		d->server = server;
	if( port != 0 )
		d->port = port;

	if( !d->notificationConnection )
	{
		d->notificationConnection = createConnection();
		connect(d->notificationConnection, SIGNAL(connected()), this, SLOT(notifyConnected()));
	}

	d->notificationConnection->connectToServer(d->server, d->port);
}

void Client::setClientInfo(const QString &passportId, const QString &password)
{
	d->passportId = passportId;
	d->password = password;
}

void Client::login()
{
	LoginTask *login = new LoginTask(d->notificationConnection->rootTask());
	login->setUserInfo(d->passportId, d->password);
	connect(login, SIGNAL(finished(Papillon::Task*)), this, SLOT(loginResult(Papillon::Task*)));
	login->go(true);
}

void Client::notifyConnected()
{
	
}

void Client::loginResult(Papillon::Task *task)
{
	
}

}

#include "client.moc"
