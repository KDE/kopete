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

#ifndef MANAGEDCONNECTIONACCOUNT_H
#define MANAGEDCONNECTIONACCOUNT_H

#include "networkstatuscommon.h"

#include "kopetepasswordedaccount.h"

namespace Kopete
{
class Protocol;
	
class KOPETE_EXPORT ManagedConnectionAccount : public PasswordedAccount
{
	Q_OBJECT
	public:
			ManagedConnectionAccount( Protocol *parent, const QString &acctId, uint maxPasswordLength = 0, const char *name = 0 );
	public slots:
		void connectWithPassword( const QString &password );
	protected:
		virtual void performConnectWithPassword( const QString & password ) = 0;
	protected slots:
		void slotConnectionStatusChanged( const QString & host, NetworkStatus::EnumStatus );
	private:
		QString m_password;
		bool m_waitingForConnection;
};

}

#endif
