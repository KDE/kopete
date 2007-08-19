/*
   ackmessagetask.cpp - Windows Live Messenger ACK/NAK processing

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
#include "Papillon/Tasks/ACKMessageTask"

// Qt includes
#include <QtDebug>

// Papillon includes
#include "Papillon/NetworkMessage"
#include "Papillon/Connection"

namespace Papillon
{

class ACKMessageTask;:Private
{
	public:
		Private(){};
}

ACKMessageTask::ACKMessageTask(Task *parent)
	:Task(parent), d(new Private)
{
}

ACKMessageTask::~ACKMessageTask()
{
	delete d;
}

bool ACKMessageTask::take(NetworkMessage *networkMessage)
{
	if ( networkMessage->command() == QLatin1String("ACK") )
	{
		emit messageAcknowledged(networkMessage->arguments()[0], true);
		setSuccess();
		return true;
	}
	if (networkMessage->command() == QLatin1String("NAK") )
	{
		emit messageAcknowledged(networkMessage->arguments()[0], false);
		setSuccess();
		return true;
	}

	return false;
}

}
#include "ackmessagetask.moc"
