/*
   sendyahoomesssagetask.cpp - Windows Live Messenger Send yahoo Message Task
   								UUM Processing

    Copyright (c) 2007		by Zhang Panyong  <pyzhang@gmail.com>

   *************************************************************************
   *                                                                       *
   * This library is free software; you can redistribute it and/or         *
   * modify it under the terms of the GNU Lesser General Public            *
   * License as published by the Free Software Foundation; either          *
   * version 2 of the License, or (at your option) any later version.      *
   *                                                                       *
   *************************************************************************
*/
#include "Papillon/tasks/SendYahooMessageTask"
// Qt includes
#include <QtDebug>

// Papillon includes
#include "Papillon/NetworkMessage"

namespace Papillon
{

class SendYahooMessageTask::Private
{
public:
	Private()
	{}
};

SendYahooMessageTask::SendYahooMessageTask(Papillon::Task *parent)
 : Papillon::Task(parent), d(new Private)
{

}

SendYahooMessageTask::~SendYahooMessageTask()
{
	delete d;
}

bool SendYahooMessageTask::take(NetworkMessage *networkMessage)
{
	if( forMe(networkMessage) )
	{
		return true;
	}
}

bool SendYahooMessageTask::forMe(NetworkMessage *networkMessage) const
{
	if( networkMessage->type() == NetworkMessage::TransactionMessage )
	{
		if( networkMessage->command() == QLatin1String("UUM") )
		{
				setSuccess();
				return true;
		}
	}

	return false;
}

}

#include "sendyahoomessagetask.moc"
