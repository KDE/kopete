/*
    connectionmanager.h - Provides the client side interface to the kde networkstatus daemon

    Copyright (c) 2004      by Will Stephenson <lists@stevello.free-online.co.uk>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef KDE_CONNECTION_MANAGER_H
#define KDE_CONNECTION_MANAGER_H

#include <dcopobject.h>

#include "networkstatuscommon.h"

class ConnectionManagerPrivate;

class ConnectionManager : public QObject, virtual public DCOPObject
{
	Q_OBJECT
	K_DCOP
	public:
		static ConnectionManager* self();
		enum State { Inactive, Online, Offline, Pending };
		virtual ~ConnectionManager();
		NetworkStatus::EnumStatus status( const QString & host );
			// check if a hostname is available.  Ask user if offline.  Request host
		NetworkStatus::EnumRequestResult requestConnection( QWidget* mainWidget, const QString & host, bool userInitiated );
			// method to relinquish a connection
		void relinquishConnection( const QString & host );
	signals:
		// signal that the network for a hostname is up/down
		void statusChanged( const QString & host, NetworkStatus::EnumStatus status );
	protected:
		// sets up internal state
		void initialise();
		// reread the desktop status from the daemon and update internal state
		void updateStatus();
			// ask if the user would like to reconnect
		bool askToConnect( QWidget * mainWidget );
	k_dcop:
			void slotStatusChanged( QString host, int status );
	private:
		ConnectionManager( QObject *parent, const char * name );
		ConnectionManagerPrivate *d;
		static ConnectionManager * s_self;
};

#endif

