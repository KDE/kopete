/*
    gwaccount.h - Kopete GroupWise Protocol

    Copyright (c) 2004      SUSE Linux AG	 	 http://www.suse.com
    
    Based on Testbed    
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

#include "gwfield.h"

class KActionMenu;
class KopeteContact;
class KopeteMetaContact;

class GroupWiseContact;
class GroupWiseProtocol;

/**
 * This represents an account connected to GroupWise
 * @author Will Stephenson
*/
class GroupWiseAccount : public KopeteAccount
{
	Q_OBJECT
public:
	GroupWiseAccount( GroupWiseProtocol *parent, const QString& accountID, const char *name = 0 );
	~GroupWiseAccount();
	/**
	 * Construct the context menu used for the status bar icon
	 */
	virtual KActionMenu* actionMenu();

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
	 * 'Connect' to the groupwise server.  Only sets myself() online.
	 */
	virtual void connect();
	/**
	 * Disconnect from the server.  Only sets myself() offline.
	 */
	virtual void disconnect();
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

protected slots:
	/**
	 * Change the account's status.  Called by KActions and internally.
	 */
	void slotGoOnline();
	void slotGoAway();
	void slotGoOffline();
	void slotGoBusy();
	void slotGoAppearOffline();
	/**
	 * Update the local user's metadata
	 */
	void slotGotMyDetails( Field::FieldList & fields );
};

#endif
