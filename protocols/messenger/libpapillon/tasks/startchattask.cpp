/*
   startchattask.cpp - Windows Live Messenger start chat task

    Copyright (c) 2007		by Zhang Panyong  <pyzhang8@gmail.com>

   *************************************************************************
   *                                                                       *
   * This library is free software; you can redistribute it and/or         *
   * modify it under the terms of the GNU Lesser General Public            *
   * License as published by the Free Software Foundation; either          *
   * version 2 of the License, or (at your option) any later version.      *
   *                                                                       *
   *************************************************************************
*/

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

bool StartChatTask::take(Transfer *transfer)
{
	if(( transfer->command() == QLatin1String("XFR")) &&
			(transfer->arguments()[0])== QLatin1String("SB") )
	{
		emit startChat(	transfer->arguments()[1] , transfer->arguments()[3] );
		return true;
	}

	if(transfer->command() == QLatin1String("RNG"))
	{
		//TODO finish the invite to chat code
	}
	return false;
}

void StartChatTask::sendStartChatCommand()
{
	qDebug() << PAPILLON_FUNCINFO << "Sending XFR SB command.";
	Transfer *startChatTransfer = new Transfer(Transfer::TransactionTransfer);

	pingTransfer->setCommand( QLatin1String("XFR") );
	d->currentTransactionId = QString::number( connection()->transactionId());
	startChatTransfer->setTransactionId( d->currentTransactionId);
	startChatTransfer->setArguments( QString("SB") );

	send(startChatTransfer);
}

void StartChatTask::onGo()
{
	sendStartChatCommand();
}

}

#include "startchattask.moc"
