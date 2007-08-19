/*
    joinchattask.cpp - Windows Live Messenger Join chat Task

   Copyright (c) 2007      by Zhang Panyong  <pyzhang@gmail.com>
   Kopete    (c) 2002-2005 by the Kopete developers  <kopete-devel@kde.org>

   *************************************************************************
   *                                                                       *
   * This library is free software; you can redistribute it and/or         *
   * modify it under the terms of the GNU Lesser General Public            *
   * License as published by the Free Software Foundation; either          *
   * version 2 of the License, or (at your option) any later version.      *
   *                                                                       *
   *************************************************************************
*/
#include "Papillon/Tasks/JoinChatTask"

// Qt includes
#include <QtDebug>

// Papillon includes
#include "Papillon/NetworkMessage"
#include "Papillon/Connection"

namespace Papillon
{
class JoinChatTask::Private
{
	public:
		Private()
		{
		}
}

JoinChatTask::JoinChatTask(Task *parent)
	:Task(parent), d(new Private)
{

}

JoinChatTask::~JoinChatTask()
{
	delete d;
}

void JoinChatTask::onGo()
{

}

void JoinChatTask::sendCAL(QString &handle)
{
	NetworkMessage *calMessage = new NetworkMessage(NetworkMessage::NormalMessage);

	qDebug() << Q_FUNC_INFO << "Send CAL";
	calMessage->setCommand( QLatin1String("CAL") );
	calMessage->setArguments( handle);

	send(calMessage);
}

bool JoinChatTask::take(NetworkMessage *networkMessage)
{
	if( networkMessage->command() == QLatin1String("JOI") )
	{
		qDebug() << Q_FUNC_INFO << "new User Join";
		QString handle 		= networkMessage->arguments()[0];
		QString screenName	= networkMessage->arguments()[1];

		emit userJoined(handle, screenName, false);
		return true;
	}
	else if ( networkMessage->command() == QLatin1String("IRO") )
	{
		qDebug() << Q_FUNC_INFO << "Join a multi Chat";
		QString handle 		= networkMessage->arguments()[2];
		QString screenName	= networkMessage->arguments()[3];

		emit userJoined(handle, screenName, true);
		return true;
	}
	return false;
}

}

#include "joinchattask.moc"
