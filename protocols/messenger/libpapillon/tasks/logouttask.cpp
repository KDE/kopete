/*
    logouttask.cpp - Windows Live Messenger logout task

    Copyright (c) 2007		by Zhang Panyong        <pyzhang@gmail.com>
    Kopete    (c) 2002-2005 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

// Qt includes
#include <QtDebug>

// Papillon includes
#include "Papillon/NetworkMessage"

namespace Papillon 
{

class LogoutTask::Private
{
public:
	Private()
}

LogoutTask::LogoutTask(Task *parent)
 : Task(parent), d(new Private)
{
}

LogoutTask::~LogoutTask()
{
	delete d;
}

void LogoutTask::onGo()
{
	qDebug() << Q_FUNC_INFO << "send out OUT command...";
	sendLogoutCommand();
}

void LogoutTask::sendLogoutCommand()
{
	qDebug() << Q_FUNC_INFO << "Sending OUT command.";
	NetworkMessage *logoutMessage = new NetworkMessage(NetworkMessage::TransactionTransfer);
	logoutMessage->setCommand( QLatin1String("OUT") );
	logoutMessage->setTransactionId( QString() );

	send(logoutMessage);
}

bool LogoutTask::take(NetworkMessage *networkMessage)
{
	if( forMe(networkMessage) )
	{
		//Out Error
		if(networkMessage->arguments()[0] == QLatin1String("OUH"))
		{

		}
		//Server Down
		if(networkMessage->arguments()[0] == QLatin1String("SSD"))
		{

		}
		return true;
	}
	return false
}

bool logoutTask::forMe(NetworkMessage *networkMessage) const
{
	if( networkMessage->command() == QLatin1String("OUT") )
	{
		return true;
	}
	return false;	
}

}

#include "logouttask.moc"
