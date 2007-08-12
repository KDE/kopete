/*
   removelisttask.cpp - Windows Live Messenger RML processing

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
#include "Papillon/Tasks/RemoveListTask"

// Qt includes
#include <QtDebug>
#include <QtCore/QLatin1String>

// Papillon includes
#include "Papillon/NetworkMessage"
#include "Papillon/Connection"

namespace Papillon
{
class RemoveListTask::Private
{
	public:
		Private()
		{
		}
		// Keep track of the expected transaction ID.
		QString currentTransactionId;
		QString rmlPayload;
}

RemoveListTask::RemoveListTask(Task *parent)
	:Task(parent), d(new Private)
{
}

RemoveListTask::~RemoveListTask()
{
	delete d;
}

void RemoveListTask::onGo()
{
	qDebug() << Q_FUNC_INFO << "post RML XML to server...";
	postRMLCommand();
}

void RemoveListTask::setRMLPayload(QString payload)
{
	d->rmlPayload = payload;
}

void RemoveListTask::postRMLCommand()
{
	qDebug() << Q_FUNC_INFO << "Sending RML command.";
	NetworkMessage *rmlMessage = new NetworkMessage(NetworkMessage::TransactionTransfer);
	rmlMessage->setCommand( QLatin1String("RML") );
	d->currentTransactionId = QString::number( connection()->transactionId() );
	rmlMessage->setTransactionId( d->currentTransactionId );

	rmlMessage->setArguments(d->rmlPayload);

	send( rmlMessage );
}

bool RemoveListTask::take(NetworkMessage *networkMessage)
{
	if( forMe(networkMessage) )
	{
		return true;
	}
}

bool RemoveListTask::forMe(NetworkMessage *networkMessage) const
{
	if( networkMessage->type() == NetworkMessage::TransactionMessage )
	{
		if( networkMessage->command() == QLatin1String("RML") )
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
#include "removelisttask.moc"
