/*
   notifypresencetask.cpp - Notify about presence changes of contacts

   Copyright (c) 2006 by Michaël Larouche <michael.larouche@kdemail.net>

   *************************************************************************
   *                                                                       *
   * This library is free software; you can redistribute it and/or         *
   * modify it under the terms of the GNU Lesser General Public            *
   * License as published by the Free Software Foundation; either          *
   * version 2 of the License, or (at your option) any later version.      *
   *                                                                       *
   *************************************************************************
*/
#include "notifypresencetask.h"

// Qt includes
#include <QtCore/QStringList>
#include <QtCore/QLatin1String>
#include <QtDebug>

// Papillon includes
#include "transfer.h"
#include "papillonglobal.h"

namespace Papillon
{

class NotifyPresenceTask::Private
{
public:
	Private()
	{}
};

NotifyPresenceTask::NotifyPresenceTask(Papillon::Task *parent)
 : Papillon::Task(parent), d(new Private)
{}

NotifyPresenceTask::~NotifyPresenceTask()
{
	delete d;
}


bool NotifyPresenceTask::take(Transfer *transfer)
{
	if( forMe(transfer) )
	{
		QString contactId;
		Papillon::OnlineStatus::Status newOnlineStatus;

		// ILN is initial presence and NLN normal presence change.
		if( transfer->command() == QLatin1String("NLN") || transfer->command() == QLatin1String("ILN") )
		{
			newOnlineStatus = stringToStatus( transfer->arguments()[0] );
			contactId = transfer->arguments()[1];
			// TODO: Handle nickname, features, MsnObject
		}
		// Contact went offline
		else if( transfer->command() == QLatin1String("FLN") )
		{
			newOnlineStatus = OnlineStatus::Offline;
			contactId = transfer->arguments()[0];
		}

		emit contactStatusChanged(contactId, newOnlineStatus);

		return true;
	}

	return false;
}

bool NotifyPresenceTask::forMe(Transfer *transfer) const
{
	if( transfer->command() == QLatin1String("ILN") ||
		transfer->command() == QLatin1String("NLN") ||
		transfer->command() == QLatin1String("FLN") )
	{
		return true;
	}
	
	return false;
}

}

#include "notifypresencetask.moc"
