/*
   client.cpp - Papillon Client to Windows Live Messenger.

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
#include "Papillon/Client"

// Qt includes
#include <QtDebug>

// QCA include
#include <QtCrypto>

// Papillon includes
#include "Papillon/Connection"
#include "Papillon/Base/Connector"
#include "Papillon/Http/SecureStream"
#include "Papillon/ClientStream"
#include "Papillon/Transfer"
#include "Papillon/MimeHeader"
#include "Papillon/ContactList"
#include "Papillon/UserContact"

// Papillon tasks
#include "Papillon/Tasks/LoginTask"
#include "Papillon/Tasks/NotifyMessageTask"
#include "Papillon/Tasks/NotifyPresenceTask"
#include "Papillon/Tasks/NotifyStatusMessageTask"

namespace Papillon
{

class Client::Private
{
public:
	Private()
	 : connector(0), notificationConnection(0),
	   server( QLatin1String("messenger.hotmail.com") ), port(1863), contactList(0), userContact(0),
	   loginTask(0), notifyMessageTask(0), notifyPresenceTask(0), notifyStatusMessageTask(0)
	{}

	Connector *connector;
	Connection *notificationConnection;

	QString server;
	quint16 port;

	QString passportId;
	QString password;
	QString passportAuthTicket;
	// Convience object that init QCA.
	QCA::Initializer qcaInit;

	ContactList *contactList;
	UserContact *userContact;

	// All the tasks
	LoginTask *loginTask;
	NotifyMessageTask *notifyMessageTask;
	NotifyPresenceTask *notifyPresenceTask;
	NotifyStatusMessageTask *notifyStatusMessageTask;
};

Client::Client(Connector *connector, QObject *parent)
 : QObject(parent), d(new Private)
{
	d->connector = connector;
	d->contactList = new ContactList(this);
	d->userContact = new UserContact(this);
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

ContactList *Client::contactList()
{
	return d->contactList;
}

UserContact *Client::userContact()
{
	return d->userContact;
}

Connection *Client::notificationConnection()
{
	return d->notificationConnection;
}

void Client::connectToServer(Papillon::OnlineStatus::Status initialStatus)
{
	// TODO: Make use of initial status.
	Q_UNUSED(initialStatus);

	if( !d->notificationConnection )
	{
		d->notificationConnection = createConnection();
		connect(d->notificationConnection, SIGNAL(connected()), this, SIGNAL(connected()));
		connect(d->notificationConnection, SIGNAL(connected()), this, SLOT(initNotificationTasks()));
	}

	d->notificationConnection->connectToServer(d->server, d->port);
}

void Client::setServer(const QString &server, quint16 port)
{
	// Don't override the default or another alternative server with an empty server.
	if( !server.isEmpty() )
		d->server = server;
	if( port != 0 )
		d->port = port;
}

void Client::initNotificationTasks()
{
	if( !d->notifyMessageTask )
	{
		d->notifyMessageTask = new NotifyMessageTask( d->notificationConnection->rootTask() );
		connect(d->notifyMessageTask, SIGNAL(profileMessage(const Papillon::MimeHeader &)), this, SLOT(gotInitalProfile(const Papillon::MimeHeader& )));
	}

	if( !d->notifyPresenceTask )
	{
		d->notifyPresenceTask = new NotifyPresenceTask( d->notificationConnection->rootTask() );
		connect(d->notifyPresenceTask, SIGNAL(contactStatusChanged( const QString&, Papillon::OnlineStatus::Status )), this, SLOT(slotContactStatusChanged( const QString&, Papillon::OnlineStatus::Status )));
	}

	if( !d->notifyStatusMessageTask )
	{
		d->notifyStatusMessageTask = new NotifyStatusMessageTask( d->notificationConnection->rootTask() );
		connect(d->notifyStatusMessageTask, SIGNAL(contactStatusMessageChanged(const QString &, const Papillon::StatusMessage &)), this, SLOT(slotContactStatusMessageChanged(const QString &, const Papillon::StatusMessage &)));
	}
}

void Client::login()
{
	if(d->loginTask)
	{
		delete d->loginTask;
		d->loginTask = 0;
	}

	d->loginTask = new LoginTask(d->notificationConnection->rootTask());
	connect(d->loginTask, SIGNAL(redirection(const QString &, quint16)), this, SLOT(loginRedirect( const QString&, quint16 )));
	connect(d->loginTask, SIGNAL(finished(Papillon::Task*)), this, SLOT(loginResult(Papillon::Task*)));
	d->loginTask->go(Task::AutoDelete);
}

void Client::loginRedirect(const QString &server, quint16 port)
{
	qDebug() << PAPILLON_FUNCINFO << "Redirect to" << QString("%1:%2").arg(server).arg(port);

	d->notificationConnection->disconnectFromServer();
	d->notificationConnection->connectToServer(server, port);
	
	// Redo login process.
	login();
}

void Client::loginResult(Papillon::Task *task)
{
	Q_UNUSED(task);
}

void Client::gotInitalProfile(const Papillon::MimeHeader &profileMessage)
{
	QString passportAuthTicket = profileMessage.value( QLatin1String("MSPAuth") ).toString();

	userContact()->setLoginCookie( passportAuthTicket );

	qDebug() << PAPILLON_FUNCINFO << "Received auth ticket:" << passportAuthTicket;
}

void Client::slotContactStatusChanged(const QString &contactId, Papillon::OnlineStatus::Status status)
{
	emit contactStatusChanged(contactId, status);
}

void Client::slotContactStatusMessageChanged(const QString &contactId, const Papillon::StatusMessage &newStatusMessage)
{
	emit contactStatusMessageChanged(contactId, newStatusMessage);
}

void Client::writeCommand(Transfer *command)
{
	d->notificationConnection->send(command);
}

}

#include "client.moc"
