/*
   inboxtask.cpp - Windows Live Messenger inbox  task

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

class InboxTask::Private
{
public:
	Private()
	{}
	QString currentTransactionId;
};

InboxTask::InboxTask(Task *parent)
 : Papillon::Task(parent), d(new Private)
{
}


InboxTask::~InboxTask()
{
	delete d;
}

void InboxTask::sendInboxCommand()
{
	qDebug() << PAPILLON_FUNCINFO << "Sending URL INBOX command.";
	Transfer *inboxTransfer = new Transfer(Transfer::TransactionTransfer);

	inboxTransfer->setCommand( QLatin1String("URL") );
	d->currentTransactionId = QString::number(connection()->transactionId()) ;
	inboxTransfer->setTransactionId( d->currentTransactionId );
	inboxTransfer->setArguments( QString("INBOX") );

	send(inboxTransfer);
}

InboxTask::onGo()
{
	sendInboxCommand();
}

bool InboxTask::take(Transfer *transfer)
{
	if( forMe(transfer) )
	{
		/*TODO emit signal of URL
		 * receive format is:  URL 9 /cgi-bin/HoTMaiL https://login.live.com/ppsecure/md5auth.srf?lc=2052 2
		 */
		return true;
	}

	return false;
}


bool InboxTask::forMe(Transfer *transfer) const
{
	if( transfer->command() == QLatin1String("URL") )
	{
		return true;
	}

	return false;	
}

}

#include "inboxtask.moc"
