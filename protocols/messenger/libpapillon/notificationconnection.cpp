/*
   notificationconnection.cpp - Notification service connection.

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
#include "notificationconnection.h"

// Papillon includes
#include "papillonclientstream.h"

namespace Papillon 
{

class NotificationConnection::Private
{
public:
	Private()
	{}

	
};

NotificationConnection::NotificationConnection(ClientStream *stream)
 : Connection(stream), d(new Private)
{
}

NotificationConnection::~NotificationConnection()
{
}

}

#include "notificationconnection.moc"
