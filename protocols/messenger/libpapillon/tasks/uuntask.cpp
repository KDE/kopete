/*
   uuntask.cpp - Windows Live Messenger UUN processing

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
#include "Papillon/Tasks/UUNTask"

// Qt includes
#include <QtDebug>
#include <QtCore/QLatin1String>

// Papillon includes
#include "Papillon/NetworkMessage"
#include "Papillon/Connection"

namespace Papillon
{
class UUNTask::Private
{
	public:
		Private()
		{
		}
		// Keep track of the expected transaction ID.
		QString currentTransactionId;
		QString UUNReceiver;
		QString UUNType;
		QString UUNPayload;
}

UUNTask::UUNTask(Task *parent)
	:Task(parent), d(new Private)
{

}

UUNTask::~UUNTask()
{
	delete d;
}

void UUNTask::onGo()
{
	qDebug() << Q_FUNC_INFO << "post UUN to server...";
	postUUNCommand();
}

void UUNTask::setUUNReveiver(QString receiver)
{
	d->UUNReceiver = receiver;
}

void UUNTask::setUUNType(QString type)
{
	d->UUNType = type;
}

void UUNTask::setUUNPayload(QString payload)
{
	d->UUNPayload = payload;
}

void UUNTask::postUUNCommand()
{
	qDebug() << Q_FUNC_INFO << "Sending UUN command.";
	NetworkMessage *UUNMessage = new NetworkMessage(NetworkMessage::TransactionTransfer);
	UUNMessage->setCommand( QLatin1String("UUN") );
	d->currentTransactionId = QString::number( connection()->transactionId() );
	UUNMessage->setTransactionId( d->currentTransactionId );

	UUNMessage->setArguments( QString("%1 %2 %3 %4").arg( d->UUNReceiver ).arg( d->UUNType ).arg( d->UUNPayload.size() ).arg( d->UUNPayload ) );

	send(UUNMessage);
}

bool UUNListTask::take(NetworkMessage *networkMessage)
{
	if( forMe(networkMessage) )
	{
		return true;
	}
}

bool UUNTask::forMe(NetworkMessage *networkMessage) const
{
	if( networkMessage->type() == NetworkMessage::TransactionMessage )
	{
		if( networkMessage->command() == QLatin1String("UUN") )
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
#include "uuntask.moc"
