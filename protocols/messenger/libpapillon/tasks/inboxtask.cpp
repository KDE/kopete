/*
   inboxtask.cpp - Windows Live Messenger inbox  task

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
// Papillon includes
#include "Papillon/NetworkMessage"

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
	qDebug() << Q_FUNC_INFO << "Sending URL INBOX command.";
	NetworkMessage *inboxMessage = new NetworkMessage(NetworkMessage::TransactionTransfer);

	inboxMessage->setCommand( QLatin1String("URL") );
	d->currentTransactionId = QString::number(connection()->transactionId()) ;
	inboxMessage->setTransactionId( d->currentTransactionId );
	inboxMessage->setArguments( QString("INBOX") );

	send(inboxMessage);
}

InboxTask::onGo()
{
	sendInboxCommand();
}

bool InboxTask::take(NetworkMessage *networkMessage) const
{
	if( forMe(networkMessage) )
	{
		/*TODO emit signal of URL
		 * receive format is:  URL 9 /cgi-bin/HoTMaiL https://login.live.com/ppsecure/md5auth.srf?lc=2052 2
		 */
		return true;
	}

	return false;
}


bool InboxTask::forMe(NetworkMessage *networkMessage) const
{
	if( networkMessage->command() == QLatin1String("URL") )
	{
		return true;
	}

	return false;	
}

}

#include "inboxtask.moc"
