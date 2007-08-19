/*
   offlinemessage.cpp - Windows Live Messenger Offline Message processing

   Copyright (c) 2007		by Zhang Panyong  <pyzhang@gmail.com>
   Kopete    (c) 2002-2005 by the Kopete developers	<kopete-devel@kde.org>

   *************************************************************************
   *                                                                       *
   * This library is free software; you can redistribute it and/or         *
   * modify it under the terms of the GNU Lesser General Public            *
   * License as published by the Free Software Foundation; either          *
   * version 2 of the License, or (at your option) any later version.      *
   *                                                                       *
   *************************************************************************
 */
#include "Papillon/OfflineMessage"

// Qt includes
#include <QtDebug>

// Papillon includes
#include "Papillon/Client"

namespace Papillon
{

class OfflineMessage::Private
{
public:
	Private()
	{}
};

OfflineMessage::OfflineMessage(Client *client)
 : QObject(client), d(new Private)
{
	d->client = client;
}

OfflineMessage::~OfflineMessage()
{
	delete d;
}

Client *OfflineMessage::client()
{
	return d->client;
}

OfflineMessage::getMessage()
{

}

OfflineMessage::sendMessage()
{

}

OfflineMessage::deleteMessage()
{

}

}
#include "offlinemessage.moc"
