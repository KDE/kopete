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
#include "Papillon/Tasks/SetPresenceTask"

// Qt includes
#include <QtCore/QStringList>
#include <QtCore/QLatin1String>
#include <QtDebug>

// Papillon includes
#include "Papillon/Client"
#include "Papillon/Connection"
#include "Papillon/Transfer"
#include "Papillon/Global"

namespace Papillon
{

class SetPresenceTask::Private
{
public:
	Private()
	 : features(0)
	{}

	Papillon::Presence::Status presence;
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

void SetPresenceTask::setPresence(Papillon::Presence::Status presence)
{
	d->presence = presence;
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
	// String representation of the presence
	args << Papillon::Global::presenceToString(d->presence);
	// Features that we support
	args << QString::number( static_cast<int>(d->features) );
	// TODO: Add MSNObject

	setPresenceTransfer->setArguments( args );

	qDebug() << PAPILLON_FUNCINFO << "Changing our own presence to:" << Papillon::Global::presenceToString(d->presence);

	send(setPresenceTransfer);
}

}

#include "setpresencetask.moc"
