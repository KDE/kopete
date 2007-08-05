/*
   notifystatusmessagetask.cpp - Notify contact status message changes.

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
#include "Papillon/Tasks/NotifyStatusMessageTask"

// Qt includes
#include <QtCore/QStringList>
#include <QtCore/QLatin1String>

// Papillon includes
#include "Papillon/NetworkMessage"
#include "Papillon/StatusMessage"

namespace Papillon
{

class NotifyStatusMessageTask::Private
{
public:
	Private()
	{}
};

NotifyStatusMessageTask::NotifyStatusMessageTask(Papillon::Task *parent)
 : Papillon::Task(parent), d(new Private)
{}

NotifyStatusMessageTask::~NotifyStatusMessageTask()
{
	delete d;
}

bool NotifyStatusMessageTask::take(NetworkMessage *networkMessage)
{
	if( networkMessage->command() == QLatin1String("UBX") )
	{
		QString rawStatusMessage( networkMessage->payloadData() );
		QString contactId = networkMessage->arguments()[0];
	
		Papillon::StatusMessage newStatusMessage = Papillon::StatusMessage::fromXml(rawStatusMessage);

		emit contactStatusMessageChanged(contactId, newStatusMessage);

		return true;
	}

	return false;
}

}
#include "notifystatusmessagetask.moc"
