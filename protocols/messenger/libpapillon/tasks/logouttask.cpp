/*
    logouttask.cpp - Windows Live Messenger logout task

    Copyright (c) 2007		by Zhang Panyong        <pyzhang8@gmail.com>
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
	qDebug() << PAPILLON_FUNCINFO << "send out OUT command...";
	sendLogoutCommand();
}

void LogoutTask::sendLogoutCommand()
{
	qDebug() << PAPILLON_FUNCINFO << "Sending OUT command.";
	Transfer *logoutTransfer = new Transfer(Transfer::TransactionTransfer);
	logoutTransfer->setCommand( QLatin1String("OUT") );
	logoutTransfer->setTransactionId( QString() );
	send(logoutTransfer);
}

bool LogoutTask::take(Transfer *transfer)
{
	if( forMe(transfer) )
	{
		//Out Error
		if(transfer->arguments()[0] == QLatin1String("OUH"))
		{

		}
		//Server Down
		if(transfer->arguments()[0] == QLatin1String("SSD"))
		{

		}
		return true;
	}
	return false
}

bool logoutTask::forMe(Transfer *transfer) const
{
	if( transfer->command() == QLatin1String("OUT") )
	{
		return true;
	}
	return false;	
}

}

#include "logouttask.moc"
