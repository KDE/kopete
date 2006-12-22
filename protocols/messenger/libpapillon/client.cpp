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
	   server( QLatin1String("messenger.hotmail.com") ), port(1863), connectionStatus(Disconnected), contactList(0), userContact(0),
	   loginTask(0), notifyMessageTask(0), notifyPresenceTask(0), notifyStatusMessageTask(0)
	{}

	Connector *connector;
	Connection *notificationConnection;

	QString server;
	quint16 port;
	// Convience object that init QCA.
	QCA::Initializer qcaInit;
	Papillon::Client::ConnectionStatus connectionStatus;

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

Papillon::Client::ConnectionStatus Client::connectionStatus() const
{
	return d->connectionStatus;
}

void Client::connectToServer(Papillon::Presence::Status initialPresence)
{
	// TODO: Make use of initial presence.
	Q_UNUSED(initialPresence);

	if( !d->notificationConnection )
	{
		d->notificationConnection = createConnection();
		connect(d->notificationConnection, SIGNAL(connected()), this, SLOT(notificationConnected()));
		connect(d->notificationConnection, SIGNAL(connected()), this, SLOT(initNotificationTasks()));
	}

	setConnectionStatus( Client::Connecting );
	d->notificationConnection->connectToServer(d->server, d->port);
}

void Client::disconnectFromServer()
{
	d->notificationConnection->disconnectFromServer();

	setConnectionStatus( Client::Disconnected );
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
		connect(d->notifyMessageTask, SIGNAL(profileMessage(Papillon::MimeHeader)), this, SLOT(gotInitalProfile(Papillon::MimeHeader)));
	}

	if( !d->notifyPresenceTask )
	{
		d->notifyPresenceTask = new NotifyPresenceTask( d->notificationConnection->rootTask() );
		connect(d->notifyPresenceTask, SIGNAL(contactPresenceChanged(QString, Papillon::Presence::Status )), this, SLOT(slotContactPresenceChanged(QString, Papillon::Presence::Status )));
	}

	if( !d->notifyStatusMessageTask )
	{
		d->notifyStatusMessageTask = new NotifyStatusMessageTask( d->notificationConnection->rootTask() );
		connect(d->notifyStatusMessageTask, SIGNAL(contactStatusMessageChanged(QString, Papillon::StatusMessage)), this, SLOT(slotContactStatusMessageChanged(QString, Papillon::StatusMessage)));
	}
}

void Client::notificationConnected()
{
	setConnectionStatus( Client::Connected );
	// Init login process.
	login();
}

void Client::login()
{
	// Do not login twice
	if( d->connectionStatus != Client::LoggedIn )
	{
		// LoginTask got deleted by Task::onDisconnect() because of the redirection.
		d->loginTask = new LoginTask(d->notificationConnection->rootTask());
		connect(d->loginTask, SIGNAL(redirection(QString, quint16)), this, SLOT(loginRedirect(QString, quint16 )));
		connect(d->loginTask, SIGNAL(finished(Papillon::Task*)), this, SLOT(loginResult(Papillon::Task*)));
		d->loginTask->go(Task::AutoDelete);
	}
}

void Client::loginRedirect(const QString &server, quint16 port)
{
	qDebug() << PAPILLON_FUNCINFO << "Redirect to" << QString("%1:%2").arg(server).arg(port);

	d->notificationConnection->disconnectFromServer();
	d->notificationConnection->connectToServer(server, port);
}

void Client::loginResult(Papillon::Task *task)
{
	LoginTask *loginTask = static_cast<LoginTask*>(task);
	if( loginTask )
	{
		if( loginTask->success() )
			setConnectionStatus( Client::LoggedIn );
		else if( loginTask->loginState() == LoginTask::StateBadPassword )
			setConnectionStatus( Client::LoginBadPassword );
		else if( loginTask->loginState() != LoginTask::StateRedirection )
			setConnectionStatus( Client::Disconnected );
	}
}

void Client::gotInitalProfile(const Papillon::MimeHeader &profileMessage)
{
	QString passportAuthTicket = profileMessage.value( QLatin1String("MSPAuth") ).toString();

	userContact()->setLoginCookie( passportAuthTicket );

	qDebug() << PAPILLON_FUNCINFO << "Received auth ticket:" << passportAuthTicket;
}

void Client::slotContactPresenceChanged(const QString &contactId, Papillon::Presence::Status presence)
{
	emit contactPresenceChanged(contactId, presence);
}

void Client::slotContactStatusMessageChanged(const QString &contactId, const Papillon::StatusMessage &newStatusMessage)
{
	emit contactStatusMessageChanged(contactId, newStatusMessage);
}

void Client::writeCommand(Transfer *command)
{
	d->notificationConnection->send(command);
}

void Client::setConnectionStatus(Papillon::Client::ConnectionStatus newStatus)
{
	qDebug() << PAPILLON_FUNCINFO << "New connection status: " << newStatus;

	d->connectionStatus = newStatus;

	emit connectionStatusChanged(newStatus);
}

}

#include "client.moc"
