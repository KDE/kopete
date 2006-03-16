/*
   notificationconnection.h - Notification service connection.

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
#ifndef PAPILLONNOTIFICATIONCONNECTION_H
#define PAPILLONNOTIFICATIONCONNECTION_H

#include <connection.h>
#include <papillon_macros.h>

namespace Papillon 
{

class ClientStream;
/**
 * This is a connection to the Notification service in Windows Live Messenger.
 * Handle most common tasks for Windows Live Messenger.
 *
 * This connection should be unique for a complete session.
 *
 * @author Michaël Larouche <michael.larouche@kdemail.net>
*/
class PAPILLON_EXPORT NotificationConnection : public Connection
{
	Q_OBJECT
public:
	NotificationConnection(ClientStream *stream);
	~NotificationConnection();

private:
	class Private;
	Private *d;
};

}

#endif
