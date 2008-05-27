/*
    managedconnectionaccount.h - Kopete Account that uses a  manager to
    control its connection and respond to connection events

    Copyright (c) 2005      by Will Stephenson <wstephenson@kde.org>
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

#ifndef MANAGEDCONNECTIONACCOUNT_H
#define MANAGEDCONNECTIONACCOUNT_H

#include "networkstatuscommon.h"

#include "kopetepasswordedaccount.h"

namespace Kopete
{
class Protocol;

/**
 * A ManagedConnectionAccount queries the NetworkStatus KDED Module before trying to connect using 
 * connectwithPassword, starting a network connection if needed.  If the network is not available, 
 * it delays calling performConnectWithPassword until it receives notification from the daemon 
 * that the network is up.  The account receiveds notifications from the daemon of network failures
 * and calls disconnect to set the account offline in a timely manner.
 */
class KOPETE_EXPORT ManagedConnectionAccount : public PasswordedAccount
{
	Q_OBJECT
	public:
		/**
		* @brief ManagedConnectionAccount constructor.
		* @param parent The protocol this account connects via
		* @param acctId The ID of this account - should be unique within this protocol
		* @param maxPasswordLength The maximum length for passwords for this account, or 0 for no limit
		*/
		ManagedConnectionAccount( Protocol *parent, const QString &acctId );
	public slots:
		/**
		 * @brief Begin the connection process, by checking if the connection is available with the ConnectionManager.
		 * This method is called by PasswordedAccount::connect()
		 * @param password the password to connect with.
		 */
		void connectWithPassword( const QString &password );
	protected:
		/** 
		 * @brief Connect to the server, once the network is available.
		 * This method is called by the ManagedConnectionAccount once the network is available. In this method you should set up your 
		 * network connection and connect to the server.
		 */
		virtual void performConnectWithPassword( const QString & password ) = 0;
	protected slots:
		/**
		 * @brief Handle a change in the network connection
		 * Called by the ConnectionManager when the network comes up or fails.  
		 * The default implementation calls performConnectWithPassword when the network goes online and connectWithPassword() was
		 * previously called, and calls disconnect() when the connection goes down.
		 * @param host For future expansion.
		 * @param status the new status of the network
		 */
		virtual void slotConnectionStatusChanged( const QString & host, NetworkStatus::EnumStatus status );
	private:
		QString m_password;
		bool m_waitingForConnection;
};

}

#endif
