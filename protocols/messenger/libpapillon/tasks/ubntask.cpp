/*
   ubntask.cpp - Windows Live Messenger UBN processing
	By Use UBN/UUN, User can exchange data with another without setting 
	up a Switchboard session first

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
#include "Papillon/Tasks/UBNTask"

// Qt includes
#include <QtDebug>
#include <QtCore/QLatin1String>

// Papillon includes
#include "Papillon/NetworkMessage"
#include "Papillon/Connection"

namespace Papillon
{
class UBNTask::Private
{
	public:
		Private()
		{
		}
		// Keep track of the expected transaction ID.
		QString currentTransactionId;
}

UBNTask::UBNTask(Task *parent)
	:Task(parent), d(new Private)
{

}

UBNTask::~UBNTask()
{
	delete d;
}

bool UBNTask::take(NetworkMessage *networkMessage)
{
	if( forMe(networkMessage) )
	{
		return true;
	}
}

bool UBNTask::forMe(NetworkMessage *networkMessage) const
{
	if( networkMessage->type() == NetworkMessage::TransactionMessage )
	{
		if( networkMessage->command() == QLatin1String("UBN") )
		{
			if( networkMessage->transactionId() == d->currentTransactionId )
			{
				setSuccess();
				return true;
			}
		}
	}

	return false;
}

}
#include "ubntask.moc"
