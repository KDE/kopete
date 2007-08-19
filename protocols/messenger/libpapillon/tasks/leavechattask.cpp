/*
    leavechattask.cpp - Windows Live Messenger leave chat Task

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
#include "Papillon/Tasks/LeaveChatTask"

// Qt includes
#include <QtDebug>

// Papillon includes
#include "Papillon/NetworkMessage"
#include "Papillon/Connection"

namespace Papillon
{

class LeaveChatTask::Private
{
	public:
		Private()
		{}
}

LeaveChatTask::LeaveChatTask(Task *parent)
	:Task(parent), d(new Private)
{

}

LeaveChatTask::~LeaveChatTask()
{
	delete d;
}

bool LeaveChatTask::take(NetworkMessage *networkMessage)
{
	if( networkMessage->command() == QLatin1String("BYE") )
	{
		QString handle = networkMessage->arguments()[0];
		QString reason = (networkMessage->arguments()[1] == "1") ? i18n("timeout") : QString();
		qDebug() << Q_FUNC_INFO << "User Leave Switchboard, handle"<< handle<< "reason"<<reason;

		emit userLeft(handle, reason);
		return true;
	}
	return false;
}

}

#include "leavechattask.moc"
