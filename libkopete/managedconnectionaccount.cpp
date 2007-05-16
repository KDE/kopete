/*
    managedconnectionaccount.h - Kopete Account that uses a  manager to
    control its connection and respond to connection events

    Copyright (c) 2005      by Will Stephenson <lists@stevello.free-online.co.uk>
    Kopete    (c) 2002-2005 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include <connectionmanager.h>
#include "kopeteuiglobal.h"

#include "managedconnectionaccount.h"


namespace Kopete
{
	
ManagedConnectionAccount::ManagedConnectionAccount( Protocol *parent, const QString &acctId, uint maxPasswordLength, const char *name )
	: PasswordedAccount( parent, acctId, maxPasswordLength, name ), m_waitingForConnection( false )
{
	QObject::connect( ConnectionManager::self(), SIGNAL(statusChanged( NetworkStatus::Status ) ),
										SLOT(slotConnectionStatusChanged( NetworkStatus::Status ) ) );
}

void ManagedConnectionAccount::connectWithPassword( const QString &password )
{
	m_password = password;
	NetworkStatus::Status status = ConnectionManager::self()->status();
	switch ( status )
	{
		case NetworkStatus::NoNetworks:
		case NetworkStatus::Online:
			m_waitingForConnection = false;
			performConnectWithPassword( password );
			break;
		default:
			m_waitingForConnection = true;
	}
}

void ManagedConnectionAccount::slotConnectionStatusChanged( NetworkStatus::Status status )
{
	if ( m_waitingForConnection && ( status == NetworkStatus::Online || status == NetworkStatus::NoNetworks ) )
	{
		m_waitingForConnection = false;
		performConnectWithPassword( m_password );
	}
	else if ( isConnected() && ( status == NetworkStatus::Offline 
						 || status == NetworkStatus::ShuttingDown 
						 || status == NetworkStatus::OfflineDisconnected 
				 		 || status == NetworkStatus::OfflineFailed
						 || status == NetworkStatus::Establishing ) )
	{
		m_waitingForConnection = true;
		disconnect();
	}
}

} // end namespace Kopete
#include "managedconnectionaccount.moc"
