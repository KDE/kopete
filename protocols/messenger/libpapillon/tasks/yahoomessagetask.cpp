/*
   yahoomesssagetask.cpp - Windows Live Messenger UBM processing
   							Receive the Yahoo Message

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
#include "Papillon/YahooMessageTask"

// Qt includes
#include <QtDebug>

// Papillon includes
#include "Papillon/NetworkMessage"

namespace Papillon
{

class YahooMessageTask::Private
{
public:
	Private()
	{}
};

YahooMessageTask::YahooMessageTask(Papillon::Task *parent)
 : Papillon::Task(parent), d(new Private)
{

}

YahooMessageTask::~YahooMessageTask()
{
	delete d;
}

bool YahooMessageTask::take(NetworkMessage *networkMessage)
{
	if( forMe(networkMessage) )
	{
		return true;
	}
}

bool YahooMessageTask::forMe(NetworkMessage *networkMessage) const
{
	if( networkMessage->type() == NetworkMessage::TransactionMessage )
	{
		if( networkMessage->command() == QLatin1String("UBM") )
		{
				setSuccess();
				return true;
		}
	}

	return false;
}

}

#include "yahoomessagetask.moc"
