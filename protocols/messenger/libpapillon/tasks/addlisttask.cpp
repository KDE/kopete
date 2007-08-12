/*
   addlisttask.cpp - Windows Live Messenger ADL processing

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
#include "Papillon/Tasks/AddListTask"

// Qt includes
#include <QtDebug>
#include <QtCore/QLatin1String>

// Papillon includes
#include "Papillon/NetworkMessage"
#include "Papillon/Connection"

namespace Papillon
{
class AddListTask::Private
{
	public:
		Private()
		{
		}
		// Keep track of the expected transaction ID.
		QString currentTransactionId;
}

AddListTask::AddListTask(Task *parent)
	:Task(parent), d(new Private)
{

}

AddListTask::~AddListTask()
{
	delete d;
}

void AddListTask::onGo()
{
	qDebug() << Q_FUNC_INFO << "post ADL XML to server...";
	postADLCommand();
}

void AddListTask::setADLPayload(QString payload)
{
	d->adlPayload = payload;
}

void AddListTask::postADLCommand()
{
	qDebug() << Q_FUNC_INFO << "Sending ADL command.";
	NetworkMessage *adlMessage = new NetworkMessage(NetworkMessage::TransactionTransfer);
	adlMessage->setCommand( QLatin1String("ADL") );
	d->currentTransactionId = QString::number( connection()->transactionId() );
	adlMessage->setTransactionId( d->currentTransactionId );

	adlMessage->setArguments(d->adlPayload);

	send(adlMessage);
}

bool AddListTask::take(NetworkMessage *networkMessage)
{
	if( forMe(networkMessage) )
	{
		return true;
	}
}

bool AddListTask::forMe(NetworkMessage *networkMessage) const
{
	if( networkMessage->type() == NetworkMessage::TransactionMessage )
	{
		if( networkMessage->command() == QLatin1String("ADL") )
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
#include "addlisttask.moc"
