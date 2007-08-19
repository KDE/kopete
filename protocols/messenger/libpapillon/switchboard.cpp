/*
    switchboard.cpp - Messenger Switchboard handle class

    Copyright (c) 2007		by Zhang Panyong        <pyzhang@gmail.com>
    Kopete    (c) 2002-2005 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/
#include "Papillon/SwitchBoard"

namespace Papillon
{

class SwitchBoard::Private
	:switchboardConnection(0)
{
public:
	Connection *switchboardConnection;
	QString server;
	uint	port;
	QStringList chatMembers;
}

SwitchBoard::SwitchBoard()
: QObject(parent), d(new Private)
{

}

SwitchBoard::~SwitchBoard()
{
	delete d;
}

Connection *SwitchBoard::connection()
{
	return d->switchboardConnection;
}

SwitchBoard::setServer(const QString &server, uint port)
{
	d->server = server;
	d->port = port;
}

/*we need to call the setServer before connect()*/
void SwitchBoard::connect()
{
	if(!d->switchboardConnection){
		d->switchboardConnection = createConnection();
		connect(d->switchboardConnection, SIGNAL(connected()), this, SLOT(switchboardConnected()));
	}
	d->switchboardConnection->connectToServer(d->server, d->port);
}

void Switchboard::switchboardConnected()
{

}

void Switchboard::slotClosed()
{
	//TODO notify member connection closed
	emit switchBoardClosed();
}

Switchboard::slotCloseSession()
{
	//TODO send OUT command
	d->switchboardConnection->disconnectFromServer();
}

void Switchboard::slotUserLeft(const QString &handle, const QString &reason)
{
	emit userLeft(handle, reason);

	if( d->chatMembers.contains(handle) )
	{
		d->chatMembers.removeAll( handle );
	}
	if( d->chatMembers.isEmpty() )
	{
		d->switchboardConnection->disconnectFromServer();
	}
}

}
#include "switchboard.moc"
