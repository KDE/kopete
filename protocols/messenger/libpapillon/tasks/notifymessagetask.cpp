/*
   notifymessagetask.h - Notify about new messages on Notification server.

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
#include "notifymessagetask.h"

// Qt includes
#include <QtCore/QLatin1String>
#include <QtCore/QRegExp>
#include <QtCore/QStringList>

// Papillon includes
#include "transfer.h"
#include "mimeheader.h"

namespace Papillon
{

class NotifyMessageTask::Private
{
public:
	Private()
	{}
};

NotifyMessageTask::NotifyMessageTask(Task *parent)
 : Papillon::Task(parent), d(new Private)
{
}


NotifyMessageTask::~NotifyMessageTask()
{
	delete d;
}


bool NotifyMessageTask::take(Transfer *transfer)
{
	if( forMe(transfer) )
	{
		MimeHeader notifyMessage = MimeHeader::parseMimeHeader( QString(transfer->payloadData()) );

		emit profileMessage(notifyMessage);

		return true;
	}

	return false;
}


bool NotifyMessageTask::forMe(Transfer *transfer) const
{
	if( transfer->command() == QLatin1String("MSG") )
	{
		// TODO: Temp, shouldn't assume that it's Hotmail there.
		if( transfer->arguments()[0] == QLatin1String("Hotmail") )
		{
			return true;
		}
	}

	return false;	
}

}

#include "notifymessagetask.moc"
