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

class KActionMenu;
class KopeteContact;
class KopeteMetaContact;

class TestbedContact;
class TestbedProtocol;
class TestbedFakeServer;

/**
 * This represents an account connected to the testbed
 * @author Will Stephenson
*/
class TestbedAccount : public KopeteAccount
{
	Q_OBJECT
public:
	TestbedAccount( TestbedProtocol *parent, const QString& accountID, const char *name = 0 );
	~TestbedAccount();
	/**
	 * Construct the context menu used for the status bar icon
	 */
	virtual KActionMenu* actionMenu();
	/**
	 * Return the KopeteContact representing your own status
	 */
	virtual KopeteContact* myself() const;
	/**
	 * Creates a protocol specific KopeteContact subclass and adds it to the supplie
	 * KopeteMetaContact
	 */
	virtual bool addContactToMetaContact(const QString& contactId, const QString& displayName, KopeteMetaContact* parentContact);
	/**
	 * Called when Kopete is set globally away
	 */
	virtual void setAway(bool away, const QString& reason);
	/**
	 * 'Connect' to the testbed server.  Only sets myself() online.
	 */
	virtual void connect();
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
	 * This identifies the sending KopeteContact and passes it a KopeteMessage
	 */
	void receivedMessage( const QString &message );

protected:
	/**
	 * This simulates contacts going on and offline in sync with the account's status changes
	 */
	void updateContactStatus();
	TestbedContact *m_myself;
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
	void slotGoOffline();

};

#endif
