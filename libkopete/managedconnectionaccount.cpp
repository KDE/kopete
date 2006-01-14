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

#include "connectionmanager.h"
#include "kopeteuiglobal.h"

#include "managedconnectionaccount.h"


namespace Kopete
{
	
ManagedConnectionAccount::ManagedConnectionAccount( Protocol *parent, const QString &acctId, uint maxPasswordLength, const char *name )
	: PasswordedAccount( parent, acctId, maxPasswordLength, name ), m_waitingForConnection( false )
{
	QObject::connect( ConnectionManager::self(), SIGNAL(statusChanged(const QString&, NetworkStatus::EnumStatus ) ),
										SLOT(slotConnectionStatusChanged(const QString&, NetworkStatus::EnumStatus ) ) );
}

void ManagedConnectionAccount::connectWithPassword( const QString &password )
{
	m_password = password;
	NetworkStatus::EnumStatus status = ConnectionManager::self()->status( QString::null );
	if ( status == NetworkStatus::NoNetworks )
		performConnectWithPassword( password );
	else
	{
		m_waitingForConnection = true;
		// need to adapt libkopete so we know the hostname in this class and whether the connection was user initiated
		// for now, these are the default parameters to always bring up a connection to "the internet".
		NetworkStatus::EnumRequestResult response = ConnectionManager::self()->requestConnection( Kopete::UI::Global::mainWidget(), QString::null, true );
		if ( response == NetworkStatus::Connected )
		{
			m_waitingForConnection = false;
			performConnectWithPassword( password );
		}
		else if ( response == NetworkStatus::UserRefused || response == NetworkStatus::Unavailable )
			disconnect();
	}
}

void ManagedConnectionAccount::slotConnectionStatusChanged( const QString & host, NetworkStatus::EnumStatus status )
{
	Q_UNUSED(host); // as above, we didn't register a hostname, so treat any connection as our own.
	
	if ( m_waitingForConnection && ( status == NetworkStatus::Online || status == NetworkStatus::NoNetworks ) )
	{
		m_waitingForConnection = false;
		performConnectWithPassword( m_password );
	}
	else if ( isConnected() && ( status == NetworkStatus::Offline 
						 || status == NetworkStatus::ShuttingDown 
						 || status == NetworkStatus::OfflineDisconnected 
				 		 || status == NetworkStatus::OfflineFailed ) )
		disconnect();
}
		
} // end namespace Kopete
#include "managedconnectionaccount.moc"
