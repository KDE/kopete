/*
   setstatusmessagetask.cpp - Set status message on server.

   Copyright (c) 2006 by Michaël Larouche <larouche@kde.org>

   *************************************************************************
   *                                                                       *
   * This library is free software; you can redistribute it and/or         *
   * modify it under the terms of the GNU Lesser General Public            *
   * License as published by the Free Software Foundation; either          *
   * version 2 of the License, or (at your option) any later version.      *
   *                                                                       *
   *************************************************************************
 */
#include "Papillon/Tasks/SetStatusMessageTask"

// Qt includes
#include <QtCore/QLatin1String>
#include <QtDebug>

// Papillon includes
#include "Papillon/Transfer"
#include "Papillon/Connection"
#include "Papillon/StatusMessage"

namespace Papillon
{

class SetStatusMessageTask::Private
{
public:
	Private()
	{}

	Papillon::StatusMessage statusMessage;
	QString currentTransactionId;
};

SetStatusMessageTask::SetStatusMessageTask(Papillon::Task *parent)
 : Papillon::Task(parent), d(new Private)
{}

SetStatusMessageTask::~SetStatusMessageTask()
{
	delete d;
}

void SetStatusMessageTask::setStatusMessage(const Papillon::StatusMessage &statusMessage)
{
	d->statusMessage = statusMessage;
}

bool SetStatusMessageTask::take(Transfer *transfer)
{
	if( transfer->transactionId() == d->currentTransactionId )
	{
		setSuccess();
		return true;
	}

	return false;
}

void SetStatusMessageTask::onGo()
{
	d->currentTransactionId = QString::number( connection()->transactionId() );

	Transfer *setStatusTransfer = new Transfer( Transfer::TransactionTransfer | Transfer::PayloadTransfer );
	setStatusTransfer->setCommand( QLatin1String("UUX") );
	setStatusTransfer->setTransactionId( d->currentTransactionId );

	setStatusTransfer->setPayloadData( d->statusMessage.toXml().toUtf8() );

	qDebug() << PAPILLON_FUNCINFO << "Setting personal status message on server:" << d->statusMessage.toXml();
	send(setStatusTransfer);
}

}

#include "setstatusmessagetask.moc"
