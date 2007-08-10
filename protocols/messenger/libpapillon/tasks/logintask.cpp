/*
   logintask.cpp - Windows Live Messenger Login Task

    Copyright (c) 2007		by Zhang Panyong  <pyzhang8@gmail.com>
    Copyright (c) 2006 		by Michaël Larouche <larouche@kde.org>

   *************************************************************************
   *                                                                       *
   * This library is free software; you can redistribute it and/or         *
   * modify it under the terms of the GNU Lesser General Public            *
   * License as published by the Free Software Foundation; either          *
   * version 2 of the License, or (at your option) any later version.      *
   *                                                                       *
   *************************************************************************
*/
#include "Papillon/Tasks/LoginTask"

// Qt includes
#include <QtCore/QStringList>
#include <QtCore/QLatin1String>
#include <QtDebug>

// Papillon includes
#include "Papillon/NetworkMessage"
#include "Papillon/Connection"
#include "Papillon/Http/SSOHandler"
#include "Papillon/Client"
#include "Papillon/UserContact"

namespace Papillon 
{

class LoginTask::Private
{
public:
	Private()
	 : currentState(LoginTask::StateVersion)
	{}

	LoginTask::LoginState currentState;

	// Keep track of the expected transaction ID.
	QString currentTransactionId;

	QString ssoTicket;
};

LoginTask::LoginTask(Task *parent)
 : Task(parent), d(new Private)
{
}

LoginTask::~LoginTask()
{
	delete d;
}

LoginTask::LoginState LoginTask::loginState() const
{
	return d->currentState;
}

bool LoginTask::take(NetworkMessage *networkMessage)
{
	if( forMe(networkMessage) )
	{
		bool proceeded = false;
		switch(d->currentState)
		{
			case StateVersion:
			{
				if( networkMessage->command() == QLatin1String("VER") )
				{
					d->currentState = StateCVR;
					proceeded = true;
					sendCvrCommand();
				}
				break;
			}
			case StateCVR:
			{
				if( networkMessage->command() == QLatin1String("CVR") )
				{
					d->currentState = StateSSOInvite;
					proceeded = true;
					sendSSOInviteCommand();
				}
				break;
			}
			case StateSSOInvite:
			{
				if( networkMessage->command() == QLatin1String("USR") )
				{
					if( networkMessage->arguments()[0] == QLatin1String("SSO") && networkMessage->arguments()[1] == QLatin1String("S") )
					{
						d->currentState = StateSSOConfirmed;

						QString sso = networkMessage->arguments()[2];
						SSOHandler *ssoHandler = new SSOHandler( connection()->client()->createSecureStream() );
						ssoHandler->setLoginInformation(sso, passportId(), password());
						connect(ssoHandler, SIGNAL(result( SSOHandler* )), this, SLOT(ticketReceived( SSOHandler* )));
						ssoHandler->start();

						proceeded = true;
					}
				}
				else if( networkMessage->command() == QLatin1String("XFR") )
				{
					{
						QString newServer = networkMessage->arguments()[1].section(":", 0, 0);
						QString tempPort = networkMessage->arguments()[1].section(":", 1, 1);
						bool dummy;
						int newPort = tempPort.toUInt(&dummy);

						d->currentState = StateRedirection;
						proceeded = true;
						emit redirection(newServer, newPort);
					}
				}
				break;
			}
			case StateSSOConfirmed:
			{
				if( networkMessage->command() == QLatin1String("USR") )
				{
					if( networkMessage->arguments()[0] == QLatin1String("OK") )
					{
						proceeded = true;
						d->currentState = StateFinish;
						// End the login task.
						setSuccess();
					}
				}
				break;
			}
			default:
				return false;
		}

		return proceeded;
	}

	return false;
}

bool LoginTask::forMe(NetworkMessage *networkMessage)
{
	if( networkMessage->type() == NetworkMessage::TransactionMessage )
	{
		if( networkMessage->transactionId() == d->currentTransactionId )
			return true;
	}

	return false;
}

// TODO: Send VER, CVR, USR I at the same time
void LoginTask::onGo()
{
	qDebug() << Q_FUNC_INFO << "Begin login process...";

	sendVersionCommand();
}

void LoginTask::sendVersionCommand()
{
	qDebug() << Q_FUNC_INFO << "Sending version command.";
	NetworkMessage *versionNetworkMessage = new NetworkMessage(NetworkMessage::TransactionMessage);
	versionNetworkMessage->setCommand( QLatin1String("VER") );

	d->currentTransactionId = QString::number( connection()->transactionId() );
	versionNetworkMessage->setTransactionId( d->currentTransactionId );

	versionNetworkMessage->setArguments( QString("MSNP15 MSNP14 CVR0") );

	send(versionNetworkMessage);
}

void LoginTask::sendCvrCommand()
{
	qDebug() << Q_FUNC_INFO << "Sending CVR command.";
	NetworkMessage *cvrMessage = new NetworkMessage(NetworkMessage::TransactionMessage);
	cvrMessage->setCommand( QLatin1String("CVR") );
	
	d->currentTransactionId = QString::number( connection()->transactionId() );
	cvrMessage->setTransactionId( d->currentTransactionId );

	QString arguments = QString("0x0409 winnt 5.1 i386 MSNMSGR 8.1.0178 msmsgs %1").arg( passportId() );
	cvrMessage->setArguments(arguments);

	send(cvrMessage);
}

void LoginTask::sendSSOInviteCommand()
{
	qDebug() << Q_FUNC_INFO << "Sending SSO Invite Command";
	NetworkMessage *twnTransfer = new NetworkMessage(Transfer::TransactionTransfer);
	twnTransfer->setCommand("USR");

	d->currentTransactionId = QString::number( connection()->transactionId() );
	twnMessage->setTransactionId( d->currentTransactionId );

	QString arguments = QString("SSO I %1").arg( passportId() );
	twnMessage->setArguments(arguments);

	send(twnMessage);
}

void LoginTask::sendSSOConfirmation()
{
	qDebug() << Q_FUNC_INFO << "Sending SSO confirmation command.";
	NetworkMessage *ssoTransfer = new NetworkMessage(Transfer::TransactionTransfer);
	ssoTransfer->setCommand("USR");

	d->currentTransactionId = QString::number( connection()->transactionId() );
	ssoTransfer->setTransactionId( d->currentTransactionId );

	QString arguments = QString("SSO S %1").arg(d->SSOTicket);
	ssoTransfer->setArguments(arguments);

	send(ssoTransfer);
}

void LoginTask::ticketReceived(SSOHandler *ssoHandler)
{
	if( ssoHandler->success() )
	{
		d->ssoTicket = ssoHandler->ticket();
		ssoHandler->deleteLater();

		sendSSOConfirmation();
	}
	else
	{
		d->currentState = LoginTask::StateBadPassword;
		setError();
	}
}

QString LoginTask::passportId() const
{
	return connection()->client()->userContact()->passportId();
}

QString LoginTask::password() const
{
	return connection()->client()->userContact()->password();
}

}

#include "logintask.moc"
