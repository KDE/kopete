/*
   usercontact.h - Windows Live Messenger user contact

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
#include "Papillon/UserContact"

// Qt includes
#include <QtCore/QPointer>

// Papillon includes
#include "Papillon/Client"
#include "Papillon/Connection"
#include "Papillon/StatusMessage"
#include "Papillon/Tasks/SetPresenceTask"
#include "Papillon/Tasks/SetStatusMessageTask"
#include "Papillon/Tasks/SetPersonalInformationTask"

namespace Papillon
{

class UserContact::Private
{
public:
	Private()
	{}

	QPointer<Client> client;

	QString password;
	QString loginCookie;
};

UserContact::UserContact(Client *client)
 : Contact(client), d(new Private)
{
	d->client = client;
}

UserContact::~UserContact()
{
	delete d;
}

Client *UserContact::client()
{
	return d->client;
}

void UserContact::setLoginInformation(const QString &passportId, const QString &password)
{
	setPassportId(passportId);
	d->password = password;
}

QString UserContact::password() const
{
	return d->password;
}

void UserContact::setLoginCookie(const QString &cookie)
{
	d->loginCookie = cookie;
}

QString UserContact::loginCookie() const
{
	return d->loginCookie;
}

void UserContact::setPresence(Papillon::Presence::Status newPresence)
{
	SetPresenceTask *presenceTask = new SetPresenceTask( client()->notificationConnection()->rootTask() );
	presenceTask->setPresence( newPresence );
	// TODO: Set client features
	// TODO: Do something about MsnObject
	
	presenceTask->go(Task::AutoDelete);
}

void UserContact::setPersonalStatusMessage(const Papillon::StatusMessage &statusMessage)
{
	SetStatusMessageTask *messageTask = new SetStatusMessageTask( client()->notificationConnection()->rootTask() );
	messageTask->setStatusMessage( statusMessage );

	messageTask->go(Task::AutoDelete);
}

void UserContact::setPersonalInformation(Papillon::ClientInfo::PersonalInformation type, const QString &value)
{
	SetPersonalInformationTask *setInfo = new SetPersonalInformationTask( client()->notificationConnection()->rootTask() );
	setInfo->setPersonalInformation(type, value);

	setInfo->go(Task::AutoDelete);
}

}

#include "usercontact.moc"
