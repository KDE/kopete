/*
   startchattask.cpp - Windows Live Messenger start chat task

    Copyright (c) 2007		by Zhang Panyong  <pyzhang@gmail.com>

   *************************************************************************
   *                                                                       *
   * This library is free software; you can redistribute it and/or         *
   * modify it under the terms of the GNU Lesser General Public            *
   * License as published by the Free Software Foundation; either          *
   * version 2 of the License, or (at your option) any later version.      *
   *                                                                       *
   *************************************************************************
*/

// Qt includes
#include <QtDebug>

// Papillon includes
#include "Papillon/NetworkMessage"

namespace Papillon
{

class StartChatTask::Private
{
public:
	Private()
	{}
	QString currentTransactionId;
};

StartChatTask::StartChatTask(Papillon::Task *parent)
 : Papillon::Task(parent), d(new Private)
{
}

StartChatTask::~StartChatTask()
{
	delete d;
}

bool StartChatTask::take(NetworkMessage *networkMessage)
{
	if(( networkMessage->command() == QLatin1String("XFR")) &&
			(networkMessage->arguments()[0])== QLatin1String("SB") )
	{
		emit startChat(	networkMessage->arguments()[1] , networkMessage->arguments()[3] );
		return true;
	}

	if(networkMessage->command() == QLatin1String("RNG"))
	{
		//TODO finish the invite to chat code
	}
	return false;
}

void StartChatTask::sendStartChatCommand()
{
	qDebug() << Q_FUNC_INFO << "Sending XFR SB command.";
	NetworkMessage *startChatMessage = new NetworkMessage(NetworkMessage::TransactionTransfer);

	startChatMessage->setCommand( QLatin1String("XFR") );
	d->currentTransactionId = QString::number( connection()->transactionId());
	startChatMessage->setTransactionId( d->currentTransactionId);
	startChatMessage->setArguments( QString("SB") );

	send(startChatMessage);
}

void StartChatTask::onGo()
{
	sendStartChatCommand();
}

}

#include "startchattask.moc"
