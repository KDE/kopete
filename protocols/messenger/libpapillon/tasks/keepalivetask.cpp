/*
   keepalivetask.cpp - Windows Live Messenger keep alive task

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

class KeepAliveTask::Private
{
public:
	Private()
	{}
};

KeepAliveTask::KeepAliveMessageTask(Papillon::Task *parent)
 : Papillon::Task(parent), d(new Private)
{
	m_ping = false;
	m_keepaliveTimer = 0L;
}

KeepAliveTask::~KeepAliveTask()
{
	delete d;
}

void KeepAliveTask::onGo()
{
	bool _userHttpMethond = false;
	qDebug() << Q_FUNC_INFO << "Begin ping to ensure server alive";

	if( _userHttpMethond ) {
		if( m_keepaliveTimer ) {
			delete m_keepaliveTimer;
			m_keepaliveTimer = 0L;
		}
	}
	else {
		if( !m_keepaliveTimer ) {
			m_keepaliveTimer = new QTimer( this );
			m_keepaliveTimer->setObjectName( QLatin1String("m_keepaliveTimer") );
			QObject::connect( m_keepaliveTimer, SIGNAL( timeout() ), SLOT( slotSendKeepAlive() ) );
		}
	}
}

void KeepAliveTask::sendPingCommand()
{
	qDebug() << Q_FUNC_INFO << "Sending PNG command.";
	NetworkMessage *pingMessage = new NetworkMessage(NetworkMessage::TransactionMessage);

	pingMessage->setCommand( QLatin1String("PNG") );
	pingMessage->setTransactionId( QString() );

	send(pingMessage);
}

void KeepAliveTask::slotdisconnect()
{
	if( m_keepaliveTimer )
		m_keepaliveTimer->stop();
}

void KeepAliveTask::slotSendKeepAlive()
{
	//we did not received the previous QNG
	if(m_ping)
	{
		m_disconnectReason=Kopete::Account::ConnectionReset;
		//TODO emit disconnect signal
		/*KMessageBox::queuedMessageBox( Kopete::UI::Global::mainWidget(), KMessageBox::Information,
		i18n( "The connection with the Messenger network has been lost." ) , i18n ("Messenger Plugin") );*/
		emit serverDisconnect(m_disconnectReason);
		return;
	}
	else
	{
		/* Send a dummy command to fake activity. This makes sure Windows Live Messenger doesn't
		 *  disconnect you when the notify socket is idle.
		 */
		sendPingCommand();
		m_ping=true;
	}
}

bool KeepAliveTask::take(NetworkMessage *networkMessage)
{
	if( networkMessage->command() == QLatin1String("QNG") )
	{
		m_ping = false;
		if(m_keepaliveTimer)
		{
			m_keepaliveTimer->setSingleSlot(true);
			m_keepaliveTimer->start(id*950);
		}

		return true;
	}

	return false;
}

}
#include "keepalivetask.moc"
