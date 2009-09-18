/*
    testbedaccount.h - Kopete Testbed Protocol

    Copyright (c) 2003      by Will Stephenson		 <will@stevello.free-online.co.uk>
    Kopete    (c) 2002-2003 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU General Public                   *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef TESTBEDACCOUNT_H
#define TESTBEDACCOUNT_H

#include <kopeteaccount.h>
#include "testbedwebcamdialog.h"

class KActionMenu;
namespace Kopete 
{ 
	class Contact;
	class MetaContact;
	class StatusMessage;
}

class TestbedProtocol;
class TestbedFakeServer;

/**
 * This represents an account connected to the testbed
 * @author Will Stephenson
*/
class TestbedAccount : public Kopete::Account
{
	Q_OBJECT
public:
	TestbedAccount( TestbedProtocol *parent, const QString& accountID );
	~TestbedAccount();
	/**
	 * Construct the context menu used for the status bar icon
	 */
	virtual void fillActionMenu( KActionMenu *actionMenu );

	/**
	 * Creates a protocol specific Kopete::Contact subclass and adds it to the supplie
	 * Kopete::MetaContact
	 */
	virtual bool createContact(const QString& contactId, Kopete::MetaContact* parentContact);
	/**
	 * Called when Kopete is set globally away
	 */
	virtual void setAway(bool away, const QString& reason);
	/**
	 * Called when Kopete status is changed globally
	 */
	virtual void setOnlineStatus(const Kopete::OnlineStatus& status , const Kopete::StatusMessage &reason = Kopete::StatusMessage(),
	                             const OnlineStatusOptions& options = None);
	virtual void setStatusMessage(const Kopete::StatusMessage& statusMessage);
	/**
	 * 'Connect' to the testbed server.  Only sets myself() online.
	 */
	virtual void connect( const Kopete::OnlineStatus& initialStatus = Kopete::OnlineStatus() );
	/**
	 * Disconnect from the server.  Only sets myself() offline.
	 */
	virtual void disconnect();
	/**
	 * Return a reference to the server stub
	 */
	TestbedFakeServer* server();
public slots:
	/**
	 * Called by the server when it has a message for us.
	 * This identifies the sending Kopete::Contact and passes it a Kopete::Message
	 */
	void receivedMessage( const QString &message );

protected:
	/**
	 * This simulates contacts going on and offline in sync with the account's status changes
	 */
	void updateContactStatus();
	TestbedFakeServer* m_server;

protected slots:
	/**
	 * Change the account's status.  Called by KActions and internally.
	 */
	void slotGoOnline();
	/**
	 * Change the account's status.  Called by KActions and internally.
	 */
	void slotGoAway();
	/**
	 * Change the account's status.  Called by KActions and internally.
	 */
	void slotGoBusy();
	/**
	 * Change the account's status.  Called by KActions and internally.
	 */
	void slotGoOffline();
	/**
	 * Show webcam.  Called by KActions and internally.
	 */
	void slotShowVideo();

};

#endif
