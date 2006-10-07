/*
   setpresencetask.h - Set our own presence on Windows Live Messenger service.

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
#include "setpresencetask.h"

// Qt includes
#include <QtCore/QStringList>
#include <QtCore/QLatin1String>
#include <QtDebug>

// Papillon includes
#include "client.h"
#include "connection.h"
#include "transfer.h"
#include "papillonglobal.h"

namespace Papillon
{

class SetPresenceTask::Private
{
public:
	Private()
	 : features(0)
	{}

	Papillon::OnlineStatus::Status onlineStatus;
	Papillon::ClientInfo::Features features;

	// Keep track of the expected transaction ID.
	QString currentTransactionId;
};

SetPresenceTask::SetPresenceTask(Papillon::Task *parent)
 : Papillon::Task(parent), d(new Private)
{
}

SetPresenceTask::~SetPresenceTask()
{
	delete d;
}

void SetPresenceTask::setOnlineStatus(Papillon::OnlineStatus::Status onlineStatus)
{
	d->onlineStatus = onlineStatus;
}

void SetPresenceTask::setClientFeatures(Papillon::ClientInfo::Features features)
{
	d->features = features;
}

bool SetPresenceTask::take(Transfer *transfer)
{
	if( transfer->command() == QLatin1String("CHG") && transfer->transactionId() == d->currentTransactionId )
	{
		// End this task
		setSuccess();
		return true;
	}

	return false;
}

void SetPresenceTask::onGo()
{
	d->currentTransactionId = QString::number( connection()->transactionId() );

	Transfer *setPresenceTransfer = new Transfer(Transfer::TransactionTransfer);
	setPresenceTransfer->setCommand( QLatin1String("CHG") );
	setPresenceTransfer->setTransactionId( d->currentTransactionId );
	
	// Set arguments
	QStringList args;
	// String representation of the online status
	args << statusToString(d->onlineStatus);
	// Features that we support
	args << QString::number( static_cast<int>(d->features) );
	// TODO: Add MSNObject

	setPresenceTransfer->setArguments( args );

	qDebug() << PAPILLON_FUNCINFO << "Changing our own presence to:" << statusToString(d->onlineStatus);

	send(setPresenceTransfer);
}

}

#include "setpresencetask.moc"
