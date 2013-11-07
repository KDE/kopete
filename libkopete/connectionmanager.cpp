/*
     connectionmanager.cpp - Provides the client side interface to the kde networkstatus daemon

     Copyright (c) 2004      by Will Stephenson <wstephenson@kde.org>

     Kopete    (c) 2004-2007 by the Kopete developers  <kopete-devel@kde.org>

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

#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "clientiface_stub.h"
#include "networkstatuscommon.h"

// ConnectionManager's private parts
class ConnectionManagerPrivate
{
	public:
		// this holds the currently active state
		ConnectionManager::State m_state;
		ClientIface_stub * m_stub;
		bool m_userInitiatedOnly;
};

// Connection manager itself
ConnectionManager::ConnectionManager( QObject * parent, const char * name ) : DCOPObject( "ConnectionManager" ),QObject( parent )
{
	setObjectName(name);
	d = new ConnectionManagerPrivate;
	
	d->m_stub = new ClientIface_stub( kapp->dcopClient(), "kded", "networkstatus" );
	
	connectDCOPSignal( "kded", "networkstatus", "statusChange(QString,int)", "slotStatusChanged(QString,int)", false );
	d->m_userInitiatedOnly = false;
	initialise();
}

ConnectionManager *ConnectionManager::self()
{
	static ConnectionManager s;
	return &s;
}

void ConnectionManager::initialise()
{
	// determine initial state and set the state object accordingly.
	d->m_state = Inactive;
	updateStatus();
}

void ConnectionManager::updateStatus()
{
	NetworkStatus::EnumStatus daemonStatus = (NetworkStatus::EnumStatus)d->m_stub->status( QString() );
	kDebug() ;
	switch ( daemonStatus )
	{
		case NetworkStatus::Offline:
		case NetworkStatus::OfflineFailed:
		case NetworkStatus::OfflineDisconnected:
		case NetworkStatus::ShuttingDown:
			if ( d->m_state == Online )
			{
				kDebug() << "STATE IS PENDING";
				d->m_state = Pending;
			}
			else
			{
				kDebug() << "STATE IS OFFLINE";
				d->m_state = Offline;
			}
			break;
		case NetworkStatus::Establishing:
		case NetworkStatus::Online:
			kDebug() << "STATE IS ONLINE";
			d->m_state = Online;
			break;
		case NetworkStatus::NoNetworks:
		case NetworkStatus::Unreachable:
			kDebug() << "STATE IS INACTIVE";
			d->m_state = Inactive;
			break;
	}
}

ConnectionManager::~ConnectionManager()
{
	delete d;
}

NetworkStatus::EnumStatus ConnectionManager::status( const QString & host )
{
	// need also to check that the daemon hasn't died
	updateStatus();
	if ( d->m_state == Pending )
		return NetworkStatus::Offline;
	if ( d->m_state == Online )
		return NetworkStatus::Online;
	if ( d->m_state == Offline )
		return NetworkStatus::Offline;
	return NetworkStatus::NoNetworks;
}

NetworkStatus::EnumRequestResult ConnectionManager::requestConnection( QWidget * mainWidget, const QString & host, bool userInitiated )
{
	kDebug() ;
	NetworkStatus::EnumRequestResult result;
	// if offline and the user has previously indicated they didn't want any new connections, suppress it
	if ( d->m_state == Offline && !userInitiated && d->m_userInitiatedOnly )
		result = NetworkStatus::UserRefused;
	// if offline, ask the user whether this connection should be allowed
	if ( d->m_state == Offline )
	{
		if ( askToConnect( mainWidget ) )
			//result = NetworkStatus::Connected;
			result = (NetworkStatus::EnumRequestResult)d->m_stub->request( host, userInitiated );
		else
			result = NetworkStatus::UserRefused;
	}
	// otherwise, just ask for the connection
	else
		result = (NetworkStatus::EnumRequestResult)d->m_stub->request( host, userInitiated );
	
	return result;
}

void ConnectionManager::relinquishConnection( const QString & host )
{
	d->m_stub->relinquish( host );
}

void ConnectionManager::slotStatusChanged( QString host, int status )
{
	kDebug() ;
	updateStatus();
	// reset user initiated only flag if we are now online
	if ( d->m_state == Online )
		d->m_userInitiatedOnly = false;

	emit statusChanged( host, (NetworkStatus::EnumStatus)status );
}

bool ConnectionManager::askToConnect( QWidget * mainWidget )
{
	i18n( "A network connection was disconnected.  The application is now in offline mode.  Do you want the application to resume network operations when the network is available again?" );
	i18n( "This application is currently in offline mode.  Do you want to connect?" );
	return ( KMessageBox::questionYesNo( mainWidget,
			i18n("This application is currently in offline mode.  Do you want to connect in order to carry out this operation?"),
			i18n("Leave Offline Mode?"),
			i18n("Connect"), i18n("Stay Offline"),
			QLatin1String("OfflineModeAlwaysGoOnline") ) == KMessageBox::Yes );
}

#include "connectionmanager.moc"
